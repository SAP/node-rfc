//import * as Promise from "bluebird";
import {
    Client,
    RfcConnectionParameters,
    RfcClientOptions,
} from "./sapnwrfc-client";
import { isUndefined } from "util";

export interface RfcPoolOptions {
    min: number;
    max: number;
}
class Mutex {
    private _lock: Promise<any>;

    constructor() {
        this._lock = Promise.resolve();
    }

    _acquire() {
        let release;
        const lock = (this._lock = new Promise((resolve) => {
            release = resolve;
        }));
        return release;
    }
    acquire(): Promise<any> {
        const q = this._lock.then(() => release);
        const release = this._acquire();
        return q;
    }
}

export class Pool {
    private __connectionParams: RfcConnectionParameters;
    private __poolOptions: RfcPoolOptions;
    private __clientOptions: RfcClientOptions | undefined;
    private __mutex: Mutex;

    private __clients: {
        ready: Map<number, Promise<Client>>;
        active: Set<number>;
    };

    constructor(
        connectionParams: RfcConnectionParameters,
        poolOptions: RfcPoolOptions = {
            min: 2,
            max: 50,
        },
        clientOptions?: RfcClientOptions
    ) {
        this.__mutex = new Mutex();
        this.__connectionParams = connectionParams;
        this.__poolOptions = poolOptions;
        this.__clientOptions = clientOptions;
        this.__clients = {
            ready: new Map(),
            active: new Set(),
        };
    }

    acquire(): Promise<Client | TypeError | void> {
        return (async () => {
            const unlock = await this.__mutex.acquire();

            for (
                let i = this.__clients.ready.size;
                i < this.__poolOptions.min;
                i++
            ) {
                let client = this.__clientOptions
                    ? new Client(this.__connectionParams, this.__clientOptions)
                    : new Client(this.__connectionParams);
                this.__clients.ready.set(client.id, client.open());
            }
            if (this.__clients.ready.size === 0) {
                return new Promise<TypeError>((resolve, reject) =>
                    reject(TypeError("Internal pool error, size = 0"))
                );
            }
            let id = this.__clients.ready.keys().next().value;
            let client = this.__clients.ready.get(id);
            this.__clients.ready.delete(id);
            this.__clients.active.add(id);
            unlock();
            return client;
        })();
    }

    release(client: Client) {
        return (async () => {
            await client.close();
            const unlock = await this.__mutex.acquire();
            if (this.__clients.ready.size < this.__poolOptions.min) {
                this.__clients.ready.set(client.id, client.open());
                this.__clients.active.delete(client.id);
            }
            unlock();
        })();
    }

    releaseAll(): Promise<void> {
        return (async () => {
            const unlock = await this.__mutex.acquire();
            for (const [id, client] of this.__clients.ready) {
                this.__clients.ready.delete(id);
                client.then((c) => c.close());
            }
            unlock();
        })();
    }

    get status(): object {
        return {
            active: this.__clients.active.size,
            ready: this.__clients.ready.size,
            options: this.__poolOptions,
        };
    }
}

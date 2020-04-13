var Promise = require("bluebird");
import {
    Client,
    RfcConnectionParameters,
    RfcClientOptions,
} from "./sapnwrfc-client";
import { isUndefined } from "util";
export { Promise };

export interface RfcPoolOptions {
    min: number;
    //max: number;
}

export class Pool {
    private __connectionParams: RfcConnectionParameters;
    private __poolOptions: RfcPoolOptions;
    private __clientOptions: RfcClientOptions | undefined;

    private __clients: {
        ready: Map<number, Promise<Client>>;
        active: Set<number>;
    };

    constructor(
        connectionParams: RfcConnectionParameters,
        poolOptions: RfcPoolOptions = {
            min: 2,
            //max: 50,
        },
        clientOptions?: RfcClientOptions
    ) {
        this.__connectionParams = connectionParams;
        this.__poolOptions = poolOptions;
        this.__clientOptions = clientOptions;
        this.__clients = {
            ready: new Map(),
            active: new Set(),
        };
        for (let i = 0; i < this.__poolOptions.min; i++) {
            let client = this.__clientOptions
                ? new Client(this.__connectionParams, this.__clientOptions)
                : new Client(this.__connectionParams);
            this.__clients.ready.set(client.id, client.open());
        }
    }

    acquire(): Promise<Client> {
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
        let id = this.__clients.ready.keys().next().value;
        let client = this.__clients.ready.get(id) as Promise<Client>;
        this.__clients.ready.delete(id);
        this.__clients.active.add(id);
        return client;
    }

    release(client: Client): Promise<void> {
        return new Promise((resolve, reject) => {
            if (!(client instanceof Client))
                reject(
                    new TypeError(
                        "Pool release() method requires a client instance as argument"
                    )
                );
            if (this.__clients.ready.size < this.__poolOptions.min) {
                this.__clients.active.delete(client.id);
                this.__clients.ready.set(client.id, client.reopen());
            } else {
                client.close(() => {});
            }
            resolve(client.id);
        });
    }

    releaseAll(): Promise<void> {
        const result = [Promise];
        this.__clients.ready.forEach((c, id) => {
            result.push(c.then((client) => client.close(() => {})));
        });
        this.__clients.ready = new Map();
        return Promise.all(result);
    }

    get status(): object {
        return {
            active: this.__clients.active.size,
            ready: this.__clients.ready.size,
            options: this.__poolOptions,
        };
    }
}

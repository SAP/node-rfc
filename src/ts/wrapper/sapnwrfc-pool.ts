var Promise = require("bluebird");
import {
    Client,
    RfcConnectionParameters,
    RfcClientOptions,
} from "./sapnwrfc-client";
import { isUndefined } from "util";
import { reject } from "bluebird";
export { Promise };

export interface RfcPoolOptions {
    min: number;
    //max: number;
}

export class Pool {
    private __connectionParams: RfcConnectionParameters;
    private __poolOptions: RfcPoolOptions;
    private __clientOptions: RfcClientOptions | undefined;
    private __fillRequests: number;

    private __clients: {
        ready: Array<Client>;
        active: Map<number, Client>;
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
        this.__fillRequests = 0;
        this.__clients = {
            ready: [],
            active: new Map(),
        };
    }

    newClient(): Client {
        return this.__clientOptions
            ? new Client(this.__connectionParams, this.__clientOptions)
            : new Client(this.__connectionParams);
    }

    refill() {
        if (
            this.__clients.ready.length + this.__fillRequests >=
            this.__poolOptions.min
        ) {
            return;
        }

        this.__fillRequests++;
        const client = this.newClient();
        client.connect((err) => {
            this.__fillRequests--;
            if (isUndefined(err)) {
                if (this.__clients.ready.length < this.__poolOptions.min) {
                    this.__clients.ready.unshift(client);
                    if (this.__clients.ready.length < this.__poolOptions.min)
                        this.refill();
                } else {
                    client.close(() => {});
                }
            } else {
                throw new Error(err);
            }
        });
    }

    acquire(): Promise<Client> {
        return new Promise(
            (resolve: (arg: Client) => void, reject: (arg: any) => void) => {
                const client = this.__clients.ready.pop();
                if (this.__clients.ready.length < this.__poolOptions.min)
                    this.refill();
                if (client instanceof Client) {
                    this.__clients.active.set(client.id, client);
                    resolve(client);
                } else {
                    const newClient: Client = this.newClient();
                    newClient.connect((err: any) => {
                        if (!isUndefined(err)) {
                            reject(err);
                        } else {
                            this.__clients.active.set(newClient.id, newClient);
                            resolve(newClient);
                        }
                    });
                }
            }
        );
    }

    release(client: Client): Promise<void> {
        return new Promise(
            (
                resolve: (arg: number) => void,
                reject: (arg: TypeError) => void
            ) => {
                if (!(client instanceof Client))
                    reject(
                        new TypeError(
                            "Pool release() method requires a client instance as argument"
                        )
                    );
                const id = client.id;
                client.close(() => {
                    this.__clients.active.delete(id);
                    if (this.__clients.ready.length < this.__poolOptions.min) {
                        this.refill();
                    }
                    resolve(id);
                });
            }
        );
    }

    releaseAll(): Promise<number> {
        return new Promise((resolve: (arg: number) => void) => {
            const toBeClosed =
                this.__clients.ready.length + this.__clients.active.size;
            let closed = 0;
            for (let [id, client] of this.__clients.active.entries()) {
                client.close(() => {
                    closed++;
                    if (closed === toBeClosed) {
                        this.__clients.ready = [];
                        this.__clients.active = new Map();
                        resolve(closed);
                    }
                });
            }
            this.__clients.ready.forEach((client) =>
                client.close(() => {
                    closed++;
                    if (closed === toBeClosed) {
                        this.__clients.ready = [];
                        this.__clients.active = new Map();
                        resolve(closed);
                    }
                })
            );
        });
    }

    get status(): object {
        return {
            active: this.__clients.active.size,
            ready: this.__clients.ready.length,
            options: this.__poolOptions,
        };
    }
}

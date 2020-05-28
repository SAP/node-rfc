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
    private static Ready: Array<Client> = [];
    private static Active: Map<number, Client> = new Map();

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
    }

    newClient(): Client {
        return this.__clientOptions
            ? new Client(this.__connectionParams, this.__clientOptions)
            : new Client(this.__connectionParams);
    }

    refill() {
        if (Pool.Ready.length + this.__fillRequests >= this.__poolOptions.min) {
            return;
        }

        this.__fillRequests++;
        const client = this.newClient();
        client.connect((err) => {
            this.__fillRequests--;
            if (isUndefined(err)) {
                if (Pool.Ready.length < this.__poolOptions.min) {
                    Pool.Ready.unshift(client);
                    if (Pool.Ready.length < this.__poolOptions.min)
                        this.refill();
                } else {
                    client.close(() => {});
                }
            } else {
                throw new Error(err);
            }
        });
    }

    acquire(reqId?: Number): Promise<Client> {
        console.log(
            `    pool acquire req: ${reqId} ready: ${Pool.Ready.length} ${this.READY}`
        );
        return new Promise(
            (resolve: (arg: Client) => void, reject: (arg: any) => void) => {
                const client = Pool.Ready.pop();
                if (Pool.Ready.length < this.__poolOptions.min) this.refill();
                if (client instanceof Client) {
                    Pool.Active.set(client.id, client);
                    console.log(
                        `    pool acquire req: ${reqId} client: ${client.id}:${client._connectionHandle} ready: ${Pool.Ready.length} ${this.READY}`
                    );
                    resolve(client);
                } else {
                    const newClient: Client = this.newClient();
                    newClient.connect((err: any) => {
                        if (!isUndefined(err)) {
                            reject(err);
                        } else {
                            Pool.Active.set(newClient.id, newClient);
                            console.log(
                                `    pool acquire req: ${reqId} client: ${newClient.id}:${newClient._connectionHandle} ready: ${Pool.Ready.length} ${this.READY}`
                            );
                            console.log(this.READY);
                            resolve(newClient);
                        }
                    });
                }
            }
        );
    }

    release(client: Client, reqId?: Number): Promise<void> {
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
                Pool.Active.delete(id);
                if (Pool.Ready.length < this.__poolOptions.min) {
                    Pool.Ready.push(client);
                } else {
                    client.close(() => {});
                }
                console.log(
                    `    pool release req: ${reqId} client: ${client.id}:${client._connectionHandle} ready: ${Pool.Ready.length} ${this.READY}`
                );
                resolve(id);
            }
        );
    }

    releaseAll(): Promise<number> {
        return new Promise((resolve: (arg: number) => void) => {
            const toBeClosed = Pool.Ready.length + Pool.Active.size;
            let closed = 0;
            for (let [id, client] of Pool.Active.entries()) {
                client.close(() => {
                    closed++;
                    if (closed === toBeClosed) {
                        Pool.Ready = [];
                        Pool.Active = new Map();
                        resolve(closed);
                    }
                });
            }
            Pool.Ready.forEach((client) =>
                client.close(() => {
                    closed++;
                    if (closed === toBeClosed) {
                        Pool.Ready = [];
                        Pool.Active = new Map();
                        resolve(closed);
                    }
                })
            );
        });
    }

    get status(): object {
        return {
            active: Pool.Active.size,
            ready: Pool.Ready.length,
            options: this.__poolOptions,
        };
    }

    get READY(): Array<Number> {
        const ready = new Array<Number>();
        for (let c of Pool.Ready) ready.push(c._connectionHandle);
        return ready;
    }
}

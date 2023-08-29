// SPDX-FileCopyrightText: 2014 SAP SE Srdjan Boskovic <srdjan.boskovic@sap.com>
//
// SPDX-License-Identifier: Apache-2.0

import {
    Promise,
    noderfc_binding,
    environment,
    NodeRfcEnvironment,
} from "./noderfc-bindings";

import { RfcConnectionParameters, RfcLoggingLevel } from "./sapnwrfc";

import { Client, RfcClientBinding, RfcClientOptions } from "./sapnwrfc-client";

//
// RfcPool
//

export interface RfcPoolOptions {
    low: number;
    high: number;
    logLevel?: RfcLoggingLevel;
}

export interface RfcPoolStatus {
    ready: number;
    leased: number;
}

export interface RfcPoolConfiguration {
    connectionParameters: RfcConnectionParameters;
    clientOptions?: RfcClientOptions;
    poolOptions?: RfcPoolOptions;
}
export interface RfcPoolBinding {
    /* eslint-disable @typescript-eslint/no-misused-new */
    new (poolConfiguration: RfcPoolConfiguration): RfcPoolBinding;
    (poolConfiguration: RfcPoolConfiguration): RfcPoolBinding;
    acquire(
        clients_requested: number,
        callback: Function
    ): RfcClientBinding | Array<RfcClientBinding>;
    release(
        clients: RfcClientBinding | Array<RfcClientBinding>,
        callback: Function
    ): void;
    ready(new_ready?: number, callback?: Function): void;
    closeAll(callback?: Function): void;
    _config: {
        connectionParameters: object;
        clientOptions?: object;
        poolOptions: RfcPoolOptions;
    };
    _id: number;
    _status: RfcPoolStatus;
}
export class Pool {
    private __connectionParams: RfcConnectionParameters;
    private __clientOptions?: RfcClientOptions;
    public __poolOptions: RfcPoolOptions;

    private __pool: RfcPoolBinding;

    constructor(poolConfiguration: RfcPoolConfiguration) {
        this.__connectionParams = poolConfiguration.connectionParameters;
        this.__clientOptions = poolConfiguration.clientOptions;
        this.__poolOptions = poolConfiguration.poolOptions || {
            low: 2,
            high: 4,
        };
        this.__pool = new noderfc_binding.Pool(poolConfiguration);
    }

    acquire(
        arg1?: number | Function,
        arg2?: number | Function
    ): void | Promise<Client | Client[]> {
        let clients_requested = 1;
        let callback: Function | undefined;
        if (typeof arg1 === "number") {
            clients_requested = arg1;
            if (arg2 !== undefined) {
                if (typeof arg2 !== "function") {
                    throw new TypeError(
                        `Pool acquire() argument must be a function, received: ${typeof arg2}`
                    );
                } else {
                    callback = arg2;
                }
            }
        } else if (typeof arg1 === "function") {
            callback = arg1;

            if (arg2 !== undefined) {
                if (typeof arg2 !== "number") {
                    throw new TypeError(
                        `Pool acquire() argument must be a number, received: ${typeof arg2}`
                    );
                } else {
                    clients_requested = arg2;
                }
            }
        } else if (arg1 !== undefined) {
            throw new TypeError(
                `Pool acquire() argument must ne a number or function, received: ${typeof arg1}`
            );
        }

        if (callback === undefined) {
            return new Promise(
                (
                    resolve: (arg0: Client | Client[]) => void,
                    reject: (arg0: unknown) => void
                ) => {
                    try {
                        this.__pool.acquire(
                            clients_requested,
                            (
                                err: unknown,
                                clientBindings:
                                    | RfcClientBinding
                                    | Array<RfcClientBinding>
                            ) => {
                                if (err !== undefined) {
                                    return reject(err);
                                }

                                if (Array.isArray(clientBindings)) {
                                    const clients: Array<Client> = [];
                                    clientBindings.forEach((cb) => {
                                        clients.push(new Client(cb));
                                    });
                                    resolve(clients);
                                } else {
                                    const c = new Client(clientBindings);
                                    resolve(c);
                                }
                            }
                        );
                    } catch (ex) {
                        reject(ex);
                    }
                }
            );
        }

        try {
            this.__pool.acquire(
                clients_requested,
                (
                    err: unknown,
                    clientBindings: RfcClientBinding | Array<RfcClientBinding>
                ) => {
                    if (err !== undefined) {
                        return (callback as Function)(err) as unknown;
                    }

                    if (Array.isArray(clientBindings)) {
                        const clients: Array<Client> = [];
                        clientBindings.forEach((cb) => {
                            clients.push(new Client(cb));
                        });
                        (callback as Function)(err, clients);
                    } else {
                        const c = new Client(clientBindings);
                        (callback as Function)(err, c);
                    }
                }
            );
        } catch (ex) {
            callback(ex);
        }
    }

    release(
        tsClient: Client | Array<Client>,
        callback?: Function
    ): void | Promise<unknown> {
        if (callback !== undefined && typeof callback !== "function") {
            throw new TypeError(
                `Pool release() 2nd argument, if provided, must be a function, received: ${typeof callback}`
            );
        }

        const client_bindings: Array<RfcClientBinding> = [];
        if (Array.isArray(tsClient)) {
            tsClient.forEach((tsc) => {
                client_bindings.push(tsc.binding);
            });
        } else if (tsClient instanceof Client) {
            client_bindings.push(tsClient.binding);
        } else {
            throw new TypeError(
                `Pool release() 1st argument must be a single client or array of clients, received: ${typeof tsClient}`
            );
        }

        if (callback === undefined) {
            return new Promise((resolve, reject) => {
                try {
                    this.__pool.release(client_bindings, (err: unknown) => {
                        if (err === undefined) {
                            resolve();
                        } else {
                            reject(err);
                        }
                    });
                } catch (ex) {
                    reject(ex);
                }
            });
        }

        try {
            this.__pool.release(client_bindings, callback);
        } catch (ex) {
            callback(ex);
        }
    }

    cancel(client: Client, callback?: Function): void | Promise<unknown> {
        if (callback !== undefined && typeof callback !== "function") {
            throw new TypeError(
                `Pool cancel() 2nd argument, if provided, must be a function, received: ${typeof callback}`
            );
        }
        if (typeof callback === "function") {
            return client.cancel(callback);
        } else {
            return client.cancel();
        }
    }

    closeAll(callback?: Function): void | Promise<void> {
        if (callback === undefined) {
            return new Promise((resolve) => {
                this.__pool.closeAll(() => {
                    resolve();
                });
            });
        }

        this.__pool.closeAll(callback);
    }

    ready(
        arg1?: number | Function,
        arg2?: number | Function
    ): void | Promise<void> {
        let new_ready: number = this.__poolOptions.low;
        let callback: Function | undefined;
        if (typeof arg1 === "number") {
            new_ready = arg1;
            if (arg2 !== undefined) {
                if (typeof arg2 !== "function") {
                    throw new TypeError(
                        `Pool ready() argument must be a function, received: ${typeof arg2}`
                    );
                } else {
                    callback = arg2;
                }
            }
        } else if (typeof arg1 === "function") {
            callback = arg1;
            if (arg2 !== undefined) {
                if (typeof arg2 !== "number") {
                    throw new TypeError(
                        `Pool ready() argument must be a number, received: ${typeof arg2}`
                    );
                } else {
                    new_ready = arg2;
                }
            }
        } else if (arg1 !== undefined) {
            throw new TypeError(
                `Pool ready() argument must ne a number or function, received: ${typeof arg1}`
            );
        }

        if (callback === undefined) {
            return new Promise((resolve, reject) => {
                this.__pool.ready(new_ready, (err: unknown) => {
                    if (err == undefined) {
                        resolve();
                    } else {
                        reject(err);
                    }
                });
            });
        }

        try {
            this.__pool.ready(new_ready, callback);
        } catch (ex) {
            callback(ex);
        }
    }

    get id(): Object {
        return this.__pool._id;
    }

    get binding(): RfcPoolBinding {
        return this.__pool;
    }

    get config(): Object {
        return this.__pool._config;
    }

    get status(): RfcPoolStatus {
        return this.__pool._status;
    }

    get connectionParameters(): RfcConnectionParameters {
        return this.__connectionParams;
    }

    get clientOptions(): RfcClientOptions | undefined {
        return this.__clientOptions;
    }

    get poolOptions(): RfcPoolOptions | undefined {
        return this.__poolOptions;
    }

    get poolConfiguration(): RfcPoolConfiguration {
        return {
            connectionParameters: this.__connectionParams,
            clientOptions: this.__clientOptions,
            poolOptions: this.__poolOptions,
        };
    }

    static get environment(): NodeRfcEnvironment {
        return environment;
    }

    get environment(): NodeRfcEnvironment {
        return environment;
    }
}

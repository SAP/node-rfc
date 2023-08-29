// SPDX-FileCopyrightText: 2014 SAP SE Srdjan Boskovic <srdjan.boskovic@sap.com>
//
// SPDX-License-Identifier: Apache-2.0

import {
    //Promise,
    noderfc_binding,
    environment,
    NodeRfcEnvironment,
} from "./noderfc-bindings";

import {
    RfcConnectionParameters,
    RfcLoggingLevel,
    RfcParameterDirection,
    RfcObject,
} from "./sapnwrfc";

//
// RfcClient
//

export type RfcClientOptions = {
    bcd?: string | Function;
    date?: { toABAP: Function; fromABAP: Function };
    time?: { toABAP: Function; fromABAP: Function };
    filter?: RfcParameterDirection;
    stateless?: boolean;
    timeout?: number;
    logLevel?: RfcLoggingLevel;
};

export type RfcCallOptions = {
    notRequested?: Array<string>;
    timeout?: number;
};

interface RfcConnectionInfo {
    dest: string;
    host: string;
    partnerHost: string;
    sysNumber: string;
    sysId: string;
    client: string;
    user: string;
    language: string;
    trace: string;
    isoLanguage: string;
    codepage: string;
    partnerCodepage: string;
    rfcRole: string;
    type: string;
    partnerType: string;
    rel: string;
    partnerRel: string;
    kernelRel: string;
    cpicConvId: string;
    progName: string;
    partnerBytesPerChar: string;
    partnerSystemCodepage: string;
    partnerIP: string;
    partnerIPv6: string;
    //reserved: string;
}

export type RfcClientConfig = {
    connectionParameters: RfcConnectionParameters;
    clientOptions?: RfcClientOptions;
};

/* eslint-disable @typescript-eslint/no-misused-new */
export interface RfcClientBinding {
    new (
        connectionParameters: RfcConnectionParameters,
        clientOptions?: RfcClientOptions
    ): RfcClientBinding;
    (
        connectionParameters: RfcConnectionParameters,
        options?: RfcClientOptions
    ): RfcClientBinding;
    _id: number;
    _alive: boolean;
    _connectionHandle: number;
    _pool_id: number;
    _config: RfcClientConfig;
    connectionInfo(): RfcConnectionInfo;
    open(callback: Function): void;
    close(callback: Function): void;
    resetServerContext(callback: Function): void;
    ping(callback: Function): void;
    cancel(callback: Function): void;
    invoke(
        rfmName: string,
        rfmParams: RfcObject,
        callback: Function,
        callOptions?: RfcCallOptions
    ): void;
    release(oneClientBinding: [RfcClientBinding], callback: Function): void;
}

export class Client {
    private __client: RfcClientBinding;

    constructor(
        arg1: RfcClientBinding | RfcConnectionParameters,
        clientOptions?: RfcClientOptions
    ) {
        if (arg1 === undefined) {
            throw new TypeError(`Client constructor requires an argument`);
        }
        if (arg1["_pool_id"] !== undefined && arg1["_pool_id"] > 0) {
            this.__client = arg1 as RfcClientBinding;
        } else {
            this.__client = clientOptions
                ? new noderfc_binding.Client(
                      arg1 as RfcConnectionParameters,
                      clientOptions
                  )
                : new noderfc_binding.Client(arg1 as RfcConnectionParameters);
        }
    }

    static get environment(): NodeRfcEnvironment {
        return environment;
    }

    get environment(): NodeRfcEnvironment {
        return environment;
    }

    get binding(): RfcClientBinding {
        return this.__client;
    }

    get id(): number {
        return this.__client._id;
    }

    get alive(): boolean {
        return this.__client._alive;
    }

    get connectionHandle(): number {
        return this.__client._connectionHandle;
    }

    get pool_id(): number {
        return this.__client._pool_id;
    }

    get config(): RfcClientConfig {
        return this.__client._config;
    }

    get _id(): string {
        return `${this.__client._id} handle: ${
            this.__client._connectionHandle
        } ${
            this.__client._pool_id
                ? `[m] pool: ${this.__client._pool_id} `
                : "[d]"
        }`;
    }

    get connectionInfo(): RfcConnectionInfo {
        return this.__client.connectionInfo();
    }

    static checkCallbackArg(method: string, callback?: Function) {
        if (callback !== undefined && typeof callback !== "function") {
            throw new TypeError(
                `Client ${method}() argument, if provided, must be a Function. Received: ${typeof callback}`
            );
        }
    }

    // for backwards compatibility only, to be deprecated
    connect(callback?: Function): void | Promise<Client> {
        Client.checkCallbackArg("connect", callback);
        return this.open(callback);
    }

    open(callback?: Function): void | Promise<Client> {
        Client.checkCallbackArg("open", callback);
        if (typeof callback === "function") {
            try {
                this.__client.open(callback);
            } catch (ex) {
                callback(ex);
            }
        } else {
            return new Promise((resolve, reject) => {
                try {
                    this.__client.open((err: unknown) => {
                        if (err !== undefined) {
                            reject(err);
                        } else {
                            resolve(this);
                        }
                    });
                } catch (ex) {
                    reject(ex);
                }
            });
        }
    }

    ping(callback?: Function): void | Promise<boolean> {
        Client.checkCallbackArg("ping", callback);

        if (typeof callback === "function") {
            try {
                this.__client.ping(callback);
            } catch (ex) {
                callback(ex);
            }
        } else {
            return new Promise((resolve, reject) => {
                try {
                    this.__client.ping((err: unknown, res: boolean) => {
                        if (err === undefined) {
                            resolve(res);
                        } else {
                            reject(err);
                        }
                    });
                } catch (ex) {
                    reject(ex);
                }
            });
        }
    }

    close(callback?: Function): void | Promise<void> {
        Client.checkCallbackArg("close", callback);

        if (typeof callback === "function") {
            try {
                this.__client.close(callback);
            } catch (ex) {
                callback(ex);
            }
        } else {
            return new Promise((resolve, reject) => {
                try {
                    this.__client.close((err: unknown) => {
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
    }

    cancel(callback?: Function): void | Promise<void> {
        Client.checkCallbackArg("cancel", callback);

        if (typeof callback === "function") {
            try {
                this.__client.cancel(callback);
            } catch (ex) {
                callback(ex);
            }
        } else {
            return new Promise((resolve, reject) => {
                try {
                    this.__client.cancel((err: unknown) => {
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
    }

    resetServerContext(callback?: Function): void | Promise<void> {
        Client.checkCallbackArg("resetServerContext", callback);

        if (typeof callback === "function") {
            try {
                this.__client.resetServerContext(callback);
            } catch (ex) {
                callback(ex);
            }
        } else {
            return new Promise((resolve, reject) => {
                try {
                    this.__client.resetServerContext((err: unknown) => {
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
    }

    release(callback?: Function): void | Promise<void> {
        Client.checkCallbackArg("release");

        if (typeof callback === "function") {
            try {
                this.__client.release([this.__client], callback);
            } catch (ex) {
                callback(ex);
            }
        } else {
            return new Promise((resolve, reject) => {
                try {
                    this.__client.release([this.__client], (err: unknown) => {
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
    }

    call(
        rfmName: string,
        rfmParams: RfcObject,
        callOptions: RfcCallOptions = {}
    ): Promise<RfcObject> {
        return new Promise(
            (
                resolve: (arg0: RfcObject) => void,
                reject: (arg0: TypeError) => void
            ) => {
                if (arguments.length < 2) {
                    reject(
                        new TypeError(
                            "Please provide remote function module name and parameters as arguments"
                        )
                    );
                }

                if (typeof rfmName !== "string") {
                    reject(
                        new TypeError(
                            "First argument (remote function module name) must be an string"
                        )
                    );
                }

                if (typeof rfmParams !== "object") {
                    reject(
                        new TypeError(
                            "Second argument (remote function module parameters) must be an object"
                        )
                    );
                }

                if (
                    callOptions !== undefined &&
                    typeof callOptions !== "object"
                ) {
                    reject(
                        new TypeError("Call options argument must be an object")
                    );
                }
                try {
                    this.invoke(
                        rfmName,
                        rfmParams,
                        (err: unknown, res: RfcObject) => {
                            if (err !== undefined && err !== null) {
                                reject(err as TypeError);
                            } else {
                                resolve(res);
                            }
                        },
                        callOptions
                    );
                } catch (ex) {
                    reject(ex as TypeError);
                }
            }
        );
    }

    invoke(
        rfmName: string,
        rfmParams: RfcObject,
        callback: Function,
        callOptions?: RfcCallOptions
    ) {
        try {
            if (typeof callback !== "function") {
                throw new TypeError("Callback function must be supplied");
            }

            if (arguments.length < 3) {
                throw new TypeError(
                    "Client invoke() argument missing: RFM name, parameters or callback"
                );
            }

            if (typeof rfmName !== "string") {
                throw new TypeError(
                    "Client invoke() 1st argument (remote function module name) must be an string"
                );
            }

            if (typeof rfmParams !== "object") {
                throw new TypeError(
                    "Client invoke() 2nd argument (remote function module parameters) must be an object"
                );
            }

            if (arguments.length === 4 && typeof callOptions !== "object") {
                throw new TypeError("Call options argument must be an object");
            }

            const clientOptions = this.config.clientOptions;
            let timeout = 0,
                callbackFunction = callback;
            if (callOptions && callOptions.timeout) {
                timeout = callOptions.timeout;
            }
            if (timeout === 0 && clientOptions && clientOptions.timeout) {
                timeout = clientOptions.timeout;
            }
            if (timeout > 0) {
                const cancelTimeout = setTimeout(() => {
                    /* eslint-disable @typescript-eslint/no-unused-vars */
                    (this.cancel as Function)(
                        (_err: unknown, _res: unknown) => {
                            // _res like
                            // { connectionHandle: 140414048978432, result: 'cancelled' }
                        }
                    );
                }, timeout * 1000);
                callbackFunction = (err: unknown, res: unknown) => {
                    clearTimeout(cancelTimeout);
                    callback(err, res);
                };
            }

            // check rfm parmeters' names
            for (const rfmParamName of Object.keys(rfmParams)) {
                if (rfmParamName.length === 0)
                    throw new TypeError(
                        `Empty RFM parameter name when calling "${rfmName}"`
                    );
                if (!rfmParamName.match(/^[a-zA-Z0-9_]*$/))
                    throw new TypeError(
                        `RFM parameter name invalid: "${rfmParamName}" when calling "${rfmName}"`
                    );
            }

            this.__client.invoke(
                rfmName,
                rfmParams,
                callbackFunction,
                callOptions
            );
        } catch (ex) {
            if (typeof callback !== "function") {
                throw ex;
            } else {
                callback(ex);
            }
        }
    }
}

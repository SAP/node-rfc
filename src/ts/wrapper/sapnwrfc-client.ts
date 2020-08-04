// SPDX-FileCopyrightText: 2014 SAP SE Srdjan Boskovic <srdjan.boskovic@sap.com>
//
// SPDX-License-Identifier: Apache-2.0

import { isUndefined } from "util";
import {
    //Promise,
    noderfc_binding,
    environment,
    NodeRfcEnvironment,
    USAGE_URL,
} from "./noderfc-bindings";

//
// RfcClient
//

enum EnumSncQop {
    DigSig = "1",
    DigSigEnc = "2",
    DigSigEncUserAuth = "3",
    BackendDefault = "8",
    Maximum = "9",
}

enum EnumTrace {
    Off = "0",
    Brief = "1",
    Verbose = "2",
    Full = "3",
}
export interface RfcConnectionParameters {
    // general
    saprouter?: string;
    snc_lib?: string;
    snc_myname?: string;
    snc_partnername?: string;
    snc_qop?: EnumSncQop;
    trace?: EnumTrace;
    // not supported
    // pcs
    // codepage
    // noCompression

    // client
    user?: string;
    passwd?: string;
    client: string;
    lang?: string;
    mysapsso2?: string;
    getsso2?: string;
    x509cert?: string;

    //
    // destination
    //

    // specific server
    dest?: string;
    ashost?: string;
    sysnr?: string;
    gwhost?: string;
    gwserv?: string;

    // load balancing
    //dest?: string,
    group?: string;
    r3name?: string;
    sysid?: string;
    mshost?: string;
    msserv?: string;

    // gateway
    //dest?: string,
    tpname?: string;
    program_id?: string;
    //gwhost?: string,
    //gwserv?: string,
}

enum RfcParameterDirection {
    RFC_IMPORT = 0x01, ///< Import parameter. This corresponds to ABAP IMPORTING parameter.
    RFC_EXPORT = 0x02, ///< Export parameter. This corresponds to ABAP EXPORTING parameter.
    RFC_CHANGING = RFC_IMPORT | RFC_EXPORT, ///< Import and export parameter. This corresponds to ABAP CHANGING parameter.
    RFC_TABLES = 0x04 | RFC_CHANGING, ///< Table parameter. This corresponds to ABAP TABLES parameter.
}
export interface RfcClientOptions {
    bcd?: string | Function;
    date?: Function;
    time?: Function;
    filter?: RfcParameterDirection;
}

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

export type RfcVariable = string | number | Buffer;
export type RfcArray = Array<RfcVariable>;
export type RfcStructure = {
    [key: string]: RfcVariable | RfcStructure | RfcTable;
};
export type RfcTable = Array<RfcStructure>;
export type RfcParameterValue =
    | RfcVariable
    | RfcArray
    | RfcStructure
    | RfcTable;
export type RfcObject = { [key: string]: RfcParameterValue };
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
    _config: {
        connectionParameters: object;
        clientOptions?: object;
    };
    connectionInfo(): RfcConnectionInfo;
    open(callback: Function): void;
    close(callback: Function): void;
    resetServerContext(callback: Function): void;
    ping(callback: Function): void;
    invoke(
        rfmName: string,
        rfmParams: RfcObject,
        callback: Function,
        callOptions?: object
    ): void;
    release(oneClientBinding: [RfcClientBinding], callback: Function): void;
}

export class Client {
    private __client: RfcClientBinding;

    constructor(
        arg1: RfcClientBinding | RfcConnectionParameters,
        clientOptions?: RfcClientOptions
    ) {
        if (isUndefined(arg1)) {
            throw new TypeError(`Client constructor requires an argument`);
        }
        if (!isUndefined(arg1["_pool_id"]) && arg1["_pool_id"] > 0) {
            this.__client = <RfcClientBinding>arg1;
        } else {
            this.__client = clientOptions
                ? new noderfc_binding.Client(
                      <RfcConnectionParameters>arg1,
                      clientOptions
                  )
                : new noderfc_binding.Client(<RfcConnectionParameters>arg1);
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

    get config(): Object {
        return this.__client._config;
    }

    get _id(): string {
        return `${this.__client._id} handle: ${
            this.__client._connectionHandle
        } ${
            this.__client._pool_id
                ? "[m] pool: " + this.__client._pool_id
                : "[d]"
        }`;
    }

    get connectionInfo(): RfcConnectionInfo {
        return this.__client.connectionInfo();
    }

    checkCallbackArg(method: string, callback?: Function) {
        if (!isUndefined(callback) && typeof callback !== "function") {
            throw new TypeError(
                `Client ${method}() argument, if provided, must be a Function. Received: ${typeof callback}`
            );
        }
    }

    // for backwards compatibility only, to be deprecated
    connect(callback?: Function): void | Promise<Client> {
        this.checkCallbackArg("connect", callback);
        return this.open(callback);
    }

    open(callback?: Function): void | Promise<Client> {
        this.checkCallbackArg("open", callback);
        if (!isUndefined(callback)) {
            try {
                this.__client.open(callback);
            } catch (ex) {
                callback(ex);
            }
        } else {
            return new Promise((resolve, reject) => {
                try {
                    this.__client.open((err) => {
                        if (!isUndefined(err)) {
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
        this.checkCallbackArg("ping", callback);

        if (!isUndefined(callback)) {
            try {
                this.__client.ping(callback);
            } catch (ex) {
                callback(ex);
            }
        } else {
            return new Promise((resolve, reject) => {
                try {
                    this.__client.ping((err: any, res: boolean) => {
                        if (isUndefined(err)) {
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
        this.checkCallbackArg("close", callback);

        if (!isUndefined(callback)) {
            try {
                this.__client.close(callback);
            } catch (ex) {
                callback(ex);
            }
        } else {
            return new Promise((resolve, reject) => {
                try {
                    this.__client.close((err) => {
                        if (isUndefined(err)) {
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
        this.checkCallbackArg("resetServerContext", callback);

        if (!isUndefined(callback)) {
            try {
                this.__client.resetServerContext(callback);
            } catch (ex) {
                callback(ex);
            }
        } else {
            return new Promise((resolve, reject) => {
                try {
                    this.__client.resetServerContext((err) => {
                        if (isUndefined(err)) {
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
        this.checkCallbackArg("release");

        if (!isUndefined(callback)) {
            try {
                this.__client.release([this.__client], callback);
            } catch (ex) {
                callback(ex);
            }
        } else {
            return new Promise((resolve, reject) => {
                try {
                    this.__client.release([this.__client], (err) => {
                        if (isUndefined(err)) {
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
        callOptions: RfcClientOptions = {}
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
                    !isUndefined(callOptions) &&
                    typeof callOptions !== "object"
                ) {
                    reject(
                        new TypeError("Call options argument must be an object")
                    );
                }

                try {
                    this.__client.invoke(
                        rfmName,
                        rfmParams,
                        (err: any, res: RfcObject) => {
                            if (!isUndefined(err)) {
                                reject(err);
                            } else {
                                resolve(res);
                            }
                        },
                        callOptions
                    );
                } catch (ex) {
                    reject(ex);
                }
            }
        );
    }

    invoke(
        rfmName: string,
        rfmParams: RfcObject,
        callback: Function,
        callOptions?: RfcClientOptions
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

            this.__client.invoke(rfmName, rfmParams, callback, callOptions);
        } catch (ex) {
            if (typeof callback !== "function") {
                throw ex;
            } else {
                callback(ex);
            }
        }
    }
}

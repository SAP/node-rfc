// SPDX-FileCopyrightText: 2014 SAP SE Srdjan Boskovic <srdjan.boskovic@sap.com>
//
// SPDX-License-Identifier: Apache-2.0

import Decimal from "decimal.js";

import {
    //Promise,
    noderfc_binding,
    environment,
    NodeRfcEnvironment,
    RfcLoggingLevel,
} from "./noderfc-bindings";

//
// RfcClient
//

export enum EnumSncQop {
    DigSig = "1",
    DigSigEnc = "2",
    DigSigEncUserAuth = "3",
    BackendDefault = "8",
    Maximum = "9",
}

export enum EnumTrace {
    Off = "0",
    Brief = "1",
    Verbose = "2",
    Full = "3",
}

type RfcConnectionParametersAllowed =
    | "ABAP_DEBUG"
    | "ALIAS_USER"
    | "ASHOST"
    | "ASXML"
    | "CFIT"
    | "CLIENT"
    | "CODEPAGE"
    | "COMPRESSION_TYPE"
    | "DELTA"
    | "DEST"
    | "EXTIDDATA"
    | "EXTIDTYPE"
    | "GETSSO2"
    | "GROUP"
    | "GWHOST"
    | "GWSERV"
    | "LANG"
    | "LCHECK"
    | "LOGON_GROUP_CHECK_INTERVAL"
    | "MAX_REG_COUNT"
    | "MSHOST"
    | "MSSERV"
    | "MYSAPSSO2"
    | "NO_COMPRESSION"
    | "ON_CCE"
    | "PASSWD"
    | "PASSWORD_CHANGE_ENFORCED"
    | "PCS"
    | "PROGRAM_ID"
    | "PROXY_HOST"
    | "PROXY_PASSWD"
    | "PROXY_PORT"
    | "PROXY_USER"
    | "R3NAME"
    | "REG_COUNT"
    | "SAPLOGON_ID"
    | "SAPROUTER"
    | "SERIALIZATION_FORMAT"
    | "SERVER_NAME"
    | "SNC_LIB"
    | "SNC_MODE"
    | "SNC_MYNAME"
    | "SNC_PARTNERNAME"
    | "SNC_PARTNER_NAMES"
    | "SNC_QOP"
    | "SNC_SSO"
    | "SYSID"
    | "SYSNR"
    | "SYS_IDS"
    | "TLS_CLIENT_CERTIFICATE_LOGON"
    | "TLS_CLIENT_PSE"
    | "TLS_SERVER_PARTNER_AUTH"
    | "TLS_SERVER_PSE"
    | "TLS_TRUST_ALL"
    | "TPNAME"
    | "TRACE"
    | "USER"
    | "USE_REPOSITORY_ROUNDTRIP_OPTIMIZATION"
    | "USE_SAPGUI"
    | "USE_SYMBOLIC_NAMES"
    | "USE_TLS"
    | "WSHOST"
    | "WSPORT"
    | "X509CERT"
    | "abap_debug"
    | "alias_user"
    | "ashost"
    | "asxml"
    | "cfit"
    | "client"
    | "codepage"
    | "compression_type"
    | "delta"
    | "dest"
    | "extiddata"
    | "extidtype"
    | "getsso2"
    | "group"
    | "gwhost"
    | "gwserv"
    | "lang"
    | "lcheck"
    | "logon_group_check_interval"
    | "max_reg_count"
    | "mshost"
    | "msserv"
    | "mysapsso2"
    | "no_compression"
    | "on_cce"
    | "passwd"
    | "password_change_enforced"
    | "pcs"
    | "program_id"
    | "proxy_host"
    | "proxy_passwd"
    | "proxy_port"
    | "proxy_user"
    | "r3name"
    | "reg_count"
    | "saplogon_id"
    | "saprouter"
    | "serialization_format"
    | "server_name"
    | "snc_lib"
    | "snc_mode"
    | "snc_myname"
    | "snc_partnername"
    | "snc_partner_names"
    | "snc_qop"
    | "snc_sso"
    | "sysid"
    | "sysnr"
    | "sys_ids"
    | "tls_client_certificate_logon"
    | "tls_client_pse"
    | "tls_server_partner_auth"
    | "tls_server_pse"
    | "tls_trust_all"
    | "tpname"
    | "trace"
    | "user"
    | "use_repository_roundtrip_optimization"
    | "use_sapgui"
    | "use_symbolic_names"
    | "use_tls"
    | "wshost"
    | "wsport"
    | "x509cert";

enum RfcParameterDirection {
    RFC_IMPORT = 0x01, ///< Import parameter. This corresponds to ABAP IMPORTING parameter.
    RFC_EXPORT = 0x02, ///< Export parameter. This corresponds to ABAP EXPORTING parameter.
    RFC_CHANGING = RFC_IMPORT | RFC_EXPORT, ///< Import and export parameter. This corresponds to ABAP CHANGING parameter.
    RFC_TABLES = 0x04 | RFC_CHANGING, ///< Table parameter. This corresponds to ABAP TABLES parameter.
}

export type RfcConnectionParameters = Partial<
    Record<RfcConnectionParametersAllowed, string>
>;

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

export type RfcVariable =
    | string
    | number
    | Buffer
    | Date
    | Decimal
    | Uint8Array
    | Uint16Array
    | Uint32Array;
export type RfcArray = Array<RfcVariable>;
export type RfcStructure = {
    [key: string]: RfcVariable | RfcStructure | RfcTable;
};
export type RfcTable = Array<RfcStructure | RfcVariable>;
export type RfcParameterValue =
    | RfcVariable
    | RfcArray
    | RfcStructure
    | RfcTable;
export type RfcObject = { [key: string]: RfcParameterValue };

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

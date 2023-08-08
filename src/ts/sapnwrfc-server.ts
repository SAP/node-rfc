// SPDX-FileCopyrightText: 2014 SAP SE Srdjan Boskovic <srdjan.boskovic@sap.com>
//
// SPDX-License-Identifier: Apache-2.0

import {
    //Promise,
    noderfc_binding,
    environment,
    NodeRfcEnvironment,
} from "./noderfc-bindings";

import { RfcConnectionParameters } from "./sapnwrfc-client";

//
// RfcServer
//

export enum EnumLogClass {
    client = 0,
    pool = 1,
    throughput = 2,
    server = 3,
}

export enum EnumLogSeverity {
    off = 0,
    error = 1,
    warning = 2,
    info = 3,
}

export type RfcServerOptions = {
    logging?: boolean;
};

export type RfcServerConfiguration = {
    serverConnection: RfcConnectionParameters;
    clientConnection: RfcConnectionParameters;
    serverOptions?: RfcServerOptions;
};

/* eslint-disable @typescript-eslint/no-misused-new */
export interface RfcServerBinding {
    new (serverConfiguration: RfcServerConfiguration): RfcServerBinding;
    (serverConfiguration: RfcServerConfiguration): RfcServerBinding;
    _id: number;
    _alive: boolean;
    _server_conn_handle: number;
    _client_conn_handle: number;
    start(callback: Function): void;
    stop(callback: Function): void;
    addFunction(
        abapFunctionName: string,
        jsFunction: Function,
        callback: Function
    ): void;
    removeFunction(abapFunctionName: string, callback: Function): void;
    getFunctionDescription(rfmName: string, callback: Function): void;
}

export class Server {
    private __server: RfcServerBinding;

    constructor(serverConfiguration: RfcServerConfiguration) {
        this.__server = new noderfc_binding.Server({
            serverConnection: serverConfiguration.serverConnection,
            clientConnection: serverConfiguration.clientConnection,
            serverOptions: serverConfiguration.serverOptions || {},
        });
    }

    start(callback?: Function): void | Promise<void> {
        if (callback === undefined) {
            return new Promise((resolve, reject) => {
                this.__server.start((err: unknown) => {
                    if (err === undefined) {
                        resolve();
                    } else {
                        reject(err);
                    }
                });
            });
        }

        this.__server.start(callback);
    }

    stop(callback?: Function): void | Promise<void> {
        if (callback === undefined) {
            return new Promise((resolve, reject) => {
                this.__server.stop((err: unknown) => {
                    if (err === undefined) {
                        resolve();
                    } else {
                        reject(err);
                    }
                });
            });
        }

        this.__server.stop(callback);
    }

    addFunction(
        abapFunctionName: string,
        jsFunction: Function,
        callback?: Function
    ): void | Promise<void> {
        if (callback === undefined) {
            return new Promise((resolve, reject) => {
                this.__server.addFunction(
                    abapFunctionName,
                    jsFunction,
                    (err: unknown) => {
                        if (err === undefined) {
                            resolve();
                        } else {
                            reject(err);
                        }
                    }
                );
            });
        }

        this.__server.addFunction(abapFunctionName, jsFunction, callback);
    }

    removeFunction(
        abapFunctionName: string,
        callback?: Function
    ): void | Promise<void> {
        if (callback === undefined) {
            return new Promise((resolve, reject) => {
                this.__server.removeFunction(
                    abapFunctionName,
                    (err: unknown) => {
                        if (err === undefined) {
                            resolve();
                        } else {
                            reject(err);
                        }
                    }
                );
            });
        }

        this.__server.removeFunction(abapFunctionName, callback);
    }

    getFunctionDescription(rfmName: string, callback?: Function) {
        if (callback === undefined) {
            return new Promise((resolve, reject) => {
                this.__server.getFunctionDescription(
                    rfmName,
                    (err: unknown, rfmFunctionDescription: object) => {
                        if (err === undefined) {
                            resolve(rfmFunctionDescription);
                        } else {
                            reject(err);
                        }
                    }
                );
            });
        }

        this.__server.getFunctionDescription(rfmName, callback);
    }

    static get environment(): NodeRfcEnvironment {
        return environment;
    }

    get environment(): NodeRfcEnvironment {
        return environment;
    }

    get binding(): RfcServerBinding {
        return this.__server;
    }

    get id(): number {
        return this.__server._id;
    }

    get alive(): boolean {
        return this.__server._alive;
    }

    get server_connection(): number {
        return this.__server._server_conn_handle;
    }

    get client_connection(): number {
        return this.__server._client_conn_handle;
    }
}

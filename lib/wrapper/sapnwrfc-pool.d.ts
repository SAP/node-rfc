import { NodeRfcEnvironment } from "./noderfc-bindings";
import { Client, RfcClientBinding, RfcConnectionParameters, RfcClientOptions } from "./sapnwrfc-client";
export interface RfcPoolOptions {
    low: number;
    high: number;
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
    new (poolConfiguration: RfcPoolConfiguration): RfcPoolBinding;
    (poolConfiguration: RfcPoolConfiguration): RfcPoolBinding;
    acquire(clients_requested: number, callback: Function): RfcClientBinding | Array<RfcClientBinding>;
    release(clients: RfcClientBinding | Array<RfcClientBinding>, callback: Function): void;
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
export declare class Pool {
    private __connectionParams;
    private __clientOptions?;
    __poolOptions: RfcPoolOptions;
    private __pool;
    constructor(poolConfiguration: RfcPoolConfiguration);
    acquire(arg1?: number | Function, arg2?: number | Function): void | Promise<Client>;
    release(tsClient: Client | Array<Client>, callback?: Function): void | Promise<any>;
    cancel(client: Client, callback?: Function): void | Promise<any>;
    closeAll(callback?: Function): void | Promise<void>;
    ready(arg1?: number | Function, arg2?: number | Function): void | Promise<void>;
    get id(): Object;
    get binding(): RfcPoolBinding;
    get config(): Object;
    get status(): RfcPoolStatus;
    get connectionParameters(): RfcConnectionParameters;
    get clientOptions(): RfcClientOptions | undefined;
    get poolOptions(): RfcPoolOptions | undefined;
    get poolConfiguration(): RfcPoolConfiguration;
    static get environment(): NodeRfcEnvironment;
    get environment(): NodeRfcEnvironment;
}

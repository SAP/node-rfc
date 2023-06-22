import { NodeRfcEnvironment } from "./noderfc-bindings";
import { RfcConnectionParameters, RfcClientOptions } from "./sapnwrfc-client";
export interface RfcServerBinding {
    new (serverParams: RfcConnectionParameters, clientParams: RfcConnectionParameters, clientOptions?: RfcClientOptions): RfcServerBinding;
    (serverParams: RfcConnectionParameters, clientParams: RfcConnectionParameters, clientOptions?: RfcClientOptions): RfcServerBinding;
    _id: number;
    _alive: boolean;
    _server_conn_handle: number;
    _client_conn_handle: number;
    start(callback: Function): void;
    stop(callback: Function): void;
    addFunction(abapFunctionName: string, jsFunction: Function, callback: Function): void;
    removeFunction(abapFunctionName: string, callback: Function): void;
    getFunctionDescription(rfmName: string, callback: Function): void;
}
export declare class Server {
    private __server;
    constructor(serverParams: RfcConnectionParameters, clientParams: RfcConnectionParameters, clientOptions?: RfcClientOptions);
    start(callback?: Function): void | Promise<void>;
    stop(callback?: Function): void | Promise<void>;
    addFunction(abapFunctionName: string, jsFunction: Function, callback?: Function): void | Promise<void>;
    removeFunction(abapFunctionName: string, callback?: Function): void | Promise<void>;
    getFunctionDescription(rfmName: string, callback?: Function): Promise<unknown> | undefined;
    static get environment(): NodeRfcEnvironment;
    get environment(): NodeRfcEnvironment;
    get binding(): RfcServerBinding;
    get id(): number;
    get alive(): boolean;
    get server_connection(): number;
    get client_connection(): number;
}

/// <reference types="node" />
import { RfcThroughputBinding } from "./sapnwrfc-throughput";
export interface NWRfcBinding {
    Client: RfcClientBinding;
    Throughput: RfcThroughputBinding;
    verbose(): this;
}
declare let binding: NWRfcBinding;
export { binding };
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
}
interface RfcClientVersion {
    major: string;
    minor: string;
    patch: string;
    binding: string;
}
export interface RfcClientOptions {
    bcd: string | Function;
    date: Function;
    time: Function;
}
export interface RfcClientBinding {
    new (connectionParameters: RfcConnectionParameters, options?: RfcClientOptions): RfcClientBinding;
    (connectionParameters: RfcConnectionParameters): RfcClientBinding;
    connect(callback: Function): any;
    invoke(rfmName: string, rfmParams: RfcObject, callback: Function, callOptions?: object): any;
    ping(callback: Function | undefined): void | Promise<void>;
    close(callback: Function | undefined): void | Promise<void>;
    reopen(callback: Function | undefined): void | Promise<void>;
    isAlive(): boolean;
    connectionInfo(): RfcConnectionInfo;
    id: number;
    runningRFCCalls: number;
    _connectionHandle: number;
    version: RfcClientVersion;
    options: RfcClientOptions;
    status: RfcClientStatus;
}
declare enum EnumSncQop {
    DigSig = "1",
    DigSigEnc = "2",
    DigSigEncUserAuth = "3",
    BackendDefault = "8",
    Maximum = "9"
}
declare enum EnumTrace {
    Off = "0",
    Brief = "1",
    Verbose = "2",
    Full = "3"
}
export interface RfcCallOptions {
    notRequested?: Array<String>;
    timeout?: number;
}
export interface RfcConnectionParameters {
    saprouter?: string;
    snc_lib?: string;
    snc_myname?: string;
    snc_partnername?: string;
    snc_qop?: EnumSncQop;
    trace?: EnumTrace;
    user?: string;
    passwd?: string;
    client: string;
    lang?: string;
    mysapsso2?: string;
    getsso2?: string;
    x509cert?: string;
    dest?: string;
    ashost?: string;
    sysnr?: string;
    gwhost?: string;
    gwserv?: string;
    group?: string;
    r3name?: string;
    sysid?: string;
    mshost?: string;
    msserv?: string;
    tpname?: string;
    program_id?: string;
}
export declare type RfcVariable = string | number | Buffer;
export declare type RfcArray = Array<RfcVariable>;
export declare type RfcStructure = {
    [key: string]: RfcVariable | RfcStructure | RfcTable;
};
export declare type RfcTable = Array<RfcStructure>;
export declare type RfcParameterValue = RfcVariable | RfcArray | RfcStructure | RfcTable;
export declare type RfcObject = {
    [key: string]: RfcParameterValue;
};
export interface RfcClientStatus {
    created: number;
    lastcall: number;
    lastopen: number;
    lastclose: number;
}
export declare class Client {
    private __client;
    private __status;
    constructor(connectionParams: RfcConnectionParameters, options?: RfcClientOptions);
    open(): Promise<Client>;
    reopen(callback?: Function): Promise<Client> | any;
    close(callback?: Function): Promise<void> | any;
    call(rfmName: string, rfmParams: RfcObject, callOptions?: RfcCallOptions): Promise<RfcObject>;
    connect(callback: Function): void;
    invoke(rfmName: string, rfmParams: RfcObject, callback: Function, callOptions?: object): void;
    ping(callback?: Function): Promise<boolean> | any;
    readonly isAlive: boolean;
    readonly connectionInfo: RfcConnectionInfo;
    readonly id: number;
    readonly runningRFCCalls: number;
    readonly _connectionHandle: number;
    readonly status: RfcClientStatus;
    readonly version: RfcClientVersion;
    readonly options: RfcClientOptions;
}

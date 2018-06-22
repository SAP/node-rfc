export interface RfcCallOptions {
    notRequested?: Array<String>;
    timeout?: number;
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
export declare class Client {
    private __client;
    constructor(connectionParams: RfcConnectionParameters);
    open(): Promise<Client>;
    call(rfcName: string, rfcParams: object, callOptions?: RfcCallOptions): Promise<any>;
    connect(callback: Function): void;
    invoke(rfcName: string, rfcParams: object, callback: Function, callOptions?: object): void;
    close(): object;
    reopen(callback: Function): void;
    ping(): void;
    readonly connectionInfo: object;
    readonly isAlive: boolean;
    readonly id: number;
    readonly version: object;
}
export {};

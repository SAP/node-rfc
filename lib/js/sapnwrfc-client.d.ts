declare class Client {
    private __connectionParams;
    private __client;
    constructor(connectionParams: object);
    open(): Promise<{}>;
    call(rfcName: string, rfcParams: object, callOptions?: object): Promise<{}>;
    connect(callback: Function): void;
    invoke(rfcName: string, rfcParams: object, callback: Function, callOptions?: object): void;
    close(): object;
    reopen(callback: Function): void;
    ping(): void;
    connectionInfo(): object;
    isAlive(): boolean;
    readonly id: number;
    readonly version: object;
}
export = Client;

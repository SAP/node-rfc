import { Client, RfcConnectionParameters, RfcClientOptions } from "./sapnwrfc-client";
export interface RfcPoolOptions {
    min: number;
    max: number;
}
export declare class Pool {
    private __connectionParams;
    private __poolOptions;
    private __clientOptions;
    private __mutex;
    private __clients;
    constructor(connectionParams: RfcConnectionParameters, poolOptions?: RfcPoolOptions, clientOptions?: RfcClientOptions);
    acquire(): Promise<Client | TypeError | void>;
    release(client: Client): Promise<void>;
    releaseAll(): Promise<void>;
    get status(): object;
}

declare var Promise: any;
import { Client, RfcConnectionParameters, RfcClientOptions } from "./sapnwrfc-client";
export { Promise };
export interface RfcPoolOptions {
    min: number;
}
export declare class Pool {
    private __connectionParams;
    private __poolOptions;
    private __clientOptions;
    private __fillRequests;
    private static Ready;
    private static Active;
    constructor(connectionParams: RfcConnectionParameters, poolOptions?: RfcPoolOptions, clientOptions?: RfcClientOptions);
    newClient(): Client;
    refill(): void;
    acquire(reqId?: Number): Promise<Client>;
    release(client: Client, reqId?: Number): Promise<void>;
    releaseAll(): Promise<number>;
    get status(): object;
    get READY(): Array<Number>;
}

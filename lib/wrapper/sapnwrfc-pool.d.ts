import * as Promise from "bluebird";
import { Client, RfcConnectionParameters, RfcClientOptions } from "./sapnwrfc-client";
export interface RfcPoolOptions {
    min: number;
    max: number;
}
export declare class Pool {
    private __connectionParams;
    private __poolOptions;
    private __clientOptions;
    private __clients;
    constructor(connectionParams: RfcConnectionParameters, poolOptions?: RfcPoolOptions, clientOptions?: RfcClientOptions);
    acquire(): Promise<Client | Error> | undefined;
    release(client: Client): void;
    releaseAll(): Promise<void>;
    get status(): object;
}

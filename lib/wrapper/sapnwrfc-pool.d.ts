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
    private __clients;
    constructor(connectionParams: RfcConnectionParameters, poolOptions?: RfcPoolOptions, clientOptions?: RfcClientOptions);
    newClient(): Client;
    fill(): void;
    acquire(): Promise<Client>;
    release(client: Client): Promise<void>;
    releaseAll(): Promise<number>;
    get status(): object;
}

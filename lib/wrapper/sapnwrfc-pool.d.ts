import * as Promise from 'bluebird';
import { Client, RfcConnectionParameters } from './sapnwrfc-client';
export interface RfcPoolOptions {
    min: number;
    max: number;
}
export declare class Pool {
    private __connectionParams;
    private __poolOptions;
    private __clients;
    constructor(connectionParams: RfcConnectionParameters, poolOptions?: RfcPoolOptions);
    acquire(): Promise<Client | Error> | undefined;
    release(client: Client): void;
    releaseAll(): void;
    readonly status: object;
}

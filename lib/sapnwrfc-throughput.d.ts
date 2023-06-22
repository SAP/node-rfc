import { NodeRfcEnvironment } from "./noderfc-bindings";
import { Client } from "./sapnwrfc-client";
export interface RfcThroughputBinding {
    new (): RfcThroughputBinding;
    (): RfcThroughputBinding;
    clients: Set<Client>;
    status: RfcThroughputStatus;
    _handle: number;
    setOnConnection(_connectionHandle: number): unknown;
    removeFromConnection(_connectionHandle: number): unknown;
    getFromConnection(_connectionHandle: number): unknown;
    reset(): void;
    destroy(): void;
}
export interface RfcThroughputStatus {
    numberOfCalls: number;
    sentBytes: number;
    receivedBytes: number;
    applicationTime: number;
    totalTime: number;
    serializationTime: number;
    deserializationTime: number;
}
export declare class Throughput {
    private __throughput;
    private __clients;
    private static _handles;
    constructor(client?: Client | Array<Client>);
    setOnConnection(client: Client | Array<Client>): void;
    removeFromConnection(client: Client | Array<Client>): void;
    static getFromConnection(client: Client): Throughput | void;
    reset(): void;
    destroy(): void;
    get status(): RfcThroughputStatus;
    get clients(): Set<Client>;
    get handle(): number;
    static get environment(): NodeRfcEnvironment;
    get environment(): NodeRfcEnvironment;
}

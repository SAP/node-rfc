import { binding, Client } from "./sapnwrfc-client";
import { isUndefined, isNumber } from "util";
export interface RfcThroughputBinding {
    new (): RfcThroughputBinding;
    (): RfcThroughputBinding;
    clients: Set<Client>;
    status: RfcThroughputStatus;
    _handle: number;
    setOnConnection(_connectionHandle: number): any;
    removeFromConnection(_connectionHandle: number): any;
    getFromConnection(_connectionHandle: number): any;
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

export class Throughput {
    private __throughput: RfcThroughputBinding;
    private __clients: Set<Client> = new Set();

    private static __Handles: Map<number, Throughput> = new Map();

    constructor(client?: Client | Array<Client>) {
        this.__throughput = new binding.Throughput();
        Throughput.__Handles.set(this.__throughput._handle, this);
        if (client) this.setOnConnection(client);
    }

    setOnConnection(client: Client | Array<Client>) {
        let connections: Array<Client> = new Array();
        if (client instanceof Client) {
            connections.push(client);
        } else if (client instanceof Array) {
            connections = client;
        } else
            throw new Error(
                "Client instance or array of Client instances required as argument"
            );
        connections.forEach((c) => {
            if (!isNumber(c._connectionHandle))
                throw new Error(
                    "Throughput can't be set on closed client: " + c.id
                );
            const e = this.__throughput.setOnConnection(c._connectionHandle);
            if (isUndefined(e)) {
                this.__clients.add(c);
            } else throw new Error(JSON.stringify(e));
        });
    }

    removeFromConnection(client: Client | Array<Client>) {
        let connections: Array<Client> = new Array();
        if (client instanceof Client) {
            connections.push(client);
        } else if (client instanceof Array) {
            connections = client;
        }
        connections.forEach((c) => {
            this.__clients.delete(c);
            if (isNumber(c._connectionHandle)) {
                const e = this.__throughput.removeFromConnection(
                    c._connectionHandle
                );
                if (!isUndefined(e)) throw new Error(JSON.stringify(e));
            }
        });
    }

    static getFromConnection(client: Client): Throughput | void {
        if (!isNumber(client._connectionHandle)) return;
        const e = binding.Throughput.getFromConnection(
            client._connectionHandle
        );
        if (isNumber(e)) {
            return Throughput.__Handles.get(e);
        } else throw new Error(JSON.stringify(e));
    }

    reset() {
        this.__throughput.reset();
    }

    destroy() {
        Throughput.__Handles.delete(this.__throughput._handle);
        this.__throughput.destroy();
    }

    get status(): RfcThroughputStatus {
        return this.__throughput.status;
    }

    get clients(): Set<Client> {
        return this.__clients;
    }

    get _handle(): number {
        return this.__throughput._handle;
    }
}

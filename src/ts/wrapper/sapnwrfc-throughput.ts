// Copyright 2014 SAP AG.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http: //www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific
// language governing permissions and limitations under the License.

import {
    noderfc_binding,
    environment,
    NodeRfcEnvironment,
} from "./noderfc-bindings";
import { Client } from "./sapnwrfc-client";
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

    private static _handles: Map<number, Throughput> = new Map();

    constructor(client?: Client | Array<Client>) {
        this.__throughput = new noderfc_binding.Throughput();
        Throughput._handles.set(this.__throughput._handle, this);
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
            if (c.connectionHandle === 0)
                throw new Error(
                    "Throughput can't be set on closed client: " + c.id
                );
            const e = this.__throughput.setOnConnection(c.connectionHandle);
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
            if (c.connectionHandle > 0) {
                const e = this.__throughput.removeFromConnection(
                    c.connectionHandle
                );
                if (!isUndefined(e)) throw new Error(JSON.stringify(e));
            }
        });
    }

    static getFromConnection(client: Client): Throughput | void {
        if (client.connectionHandle === 0) return;
        const e = noderfc_binding.Throughput.getFromConnection(
            client.connectionHandle
        );
        if (isNumber(e)) {
            return Throughput._handles.get(e);
        } else throw new Error(JSON.stringify(e));
    }

    reset() {
        this.__throughput.reset();
    }

    destroy() {
        Throughput._handles.delete(this.__throughput._handle);
        this.__throughput.destroy();
    }

    get status(): RfcThroughputStatus {
        return this.__throughput.status;
    }

    get clients(): Set<Client> {
        return this.__clients;
    }

    get handle(): number {
        return this.__throughput._handle;
    }

    static get environment(): NodeRfcEnvironment {
        return environment;
    }

    get environment(): NodeRfcEnvironment {
        return environment;
    }
}

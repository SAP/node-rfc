"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.Throughput = void 0;
const noderfc_bindings_1 = require("./noderfc-bindings");
const sapnwrfc_client_1 = require("./sapnwrfc-client");
class Throughput {
    __throughput;
    __clients = new Set();
    static _handles = new Map();
    constructor(client) {
        this.__throughput = new noderfc_bindings_1.noderfc_binding.Throughput();
        Throughput._handles.set(this.__throughput._handle, this);
        if (client)
            this.setOnConnection(client);
    }
    setOnConnection(client) {
        let connections = [];
        if (client instanceof sapnwrfc_client_1.Client) {
            connections.push(client);
        }
        else if (client instanceof Array) {
            connections = client;
        }
        else
            throw new Error("Client instance or array of Client instances required as argument");
        connections.forEach((c) => {
            if (c.connectionHandle === 0)
                throw new Error(`Throughput can't be set on closed client: ${c.id}`);
            const e = this.__throughput.setOnConnection(c.connectionHandle);
            if (e === undefined) {
                this.__clients.add(c);
            }
            else
                throw new Error(JSON.stringify(e));
        });
    }
    removeFromConnection(client) {
        let connections = [];
        if (client instanceof sapnwrfc_client_1.Client) {
            connections.push(client);
        }
        else if (client instanceof Array) {
            connections = client;
        }
        connections.forEach((c) => {
            this.__clients.delete(c);
            if (c.connectionHandle > 0) {
                const e = this.__throughput.removeFromConnection(c.connectionHandle);
                if (e !== undefined)
                    throw new Error(JSON.stringify(e));
            }
        });
    }
    static getFromConnection(client) {
        if (client.connectionHandle === 0)
            return;
        const e = noderfc_bindings_1.noderfc_binding.Throughput.getFromConnection(client.connectionHandle);
        if (typeof e === "number") {
            return Throughput._handles.get(e);
        }
        else
            throw new Error(JSON.stringify(e));
    }
    reset() {
        this.__throughput.reset();
    }
    destroy() {
        Throughput._handles.delete(this.__throughput._handle);
        this.__throughput.destroy();
    }
    get status() {
        return this.__throughput.status;
    }
    get clients() {
        return this.__clients;
    }
    get handle() {
        return this.__throughput._handle;
    }
    static get environment() {
        return noderfc_bindings_1.environment;
    }
    get environment() {
        return noderfc_bindings_1.environment;
    }
}
exports.Throughput = Throughput;

"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.Throughput = void 0;
const sapnwrfc_client_1 = require("./sapnwrfc-client");
const util_1 = require("util");
let Throughput = (() => {
    class Throughput {
        constructor(client) {
            this.__clients = new Set();
            this.__throughput = new sapnwrfc_client_1.binding.Throughput();
            Throughput.__Handles.set(this.__throughput._handle, this);
            if (client)
                this.setOnConnection(client);
        }
        setOnConnection(client) {
            let connections = new Array();
            if (client instanceof sapnwrfc_client_1.Client) {
                connections.push(client);
            }
            else if (client instanceof Array) {
                connections = client;
            }
            else
                throw new Error("Client instance or array of Client instances required as argument");
            connections.forEach((c) => {
                if (!util_1.isNumber(c._connectionHandle))
                    throw new Error("Throughput can't be set on closed client: " + c.id);
                const e = this.__throughput.setOnConnection(c._connectionHandle);
                if (util_1.isUndefined(e)) {
                    this.__clients.add(c);
                }
                else
                    throw new Error(JSON.stringify(e));
            });
        }
        removeFromConnection(client) {
            let connections = new Array();
            if (client instanceof sapnwrfc_client_1.Client) {
                connections.push(client);
            }
            else if (client instanceof Array) {
                connections = client;
            }
            connections.forEach((c) => {
                this.__clients.delete(c);
                if (util_1.isNumber(c._connectionHandle)) {
                    const e = this.__throughput.removeFromConnection(c._connectionHandle);
                    if (!util_1.isUndefined(e))
                        throw new Error(JSON.stringify(e));
                }
            });
        }
        static getFromConnection(client) {
            if (!util_1.isNumber(client._connectionHandle))
                return;
            const e = sapnwrfc_client_1.binding.Throughput.getFromConnection(client._connectionHandle);
            if (util_1.isNumber(e)) {
                return Throughput.__Handles.get(e);
            }
            else
                throw new Error(JSON.stringify(e));
        }
        reset() {
            this.__throughput.reset();
        }
        destroy() {
            Throughput.__Handles.delete(this.__throughput._handle);
            this.__throughput.destroy();
        }
        get status() {
            return this.__throughput.status;
        }
        get clients() {
            return this.__clients;
        }
        get _handle() {
            return this.__throughput._handle;
        }
    }
    Throughput.__Handles = new Map();
    return Throughput;
})();
exports.Throughput = Throughput;
//# sourceMappingURL=sapnwrfc-throughput.js.map
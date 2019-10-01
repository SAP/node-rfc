"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
const Promise = require("bluebird");
const sapnwrfc_client_1 = require("./sapnwrfc-client");
class Pool {
    constructor(connectionParams, poolOptions = {
        min: 2,
        max: 50,
    }) {
        this.__connectionParams = connectionParams;
        this.__poolOptions = poolOptions;
        this.__clients = {
            ready: new Map(),
            active: new Map(),
        };
    }
    acquire() {
        for (let i = this.__clients.ready.size; i < this.__poolOptions.min; i++) {
            let client = new sapnwrfc_client_1.Client(this.__connectionParams);
            this.__clients.ready.set(client.id, client.open());
        }
        if (this.__clients.ready.size === 0) {
            return new Promise((resolve, reject) => reject(TypeError('Internal pool error, size = 0')));
        }
        let id = this.__clients.ready.keys().next().value;
        let client = this.__clients.ready.get(id);
        this.__clients.ready.delete(id);
        return client;
    }
    release(client) {
        client.close(() => {
            this.__clients.ready.set(client.id, client.open());
        });
    }
    releaseAll() {
        this.__clients.ready.forEach(cp => {
            cp.then(client => {
                client.close(() => { });
            });
        });
        this.__clients.ready = new Map();
    }
    get status() {
        return {
            ready: this.__clients.ready.size,
            options: this.__poolOptions,
        };
    }
}
exports.Pool = Pool;
//# sourceMappingURL=sapnwrfc-pool.js.map
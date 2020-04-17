"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
var Promise = require("bluebird");
exports.Promise = Promise;
const sapnwrfc_client_1 = require("./sapnwrfc-client");
class Pool {
    constructor(connectionParams, poolOptions = {
        min: 2,
    }, clientOptions) {
        this.__connectionParams = connectionParams;
        this.__poolOptions = poolOptions;
        this.__clientOptions = clientOptions;
        this.__clients = {
            ready: new Map(),
            active: new Set(),
        };
        for (let i = 0; i < this.__poolOptions.min; i++) {
            let client = this.__clientOptions
                ? new sapnwrfc_client_1.Client(this.__connectionParams, this.__clientOptions)
                : new sapnwrfc_client_1.Client(this.__connectionParams);
            this.__clients.ready.set(client.id, client.open());
        }
    }
    acquire() {
        for (let i = this.__clients.ready.size; i < this.__poolOptions.min; i++) {
            let client = this.__clientOptions
                ? new sapnwrfc_client_1.Client(this.__connectionParams, this.__clientOptions)
                : new sapnwrfc_client_1.Client(this.__connectionParams);
            this.__clients.ready.set(client.id, client.open());
        }
        let id = this.__clients.ready.keys().next().value;
        let client = this.__clients.ready.get(id);
        this.__clients.ready.delete(id);
        this.__clients.active.add(id);
        return client;
    }
    release(client) {
        return new Promise((resolve, reject) => {
            if (!(client instanceof sapnwrfc_client_1.Client))
                reject(new TypeError("Pool release() method requires a client instance as argument"));
            const id = client.id;
            client.close(() => {
                resolve(id);
            });
        });
    }
    releaseAll() {
        const result = [Promise];
        this.__clients.ready.forEach((c, id) => {
            result.push(c.then((client) => client.close(() => { })));
        });
        this.__clients.ready = new Map();
        return Promise.all(result);
    }
    get status() {
        return {
            active: this.__clients.active.size,
            ready: this.__clients.ready.size,
            options: this.__poolOptions,
        };
    }
}
exports.Pool = Pool;
//# sourceMappingURL=sapnwrfc-pool.js.map
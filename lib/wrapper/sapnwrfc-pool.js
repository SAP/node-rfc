"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
const Promise = require("bluebird");
const sapnwrfc_client_1 = require("./sapnwrfc-client");
class Pool {
    constructor(connectionParams, poolOptions = {
        min: 2,
        max: 50
    }, clientOptions) {
        this.__connectionParams = connectionParams;
        this.__poolOptions = poolOptions;
        this.__clientOptions = clientOptions;
        this.__clients = {
            ready: new Map(),
            active: new Map()
        };
    }
    acquire() {
        for (let i = this.__clients.ready.size; i < this.__poolOptions.min; i++) {
            let client = this.__clientOptions
                ? new sapnwrfc_client_1.Client(this.__connectionParams, this.__clientOptions)
                : new sapnwrfc_client_1.Client(this.__connectionParams);
            this.__clients.ready.set(client.id, client.open());
        }
        if (this.__clients.ready.size === 0) {
            return new Promise((resolve, reject) => reject(TypeError("Internal pool error, size = 0")));
        }
        let id = this.__clients.ready.keys().next().value;
        let client = this.__clients.ready.get(id);
        this.__clients.ready.delete(id);
        return client;
    }
    release(client) {
        client.close().then(() => { });
    }
    releaseAll() {
        let promises = [];
        this.__clients.ready.forEach(cp => {
            cp.then(client => {
                promises.push(client.close().then(() => {
                }));
            });
        });
        return Promise.all(promises).then(() => {
            this.__clients.ready = new Map();
        });
    }
    get status() {
        return {
            ready: this.__clients.ready.size,
            options: this.__poolOptions
        };
    }
}
exports.Pool = Pool;
//# sourceMappingURL=sapnwrfc-pool.js.map
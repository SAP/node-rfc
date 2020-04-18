"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
var Promise = require("bluebird");
exports.Promise = Promise;
const sapnwrfc_client_1 = require("./sapnwrfc-client");
const util_1 = require("util");
class Pool {
    constructor(connectionParams, poolOptions = {
        min: 2,
    }, clientOptions) {
        this.__connectionParams = connectionParams;
        this.__poolOptions = poolOptions;
        this.__clientOptions = clientOptions;
        this.__fillRequests = 0;
        this.__clients = {
            ready: [],
            active: new Map(),
        };
    }
    newClient() {
        return this.__clientOptions
            ? new sapnwrfc_client_1.Client(this.__connectionParams, this.__clientOptions)
            : new sapnwrfc_client_1.Client(this.__connectionParams);
    }
    refill() {
        if (this.__clients.ready.length + this.__fillRequests >=
            this.__poolOptions.min) {
            return;
        }
        this.__fillRequests++;
        const client = this.newClient();
        client.connect((err) => {
            this.__fillRequests--;
            if (util_1.isUndefined(err)) {
                if (this.__clients.ready.length < this.__poolOptions.min) {
                    this.__clients.ready.unshift(client);
                    if (this.__clients.ready.length < this.__poolOptions.min)
                        this.refill();
                }
                else {
                    client.close(() => { });
                }
            }
            else {
                throw new Error(err);
            }
        });
    }
    acquire() {
        return new Promise((resolve, reject) => {
            const client = this.__clients.ready.pop();
            if (this.__clients.ready.length < this.__poolOptions.min)
                this.refill();
            if (client instanceof sapnwrfc_client_1.Client) {
                this.__clients.active.set(client.id, client);
                resolve(client);
            }
            else {
                const newClient = this.newClient();
                newClient.connect((err) => {
                    if (!util_1.isUndefined(err)) {
                        reject(err);
                    }
                    else {
                        this.__clients.active.set(newClient.id, newClient);
                        resolve(newClient);
                    }
                });
            }
        });
    }
    release(client) {
        return new Promise((resolve, reject) => {
            if (!(client instanceof sapnwrfc_client_1.Client))
                reject(new TypeError("Pool release() method requires a client instance as argument"));
            const id = client.id;
            client.close(() => {
                this.__clients.active.delete(id);
                if (this.__clients.ready.length < this.__poolOptions.min) {
                    this.refill();
                }
                resolve(id);
            });
        });
    }
    releaseAll() {
        return new Promise((resolve) => {
            const toBeClosed = this.__clients.ready.length + this.__clients.active.size;
            let closed = 0;
            for (let [id, client] of this.__clients.active.entries()) {
                client.close(() => {
                    closed++;
                    if (closed === toBeClosed) {
                        this.__clients.ready = [];
                        this.__clients.active = new Map();
                        resolve(closed);
                    }
                });
            }
            this.__clients.ready.forEach((client) => client.close(() => {
                closed++;
                if (closed === toBeClosed) {
                    this.__clients.ready = [];
                    this.__clients.active = new Map();
                    resolve(closed);
                }
            }));
        });
    }
    get status() {
        return {
            active: this.__clients.active.size,
            ready: this.__clients.ready.length,
            options: this.__poolOptions,
        };
    }
}
exports.Pool = Pool;
//# sourceMappingURL=sapnwrfc-pool.js.map
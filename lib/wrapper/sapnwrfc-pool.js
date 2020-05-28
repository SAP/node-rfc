"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.Pool = exports.Promise = void 0;
var Promise = require("bluebird");
exports.Promise = Promise;
const sapnwrfc_client_1 = require("./sapnwrfc-client");
const util_1 = require("util");
let Pool = (() => {
    class Pool {
        constructor(connectionParams, poolOptions = {
            min: 2,
        }, clientOptions) {
            this.__connectionParams = connectionParams;
            this.__poolOptions = poolOptions;
            this.__clientOptions = clientOptions;
            this.__fillRequests = 0;
        }
        newClient() {
            return this.__clientOptions
                ? new sapnwrfc_client_1.Client(this.__connectionParams, this.__clientOptions)
                : new sapnwrfc_client_1.Client(this.__connectionParams);
        }
        refill() {
            if (Pool.Ready.length + this.__fillRequests >= this.__poolOptions.min) {
                return;
            }
            this.__fillRequests++;
            const client = this.newClient();
            client.connect((err) => {
                this.__fillRequests--;
                if (util_1.isUndefined(err)) {
                    if (Pool.Ready.length < this.__poolOptions.min) {
                        Pool.Ready.unshift(client);
                        if (Pool.Ready.length < this.__poolOptions.min)
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
        acquire(reqId) {
            console.log(`    pool acquire req: ${reqId} ready: ${Pool.Ready.length} ${this.READY}`);
            return new Promise((resolve, reject) => {
                const client = Pool.Ready.pop();
                if (Pool.Ready.length < this.__poolOptions.min)
                    this.refill();
                if (client instanceof sapnwrfc_client_1.Client) {
                    Pool.Active.set(client.id, client);
                    console.log(`    pool acquire req: ${reqId} client: ${client.id}:${client._connectionHandle} ready: ${Pool.Ready.length} ${this.READY}`);
                    resolve(client);
                }
                else {
                    const newClient = this.newClient();
                    newClient.connect((err) => {
                        if (!util_1.isUndefined(err)) {
                            reject(err);
                        }
                        else {
                            Pool.Active.set(newClient.id, newClient);
                            console.log(`    pool acquire req: ${reqId} client: ${newClient.id}:${newClient._connectionHandle} ready: ${Pool.Ready.length} ${this.READY}`);
                            console.log(this.READY);
                            resolve(newClient);
                        }
                    });
                }
            });
        }
        release(client, reqId) {
            return new Promise((resolve, reject) => {
                if (!(client instanceof sapnwrfc_client_1.Client))
                    reject(new TypeError("Pool release() method requires a client instance as argument"));
                const id = client.id;
                Pool.Active.delete(id);
                if (Pool.Ready.length < this.__poolOptions.min) {
                    Pool.Ready.push(client);
                }
                else {
                }
                console.log(`    pool release req: ${reqId} client: ${client.id}:${client._connectionHandle} ready: ${Pool.Ready.length} ${this.READY}`);
                resolve(id);
            });
        }
        releaseAll() {
            return new Promise((resolve) => {
                const toBeClosed = Pool.Ready.length + Pool.Active.size;
                let closed = 0;
                for (let [id, client] of Pool.Active.entries()) {
                    client.close(() => {
                        closed++;
                        if (closed === toBeClosed) {
                            Pool.Ready = [];
                            Pool.Active = new Map();
                            resolve(closed);
                        }
                    });
                }
                Pool.Ready.forEach((client) => client.close(() => {
                    closed++;
                    if (closed === toBeClosed) {
                        Pool.Ready = [];
                        Pool.Active = new Map();
                        resolve(closed);
                    }
                }));
            });
        }
        get status() {
            return {
                active: Pool.Active.size,
                ready: Pool.Ready.length,
                options: this.__poolOptions,
            };
        }
        get READY() {
            const ready = new Array();
            for (let c of Pool.Ready)
                ready.push(c._connectionHandle);
            return ready;
        }
    }
    Pool.Ready = [];
    Pool.Active = new Map();
    return Pool;
})();
exports.Pool = Pool;
//# sourceMappingURL=sapnwrfc-pool.js.map
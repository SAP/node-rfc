"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.Pool = void 0;
const util_1 = require("util");
const noderfc_bindings_1 = require("./noderfc-bindings");
const sapnwrfc_client_1 = require("./sapnwrfc-client");
class Pool {
    constructor(poolConfiguration) {
        this.__connectionParams = poolConfiguration.connectionParameters;
        this.__clientOptions = poolConfiguration.clientOptions;
        this.__poolOptions = poolConfiguration.poolOptions || {
            low: 2,
            high: 4,
        };
        this.__pool = new noderfc_bindings_1.noderfc_binding.Pool(poolConfiguration);
    }
    acquire(arg1, arg2) {
        let clients_requested = 1;
        let callback;
        if (typeof arg1 === "number") {
            clients_requested = arg1;
            if (!util_1.isUndefined(arg2)) {
                if (typeof arg2 !== "function") {
                    throw new TypeError(`Pool acquire() argument must be a function, received: ${typeof arg2}`);
                }
                else {
                    callback = arg2;
                }
            }
        }
        else if (typeof arg1 === "function") {
            callback = arg1;
            if (!util_1.isUndefined(arg2)) {
                if (typeof arg2 !== "number") {
                    throw new TypeError(`Pool acquire() argument must be a number, received: ${typeof arg2}`);
                }
                else {
                    clients_requested = arg2;
                }
            }
        }
        else if (!util_1.isUndefined(arg1)) {
            throw new TypeError(`Pool acquire() argument must ne a number or function, received: ${typeof arg1}`);
        }
        if (util_1.isUndefined(callback)) {
            return new noderfc_bindings_1.Promise((resolve, reject) => {
                try {
                    this.__pool.acquire(clients_requested, (err, clientBindings) => {
                        this.__pool.ready(this.__poolOptions.low, () => { });
                        if (!util_1.isUndefined(err)) {
                            reject(err);
                        }
                        if (Array.isArray(clientBindings)) {
                            let clients = [];
                            clientBindings.forEach((cb) => {
                                clients.push(new sapnwrfc_client_1.Client(cb));
                            });
                            resolve(clients);
                        }
                        else {
                            let c = new sapnwrfc_client_1.Client(clientBindings);
                            resolve(c);
                        }
                    });
                }
                catch (ex) {
                    reject(ex);
                }
            });
        }
        try {
            this.__pool.acquire(clients_requested, (err, clientBindings) => {
                this.__pool.ready(this.__poolOptions.low, () => { });
                if (!util_1.isUndefined(err)) {
                    callback(err);
                }
                if (Array.isArray(clientBindings)) {
                    let clients = [];
                    clientBindings.forEach((cb) => {
                        clients.push(new sapnwrfc_client_1.Client(cb));
                    });
                    callback(err, clients);
                }
                else {
                    let c = new sapnwrfc_client_1.Client(clientBindings);
                    callback(err, c);
                }
            });
        }
        catch (ex) {
            callback(ex);
        }
    }
    release(tsClient, callback) {
        if (!util_1.isUndefined(callback) && typeof callback !== "function") {
            throw new TypeError(`Pool release() 2nd argument, if provided, must be a function, received: ${typeof callback}`);
        }
        let client_bindings = [];
        if (Array.isArray(tsClient)) {
            tsClient.forEach((tsc) => {
                client_bindings.push(tsc.binding);
            });
        }
        else {
            client_bindings.push(tsClient.binding);
        }
        if (util_1.isUndefined(callback)) {
            return new noderfc_bindings_1.Promise((resolve, reject) => {
                try {
                    this.__pool.release(client_bindings, (err) => {
                        if (util_1.isUndefined(err)) {
                            resolve();
                        }
                        else {
                            reject(err);
                        }
                    });
                }
                catch (ex) {
                    reject(ex);
                }
            });
        }
        try {
            this.__pool.release(client_bindings, callback);
        }
        catch (ex) {
            callback(ex);
        }
    }
    closeAll(callback) {
        if (util_1.isUndefined(callback)) {
            return new noderfc_bindings_1.Promise((resolve) => {
                this.__pool.closeAll(() => {
                    resolve();
                });
            });
        }
        this.__pool.closeAll(callback);
    }
    ready(arg1, arg2) {
        let new_ready = this.__poolOptions.low;
        let callback;
        if (typeof arg1 === "number") {
            new_ready = arg1;
            if (!util_1.isUndefined(arg2)) {
                if (typeof arg2 !== "function") {
                    throw new TypeError(`Pool ready() argument must be a function, received: ${typeof arg2}`);
                }
                else {
                    callback = arg2;
                }
            }
        }
        else if (typeof arg1 === "function") {
            callback = arg1;
            if (!util_1.isUndefined(arg2)) {
                if (typeof arg2 !== "number") {
                    throw new TypeError(`Pool ready() argument must be a number, received: ${typeof arg2}`);
                }
                else {
                    new_ready = arg2;
                }
            }
        }
        else if (!util_1.isUndefined(arg1)) {
            throw new TypeError(`Pool ready() argument must ne a number or function, received: ${typeof arg1}`);
        }
        if (util_1.isUndefined(callback)) {
            return new noderfc_bindings_1.Promise((resolve, reject) => {
                this.__pool.ready(new_ready, (err) => {
                    if (util_1.isUndefined(err)) {
                        resolve();
                    }
                    else {
                        reject(err);
                    }
                });
            });
        }
        try {
            this.__pool.ready(new_ready, callback);
        }
        catch (ex) {
            callback(ex);
        }
    }
    get id() {
        return this.__pool._id;
    }
    get binding() {
        return this.__pool;
    }
    get config() {
        return this.__pool._config;
    }
    get status() {
        return this.__pool._status;
    }
    get connectionParameters() {
        return this.__connectionParams;
    }
    get clientOptions() {
        return this.__clientOptions;
    }
    get poolOptions() {
        return this.__poolOptions;
    }
    get poolConfiguration() {
        return {
            connectionParameters: this.__connectionParams,
            clientOptions: this.__clientOptions,
            poolOptions: this.__poolOptions,
        };
    }
    static get environment() {
        return noderfc_bindings_1.environment;
    }
    get environment() {
        return noderfc_bindings_1.environment;
    }
}
exports.Pool = Pool;
//# sourceMappingURL=sapnwrfc-pool.js.map
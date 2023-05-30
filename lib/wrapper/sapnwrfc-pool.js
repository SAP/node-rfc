"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.Pool = void 0;
const noderfc_bindings_1 = require("./noderfc-bindings");
const sapnwrfc_client_1 = require("./sapnwrfc-client");
class Pool {
    __connectionParams;
    __clientOptions;
    __poolOptions;
    __pool;
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
            if (arg2 !== undefined) {
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
            if (arg2 !== undefined) {
                if (typeof arg2 !== "number") {
                    throw new TypeError(`Pool acquire() argument must be a number, received: ${typeof arg2}`);
                }
                else {
                    clients_requested = arg2;
                }
            }
        }
        else if (arg1 !== undefined) {
            throw new TypeError(`Pool acquire() argument must ne a number or function, received: ${typeof arg1}`);
        }
        if (callback === undefined) {
            return new noderfc_bindings_1.Promise((resolve, reject) => {
                try {
                    this.__pool.acquire(clients_requested, (err, clientBindings) => {
                        if (err !== undefined) {
                            return reject(err);
                        }
                        if (Array.isArray(clientBindings)) {
                            const clients = [];
                            clientBindings.forEach((cb) => {
                                clients.push(new sapnwrfc_client_1.Client(cb));
                            });
                            resolve(clients);
                        }
                        else {
                            const c = new sapnwrfc_client_1.Client(clientBindings);
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
                if (err !== undefined) {
                    return callback(err);
                }
                if (Array.isArray(clientBindings)) {
                    const clients = [];
                    clientBindings.forEach((cb) => {
                        clients.push(new sapnwrfc_client_1.Client(cb));
                    });
                    callback(err, clients);
                }
                else {
                    const c = new sapnwrfc_client_1.Client(clientBindings);
                    callback(err, c);
                }
            });
        }
        catch (ex) {
            callback(ex);
        }
    }
    release(tsClient, callback) {
        if (callback !== undefined && typeof callback !== "function") {
            throw new TypeError(`Pool release() 2nd argument, if provided, must be a function, received: ${typeof callback}`);
        }
        const client_bindings = [];
        if (Array.isArray(tsClient)) {
            tsClient.forEach((tsc) => {
                client_bindings.push(tsc.binding);
            });
        }
        else if (tsClient instanceof sapnwrfc_client_1.Client) {
            client_bindings.push(tsClient.binding);
        }
        else {
            throw new TypeError(`Pool release() 1st argument must be a single client or array of clients, received: ${typeof tsClient}`);
        }
        if (callback === undefined) {
            return new noderfc_bindings_1.Promise((resolve, reject) => {
                try {
                    this.__pool.release(client_bindings, (err) => {
                        if (err === undefined) {
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
    cancel(client, callback) {
        if (callback !== undefined && typeof callback !== "function") {
            throw new TypeError(`Pool cancel() 2nd argument, if provided, must be a function, received: ${typeof callback}`);
        }
        if (typeof callback === "function") {
            return client.cancel(callback);
        }
        else {
            return client.cancel();
        }
    }
    closeAll(callback) {
        if (callback === undefined) {
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
            if (arg2 !== undefined) {
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
            if (arg2 !== undefined) {
                if (typeof arg2 !== "number") {
                    throw new TypeError(`Pool ready() argument must be a number, received: ${typeof arg2}`);
                }
                else {
                    new_ready = arg2;
                }
            }
        }
        else if (arg1 !== undefined) {
            throw new TypeError(`Pool ready() argument must ne a number or function, received: ${typeof arg1}`);
        }
        if (callback === undefined) {
            return new noderfc_bindings_1.Promise((resolve, reject) => {
                this.__pool.ready(new_ready, (err) => {
                    if (err == undefined) {
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

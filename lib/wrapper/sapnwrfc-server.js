"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.Server = void 0;
const noderfc_bindings_1 = require("./noderfc-bindings");
class Server {
    constructor(serverParams, clientParams, clientOptions) {
        if (clientOptions !== undefined) {
            this.__server = new noderfc_bindings_1.noderfc_binding.Server(serverParams, clientParams, clientOptions);
        }
        else {
            this.__server = new noderfc_bindings_1.noderfc_binding.Server(serverParams, clientParams);
        }
    }
    start(callback) {
        if (callback === undefined) {
            return new Promise((resolve, reject) => {
                this.__server.start((err) => {
                    if (err === undefined) {
                        resolve();
                    }
                    else {
                        reject(err);
                    }
                });
            });
        }
        this.__server.start(callback);
    }
    stop(callback) {
        if (callback === undefined) {
            return new Promise((resolve, reject) => {
                this.__server.stop((err) => {
                    if (err === undefined) {
                        resolve();
                    }
                    else {
                        reject(err);
                    }
                });
            });
        }
        this.__server.stop(callback);
    }
    addFunction(abapFunctionName, jsFunction, callback) {
        if (callback === undefined) {
            return new Promise((resolve, reject) => {
                this.__server.addFunction(abapFunctionName, jsFunction, (err) => {
                    if (err === undefined) {
                        resolve();
                    }
                    else {
                        reject(err);
                    }
                });
            });
        }
        this.__server.addFunction(abapFunctionName, jsFunction, callback);
    }
    removeFunction(abapFunctionName, callback) {
        if (callback === undefined) {
            return new Promise((resolve, reject) => {
                this.__server.removeFunction(abapFunctionName, (err) => {
                    if (err === undefined) {
                        resolve();
                    }
                    else {
                        reject(err);
                    }
                });
            });
        }
        this.__server.removeFunction(abapFunctionName, callback);
    }
    getFunctionDescription(rfmName, callback) {
        if (callback === undefined) {
            return new Promise((resolve, reject) => {
                this.__server.getFunctionDescription(rfmName, (err, rfmFunctionDescription) => {
                    if (err === undefined) {
                        resolve(rfmFunctionDescription);
                    }
                    else {
                        reject(err);
                    }
                });
            });
        }
        this.__server.getFunctionDescription(rfmName, callback);
    }
    static get environment() {
        return noderfc_bindings_1.environment;
    }
    get environment() {
        return noderfc_bindings_1.environment;
    }
    get binding() {
        return this.__server;
    }
    get id() {
        return this.__server._id;
    }
    get alive() {
        return this.__server._alive;
    }
    get server_connection() {
        return this.__server._server_conn_handle;
    }
    get client_connection() {
        return this.__server._client_conn_handle;
    }
}
exports.Server = Server;
//# sourceMappingURL=sapnwrfc-server.js.map
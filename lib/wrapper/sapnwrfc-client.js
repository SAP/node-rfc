"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.Client = exports.EnumTrace = exports.EnumSncQop = void 0;
const noderfc_bindings_1 = require("./noderfc-bindings");
var EnumSncQop;
(function (EnumSncQop) {
    EnumSncQop["DigSig"] = "1";
    EnumSncQop["DigSigEnc"] = "2";
    EnumSncQop["DigSigEncUserAuth"] = "3";
    EnumSncQop["BackendDefault"] = "8";
    EnumSncQop["Maximum"] = "9";
})(EnumSncQop || (exports.EnumSncQop = EnumSncQop = {}));
var EnumTrace;
(function (EnumTrace) {
    EnumTrace["Off"] = "0";
    EnumTrace["Brief"] = "1";
    EnumTrace["Verbose"] = "2";
    EnumTrace["Full"] = "3";
})(EnumTrace || (exports.EnumTrace = EnumTrace = {}));
var RfcParameterDirection;
(function (RfcParameterDirection) {
    RfcParameterDirection[RfcParameterDirection["RFC_IMPORT"] = 1] = "RFC_IMPORT";
    RfcParameterDirection[RfcParameterDirection["RFC_EXPORT"] = 2] = "RFC_EXPORT";
    RfcParameterDirection[RfcParameterDirection["RFC_CHANGING"] = 3] = "RFC_CHANGING";
    RfcParameterDirection[RfcParameterDirection["RFC_TABLES"] = 7] = "RFC_TABLES";
})(RfcParameterDirection || (RfcParameterDirection = {}));
class Client {
    __client;
    constructor(arg1, clientOptions) {
        if (arg1 === undefined) {
            throw new TypeError(`Client constructor requires an argument`);
        }
        if (arg1["_pool_id"] !== undefined && arg1["_pool_id"] > 0) {
            this.__client = arg1;
        }
        else {
            this.__client = clientOptions
                ? new noderfc_bindings_1.noderfc_binding.Client(arg1, clientOptions)
                : new noderfc_bindings_1.noderfc_binding.Client(arg1);
        }
    }
    static get environment() {
        return noderfc_bindings_1.environment;
    }
    get environment() {
        return noderfc_bindings_1.environment;
    }
    get binding() {
        return this.__client;
    }
    get id() {
        return this.__client._id;
    }
    get alive() {
        return this.__client._alive;
    }
    get connectionHandle() {
        return this.__client._connectionHandle;
    }
    get pool_id() {
        return this.__client._pool_id;
    }
    get config() {
        return this.__client._config;
    }
    get _id() {
        return `${this.__client._id} handle: ${this.__client._connectionHandle} ${this.__client._pool_id
            ? `[m] pool: ${this.__client._pool_id} `
            : "[d]"}`;
    }
    get connectionInfo() {
        return this.__client.connectionInfo();
    }
    static checkCallbackArg(method, callback) {
        if (callback !== undefined && typeof callback !== "function") {
            throw new TypeError(`Client ${method}() argument, if provided, must be a Function. Received: ${typeof callback}`);
        }
    }
    connect(callback) {
        Client.checkCallbackArg("connect", callback);
        return this.open(callback);
    }
    open(callback) {
        Client.checkCallbackArg("open", callback);
        if (typeof callback === "function") {
            try {
                this.__client.open(callback);
            }
            catch (ex) {
                callback(ex);
            }
        }
        else {
            return new Promise((resolve, reject) => {
                try {
                    this.__client.open((err) => {
                        if (err !== undefined) {
                            reject(err);
                        }
                        else {
                            resolve(this);
                        }
                    });
                }
                catch (ex) {
                    reject(ex);
                }
            });
        }
    }
    ping(callback) {
        Client.checkCallbackArg("ping", callback);
        if (typeof callback === "function") {
            try {
                this.__client.ping(callback);
            }
            catch (ex) {
                callback(ex);
            }
        }
        else {
            return new Promise((resolve, reject) => {
                try {
                    this.__client.ping((err, res) => {
                        if (err === undefined) {
                            resolve(res);
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
    }
    close(callback) {
        Client.checkCallbackArg("close", callback);
        if (typeof callback === "function") {
            try {
                this.__client.close(callback);
            }
            catch (ex) {
                callback(ex);
            }
        }
        else {
            return new Promise((resolve, reject) => {
                try {
                    this.__client.close((err) => {
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
    }
    cancel(callback) {
        Client.checkCallbackArg("cancel", callback);
        if (typeof callback === "function") {
            try {
                this.__client.cancel(callback);
            }
            catch (ex) {
                callback(ex);
            }
        }
        else {
            return new Promise((resolve, reject) => {
                try {
                    this.__client.cancel((err) => {
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
    }
    resetServerContext(callback) {
        Client.checkCallbackArg("resetServerContext", callback);
        if (typeof callback === "function") {
            try {
                this.__client.resetServerContext(callback);
            }
            catch (ex) {
                callback(ex);
            }
        }
        else {
            return new Promise((resolve, reject) => {
                try {
                    this.__client.resetServerContext((err) => {
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
    }
    release(callback) {
        Client.checkCallbackArg("release");
        if (typeof callback === "function") {
            try {
                this.__client.release([this.__client], callback);
            }
            catch (ex) {
                callback(ex);
            }
        }
        else {
            return new Promise((resolve, reject) => {
                try {
                    this.__client.release([this.__client], (err) => {
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
    }
    call(rfmName, rfmParams, callOptions = {}) {
        return new Promise((resolve, reject) => {
            if (arguments.length < 2) {
                reject(new TypeError("Please provide remote function module name and parameters as arguments"));
            }
            if (typeof rfmName !== "string") {
                reject(new TypeError("First argument (remote function module name) must be an string"));
            }
            if (typeof rfmParams !== "object") {
                reject(new TypeError("Second argument (remote function module parameters) must be an object"));
            }
            if (callOptions !== undefined &&
                typeof callOptions !== "object") {
                reject(new TypeError("Call options argument must be an object"));
            }
            try {
                this.invoke(rfmName, rfmParams, (err, res) => {
                    if (err !== undefined && err !== null) {
                        reject(err);
                    }
                    else {
                        resolve(res);
                    }
                }, callOptions);
            }
            catch (ex) {
                reject(ex);
            }
        });
    }
    invoke(rfmName, rfmParams, callback, callOptions) {
        try {
            if (typeof callback !== "function") {
                throw new TypeError("Callback function must be supplied");
            }
            if (arguments.length < 3) {
                throw new TypeError("Client invoke() argument missing: RFM name, parameters or callback");
            }
            if (typeof rfmName !== "string") {
                throw new TypeError("Client invoke() 1st argument (remote function module name) must be an string");
            }
            if (typeof rfmParams !== "object") {
                throw new TypeError("Client invoke() 2nd argument (remote function module parameters) must be an object");
            }
            if (arguments.length === 4 && typeof callOptions !== "object") {
                throw new TypeError("Call options argument must be an object");
            }
            const clientOptions = this.config.clientOptions;
            let timeout = 0, callbackFunction = callback;
            if (callOptions && callOptions.timeout) {
                timeout = callOptions.timeout;
            }
            if (timeout === 0 && clientOptions && clientOptions.timeout) {
                timeout = clientOptions.timeout;
            }
            if (timeout > 0) {
                const cancelTimeout = setTimeout(() => {
                    this.cancel((_err, _res) => {
                    });
                }, timeout * 1000);
                callbackFunction = (err, res) => {
                    clearTimeout(cancelTimeout);
                    callback(err, res);
                };
            }
            for (const rfmParamName of Object.keys(rfmParams)) {
                if (rfmParamName.length === 0)
                    throw new TypeError(`Empty RFM parameter name when calling "${rfmName}"`);
                if (!rfmParamName.match(/^[a-zA-Z0-9_]*$/))
                    throw new TypeError(`RFM parameter name invalid: "${rfmParamName}" when calling "${rfmName}"`);
            }
            this.__client.invoke(rfmName, rfmParams, callbackFunction, callOptions);
        }
        catch (ex) {
            if (typeof callback !== "function") {
                throw ex;
            }
            else {
                callback(ex);
            }
        }
    }
}
exports.Client = Client;

"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
const Promise = require("bluebird");
let binding;
exports.binding = binding;
try {
    exports.binding = binding = require("../binding/sapnwrfc");
}
catch (ex) {
    if (ex.message.indexOf("sapnwrfc.node") !== -1)
        ex.message +=
            ["win32", "linux", "darwin"].indexOf(process.platform) !== -1
                ? "\n\n The SAP NW RFC SDK could not be loaded, check the installation: http://sap.github.io/node-rfc/install.html#sap-nw-rfc-sdk-installation"
                : `\n\nPlatform not supported: ${process.platform}`;
    throw ex;
}
var EnumSncQop;
(function (EnumSncQop) {
    EnumSncQop["DigSig"] = "1";
    EnumSncQop["DigSigEnc"] = "2";
    EnumSncQop["DigSigEncUserAuth"] = "3";
    EnumSncQop["BackendDefault"] = "8";
    EnumSncQop["Maximum"] = "9";
})(EnumSncQop || (EnumSncQop = {}));
var EnumTrace;
(function (EnumTrace) {
    EnumTrace["Off"] = "0";
    EnumTrace["Brief"] = "1";
    EnumTrace["Verbose"] = "2";
    EnumTrace["Full"] = "3";
})(EnumTrace || (EnumTrace = {}));
class Client {
    constructor(connectionParams, options) {
        this.__client = options
            ? new binding.Client(connectionParams, options)
            : new binding.Client(connectionParams);
        this.__status = {
            created: Date.now(),
            lastopen: 0,
            lastclose: 0,
            lastcall: 0
        };
    }
    open() {
        return new Promise((resolve, reject) => {
            try {
                this.__status.lastopen = Date.now();
                this.__client.connect((err) => {
                    if (err) {
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
            if (arguments.length === 3 && typeof callOptions !== "object") {
                reject(new TypeError("Call options argument must be an object"));
            }
            this.__status.lastcall = Date.now();
            try {
                if (!this.__client.isAlive()) {
                    reject(new Error(`Client invoked RFC call with closed connection: id=${this.__client.id}`));
                }
                this.__client.invoke(rfmName, rfmParams, (err, res) => {
                    if (err) {
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
    connect(callback) {
        this.__status.lastopen = Date.now();
        this.__client.connect(callback);
    }
    invoke(rfmName, rfmParams, callback, callOptions) {
        try {
            if (!this.__client.isAlive()) {
                throw new Error(`Client invoked RFC call with closed connection: id=${this.__client.id}`);
            }
            if (typeof callback !== "function") {
                throw new TypeError("Callback function must be supplied");
            }
            if (arguments.length < 3) {
                callback(new TypeError("Please provide rfc module name, parameters and callback as arguments"));
                return;
            }
            if (typeof rfmName !== "string") {
                callback(new TypeError("First argument (remote function module name) must be an string"));
                return;
            }
            if (typeof rfmParams !== "object") {
                callback(new TypeError("Second argument (remote function module parameters) must be an object"));
                return;
            }
            if (arguments.length === 4 && typeof callOptions !== "object") {
                callback(new TypeError("Call options argument must be an object"));
                return;
            }
            this.__status.lastcall = Date.now();
            this.__client.invoke(rfmName, rfmParams, callback, callOptions);
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
    close(callback) {
        this.__status.lastclose = Date.now();
        if (typeof callback === "function") {
            return this.__client.close(callback);
        }
        else {
            return new Promise((resolve, reject) => {
                this.__client.close((err) => {
                    if (err) {
                        reject(err);
                    }
                    else {
                        resolve();
                    }
                });
            });
        }
    }
    reopen(callback) {
        this.__status.lastopen = Date.now();
        if (typeof callback === "function") {
            return this.__client.reopen(callback);
        }
        else {
            return new Promise((resolve, reject) => {
                this.__client.reopen((err) => {
                    if (err) {
                        reject(err);
                    }
                    else {
                        resolve();
                    }
                });
            });
        }
    }
    ping(callback) {
        this.__status.lastcall = Date.now();
        if (typeof callback === "function") {
            return this.__client.ping(callback);
        }
        else {
            return new Promise((resolve, reject) => {
                this.__client.ping((err, res) => {
                    if (err) {
                        reject(err);
                    }
                    else {
                        resolve(res);
                    }
                });
            });
        }
    }
    get isAlive() {
        return this.__client.isAlive();
    }
    get connectionInfo() {
        return this.__client.connectionInfo();
    }
    get id() {
        return this.__client.id;
    }
    get _connectionHandle() {
        return this.__client._connectionHandle;
    }
    get status() {
        return this.__status;
    }
    get version() {
        return this.__client.version;
    }
    get options() {
        return this.__client.options;
    }
}
exports.Client = Client;
//# sourceMappingURL=sapnwrfc-client.js.map
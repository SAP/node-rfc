"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
const Promise = require("bluebird");
const binary = require('node-pre-gyp');
const path = require('path');
const binding_path = binary.find(path.resolve(path.join(__dirname, '../../package.json')));
let binding;
try {
    binding = require(binding_path);
}
catch (ex) {
    if (ex.message.indexOf('sapnwrfc.node') == -1)
        switch (process.platform) {
            case 'win32':
                ex.message += '\n\nCheck if SAP NW RFC SDK DLL libs are on %PATH%';
                break;
            case 'linux':
                ex.message += '\n\nCheck is SAP NW RFC SDK libs loaded, run "ldconfig -p | grep sap" for example.';
                break;
            case 'darwin':
                ex.message += '\n\nCheck if SAP NW RFC SDK libs properly installed.';
                break;
            default:
                ex.message += `\n\nPlatform not supported: ${process.platform}`;
        }
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
        this.__client = options ? new binding.Client(connectionParams, options) : new binding.Client(connectionParams);
    }
    open() {
        return new Promise((resolve, reject) => {
            try {
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
                reject(new TypeError('Please provide remote function module name and parameters as arguments'));
            }
            if (typeof rfmName !== 'string') {
                reject(new TypeError('First argument (remote function module name) must be an string'));
            }
            if (typeof rfmParams !== 'object') {
                reject(new TypeError('Second argument (remote function module parameters) must be an object'));
            }
            if (arguments.length === 3 && typeof callOptions !== 'object') {
                reject(new TypeError('Call options argument must be an object'));
            }
            try {
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
        this.__client.connect(callback);
    }
    invoke(rfmName, rfmParams, callback, callOptions) {
        try {
            if (typeof callback !== 'function') {
                throw new TypeError('Callback function must be supplied');
            }
            if (arguments.length < 3) {
                callback(new TypeError('Please provide rfc module name, parameters and callback as arguments'));
                return;
            }
            if (typeof rfmName !== 'string') {
                callback(new TypeError('First argument (remote function module name) must be an string'));
                return;
            }
            if (typeof rfmParams !== 'object') {
                callback(new TypeError('Second argument (remote function module parameters) must be an object'));
                return;
            }
            if (arguments.length === 4 && typeof callOptions !== 'object') {
                callback(new TypeError('Call options argument must be an object'));
                return;
            }
            this.__client.invoke(rfmName, rfmParams, callback, callOptions);
        }
        catch (ex) {
            if (typeof callback !== 'function') {
                throw ex;
            }
            else {
                callback(ex);
            }
        }
    }
    close(callback) {
        if (typeof callback === 'function') {
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
        if (typeof callback === 'function') {
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
        if (typeof callback === 'function') {
            return this.__client.ping(callback);
        }
        else {
            return new Promise((resolve, reject) => {
                this.__client.ping((err) => {
                    if (err) {
                        reject(err);
                    }
                    else {
                        resolve(true);
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
    get version() {
        return this.__client.version;
    }
    get options() {
        return this.__client.options;
    }
}
exports.Client = Client;
//# sourceMappingURL=sapnwrfc-client.js.map
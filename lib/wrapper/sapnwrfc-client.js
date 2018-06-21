"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
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
const binary = require('node-pre-gyp');
const path = require('path');
const binding_path = binary.find(path.resolve(path.join(__dirname, '../../package.json')));
const binding = require(binding_path);
class Client {
    constructor(connectionParams) {
        this.__client = new binding.Client(connectionParams);
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
    call(rfcName, rfcParams, callOptions = {}) {
        return new Promise((resolve, reject) => {
            if (typeof callOptions === 'function') {
                reject(new TypeError('No callback argument allowed in promise based call()'));
            }
            try {
                this.__client.invoke(rfcName, rfcParams, (err, res) => {
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
    invoke(rfcName, rfcParams, callback, callOptions) {
        try {
            this.__client.invoke(rfcName, rfcParams, callback, callOptions);
        }
        catch (ex) {
            callback(ex);
        }
    }
    close() {
        return this.__client.close();
    }
    reopen(callback) {
        return this.__client.reopen(callback);
    }
    ping() {
        return this.__client.ping();
    }
    get connectionInfo() {
        return this.__client.connectionInfo();
    }
    get isAlive() {
        return this.__client.isAlive();
    }
    get id() {
        return this.__client.id;
    }
    get version() {
        return this.__client.version;
    }
}
exports.Client = Client;
//# sourceMappingURL=sapnwrfc-client.js.map
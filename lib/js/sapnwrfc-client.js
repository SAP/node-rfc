"use strict";
var binary = require('node-pre-gyp');
var path = require('path');
var binding_path = binary.find(path.resolve(path.join(__dirname, '../../package.json')));
var binding = require(binding_path);
var Client = (function () {
    function Client(connectionParams) {
        this.__connectionParams = connectionParams;
        this.__client = new binding.Client(connectionParams);
    }
    Client.prototype.open = function () {
        var _this = this;
        return new Promise(function (resolve, reject) {
            try {
                _this.__client.connect(function (err) {
                    if (err) {
                        reject(err);
                    }
                    else {
                        resolve(_this);
                    }
                });
            }
            catch (ex) {
                reject(ex);
            }
        });
    };
    Client.prototype.call = function (rfcName, rfcParams, callOptions) {
        var _this = this;
        if (callOptions === void 0) { callOptions = {}; }
        return new Promise(function (resolve, reject) {
            if (typeof callOptions === 'function') {
                reject(new TypeError('No callback argument allowed in promise based call()'));
            }
            try {
                _this.__client.invoke(rfcName, rfcParams, function (err, res) {
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
    };
    Client.prototype.connect = function (callback) {
        this.__client.connect(callback);
    };
    Client.prototype.invoke = function (rfcName, rfcParams, callback, callOptions) {
        try {
            this.__client.invoke(rfcName, rfcParams, callback, callOptions);
        }
        catch (ex) {
            callback(ex);
        }
    };
    Client.prototype.close = function () {
        return this.__client.close();
    };
    Client.prototype.reopen = function (callback) {
        return this.__client.reopen(callback);
    };
    Client.prototype.ping = function () {
        return this.__client.ping();
    };
    Client.prototype.connectionInfo = function () {
        return this.__client.connectionInfo();
    };
    Client.prototype.isAlive = function () {
        return this.__client.isAlive();
    };
    Object.defineProperty(Client.prototype, "id", {
        get: function () {
            return this.__client.id;
        },
        enumerable: true,
        configurable: true
    });
    Object.defineProperty(Client.prototype, "version", {
        get: function () {
            return this.__client.version;
        },
        enumerable: true,
        configurable: true
    });
    return Client;
}());
module.exports = Client;
//# sourceMappingURL=sapnwrfc-client.js.map
'use strict';

const binary = require('node-pre-gyp');
const path = require('path');
const binding_path = binary.find(path.resolve(path.join(__dirname, '../package.json')));
const binding = require(binding_path);

// wrap connect and invoke functions as open and call promises
binding.Client.new = function(connectionParams) {
    const client = new binding.Client(connectionParams);

    client.open = (...args) => {
        return new Promise((resolve, reject) => {
            if (args.some(p => typeof p === 'function'))
                reject(new TypeError('No callback argument allowed in promise based open()'));
            try {
                client.connect(err => {
                    if (err) {
                        reject(err);
                    } else {
                        resolve(client);
                    }
                });
            } catch (ex) {
                reject(ex);
            }
        });
    };

    client.call = (...args) => {
        return new Promise((resolve, reject) => {
            if (args.some(p => typeof p === 'function'))
                reject(new TypeError('No callback argument allowed in promise based call()'));
            try {
                client.invoke(...args, (err, res) => {
                    if (err) {
                        reject(err);
                    } else {
                        resolve(res);
                    }
                });
            } catch (ex) {
                reject(ex);
            }
        });
    };

    return client;
};

binding.Pool = class {
    constructor(connectionParams, options) {
        this.options = options
            ? options
            : {
                min: 2,
                margin: 5,
                max: 50,
			  };

        this.__connectionParams = connectionParams;
        this.__connections = {
            ready: new Map(),
            active: new Map(),
        };

        this.__idCounter = 0;
    }

    dispose() {
        this.__connections.active.forEach(client => {
            client.close();
            this.__connections.active.delete(client.__poolId);
        });
        this.__connections.ready.forEach(client => {
            client.close();
            this.__connections.ready.delete(client.__poolId);
        });
    }

    __activate(client) {
        client.__poolId = this.__idCounter++;
        this.__connections.active.set(client.__poolId, client);
        return client.__poolId;
    }

    acquire(callback) {
        if (callback && typeof callback === 'function') {
            let client = new binding.Client(this.__connectionParams);
            this.__activate(client);
            callback(client);
        } else {
            let client = binding.Client.new(this.__connectionParams);
            this.__activate(client);
            return client.open(this.__connectionParams);
        }
    }

    release(client) {
        if (this.__connections.ready.size < this.options.margin) {
            // back to ready, if needed
            this.__connections.ready.set(client.__poolId, client);
            this.__connections.active.delete(client.__poolId);
        } else {
            // else close
            this.__close(client);
        }
    }

    status() {
        return {
            active: this.__connections.active.size,
            ready: this.__connections.ready.size,
            options: this.options,
        };
    }
};

module.exports = exports = binding;

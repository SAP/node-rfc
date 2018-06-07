const binary = require('node-pre-gyp');
const path = require('path');
const binding_path = binary.find(path.resolve(path.join(__dirname, '../package.json')));
const binding = require(binding_path);

// wrap connect and invoke functions as open and call promises
binding.Client.new = connParams => {
    const client = new binding.Client(connParams);

    client.open = (...args) => {
        return new Promise((resolve, reject) => {
            if (args.some(p => typeof p === 'function'))
                reject(new TypeError('No callback argument allowed in promise based open()'));
            try {
                client.connect(err => {
                    if (err) {
                        reject(err);
                    } else {
                        resolve();
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

module.exports = exports = binding;

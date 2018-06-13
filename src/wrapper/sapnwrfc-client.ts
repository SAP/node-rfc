/// <reference types="node" />

//
// SAP NW RFC Client Binding Wrapper
//

interface RfcClient {
	new (connectionParameters: object): RfcClient;
	(connectionParameters: object): RfcClient;
	connectionInfo(): object;
	connect(callback: Function): any;
	invoke(rfcName: string, rfcParams: object, callback: Function, callOptions?: object): any;
	ping(): void;
	close(): object;
	reopen(callback: Function): void;
	isAlive(): boolean;
	id: number;
	version: object;
}

interface RfcClientBinding {
	Client: RfcClient;
	getVersion(): object;
	verbose(): this;
}

const binary = require('node-pre-gyp');
const path = require('path');
const binding_path = binary.find(path.resolve(path.join(__dirname, '../../package.json')));
const binding: RfcClientBinding = require(binding_path);

class Client {
	private __connectionParams: object;
	private __client: RfcClient;

	constructor(connectionParams: object) {
	    this.__connectionParams = connectionParams;
	    this.__client = new binding.Client(connectionParams);
	}

	open() {
	    return new Promise((resolve, reject) => {
	        try {
	            this.__client.connect((err: any) => {
	                if (err) {
	                    reject(err);
	                } else {
	                    resolve(this);
	                }
	            });
	        } catch (ex) {
	            reject(ex);
	        }
	    });
	}

	call(rfcName: string, rfcParams: object, callOptions: object = {}) {
	    return new Promise((resolve, reject) => {
	        if (typeof callOptions === 'function') {
	            reject(new TypeError('No callback argument allowed in promise based call()'));
	        }
	        try {
	            this.__client.invoke(
	                rfcName,
	                rfcParams,
	                (err: any, res: any) => {
	                    if (err) {
	                        reject(err);
	                    } else {
	                        resolve(res);
	                    }
	                },
	                callOptions
	            );
	        } catch (ex) {
	            reject(ex);
	        }
	    });
	}

	connect(callback: Function) {
	    this.__client.connect(callback);
	}

	invoke(rfcName: string, rfcParams: object, callback: Function, callOptions?: object) {
	    try {
	        this.__client.invoke(rfcName, rfcParams, callback, callOptions);
	    } catch (ex) {
	        callback(ex);
	    }
	}

	close() {
	    return this.__client.close();
	}

	reopen(callback: Function) {
	    return this.__client.reopen(callback);
	}

	ping() {
	    return this.__client.ping();
	}
	connectionInfo() {
	    return this.__client.connectionInfo();
	}

	isAlive() {
	    return this.__client.isAlive();
	}

	get id(): number {
	    return this.__client.id;
	}

	get version(): object {
	    return this.__client.version;
	}
}

export = Client;

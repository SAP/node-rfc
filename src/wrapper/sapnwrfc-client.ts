/// <reference types="node" />

//
// SAP NW RFC Client Binding Wrapper
//

const binary = require('node-pre-gyp');
const path = require('path');
const binding_path = binary.find(path.resolve(path.join(__dirname, '../../package.json')));
const binding = require(binding_path);

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

/*
//
// Client Connections Pool
//

interface RfcPoolOptions {
	low: number;
	bucket: number;
	max: number;
}

class Pool {
	private __connectionParams: object = null;
	private __clientBinding: RfcClientBinding = null;
	private __poolOptions: RfcPoolOptions = null;

	private __clients: { [key: string]: Map<number, Client> };

	constructor(
	    connectionParams: object,
	    clientBinding: RfcClientBinding,
	    poolOptions: RfcPoolOptions = {
	        low: 2,
	        bucket: 5,
	        max: 50,
	    }
	) {
	    this.__connectionParams = connectionParams;
	    this.__clientBinding = clientBinding;
	    this.__poolOptions = poolOptions;

	    this.__clients = {
	        ready: new Map(),
	        active: new Map(),
	    };
	}

	private activateClient() {
	    let client = this.__clients.ready.values().next().value;
	    this.__clients.active.set(client.id, client);
	    this.__clients.ready.delete(client.id);
	    return client;
	}

	acquire() {
	    if (this.__clients.ready.size <= this.__poolOptions.low) {
	        let client;
	        let newClients = [];

	        for (let i = this.__clients.ready.size; i < this.__poolOptions.bucket; i++) {
	            client = new Client(this.__connectionParams);
	            this.__clients.ready.set(client.id, client);
	            newClients.push(client.open());
	        }
	        return new Promise(resolve => {
	            Promise.all(newClients).then(() => {
	                return new Promise(resolve => resolve(this.activateClient()));
	            });
	        });
	    } else {
	        return new Promise(resolve => resolve(this.activateClient()));
	    }
	}

	release(client: Client) {
	    if (this.__clients.active.has(client.id)) {
	        this.__clients.ready.set(client.id, client);
	        this.__clients.active.delete(client.id);
	    } else if (this.__clients.ready.has(client.id)) {
	        // do nothing
	    } else {
	        throw new Error('node-rfc Client Pool internal error:' + JSON.stringify(this.status));
	    }
	}

	get status(): object {
	    return {
	        active: this.__clients.active.size,
	        ready: this.__clients.ready.size,
	        options: this.__poolOptions,
	    };
	}
}



*/

/// <reference types="node" />

//
// SAP NW RFC Client Binding Wrapper
//

interface RfcClientInstance {
	new (connectionParameters: object): RfcClientInstance;
	(connectionParameters: object): RfcClientInstance;
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
	Client: RfcClientInstance;
	getVersion(): object;
	verbose(): this;
}

export interface RfcCallOptions {
	notRequested?: Array<String>;
	timeout?: number;
}

enum EnumSncQop {
	DigSig = '1',
	DigSigEnc = '2',
	DigSigEncUserAuth = '3',
	BackendDefault = '8',
	Maximum = '9',
}

enum EnumTrace {
	Off = '0',
	Brief = '1',
	Verbose = '2',
	Full = '3',
}

export interface RfcConnectionParameters {
	// general
	saprouter?: string;
	snc_lib?: string;
	snc_myname?: string;
	snc_partnername?: string;
	snc_qop?: EnumSncQop;
	trace?: EnumTrace;
	// not supported
	// pcs
	// codepage
	// noCompression

	// client
	user?: string;
	passwd?: string;
	client: string;
	lang?: string;
	mysapsso2?: string;
	getsso2?: string;
	x509cert?: string;

	//
	// destination
	//

	// specific server
	dest?: string;
	ashost?: string;
	sysnr?: string;
	gwhost?: string;
	gwserv?: string;

	// load balancing
	//dest?: string,
	group?: string;
	r3name?: string;
	sysid?: string;
	mshost?: string;
	msserv?: string;

	// gateway
	//dest?: string,
	tpname?: string;
	program_id?: string;
	//gwhost?: string,
	//gwserv?: string,
}

const binary = require('node-pre-gyp');
const path = require('path');
const binding_path = binary.find(path.resolve(path.join(__dirname, '../../package.json')));
const binding: RfcClientBinding = require(binding_path);

export class RfcClient {
	private __connectionParams: RfcConnectionParameters;
	private __client: RfcClientInstance;

	constructor(connectionParams: RfcConnectionParameters) {
	    this.__connectionParams = connectionParams;
	    this.__client = new binding.Client(connectionParams);
	}

	open(): Promise<{}> {
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

	call(rfcName: string, rfcParams: object, callOptions: RfcCallOptions = {}): Promise<{}> {
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

	get connectionInfo(): object {
	    return this.__client.connectionInfo();
	}

	get isAlive(): boolean {
	    return this.__client.isAlive();
	}

	get id(): number {
	    return this.__client.id;
	}

	get version(): object {
	    return this.__client.version;
	}

	get connectionParameters(): RfcConnectionParameters {
	    return this.__connectionParams;
	}
}

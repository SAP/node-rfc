import * as Promise from 'bluebird';
import { Client, RfcConnectionParameters } from './sapnwrfc-client';

export interface RfcPoolOptions {
	min: number;
	max: number;
}

export class Pool {
	private __connectionParams: RfcConnectionParameters;
	private __poolOptions: RfcPoolOptions;

	private __clients: { [key: string]: Map<number, Promise<Client>> };

	constructor(
		connectionParams: RfcConnectionParameters,
		poolOptions: RfcPoolOptions = {
			min: 2,
			max: 50,
		}
	) {
		this.__connectionParams = connectionParams;
		this.__poolOptions = poolOptions;

		this.__clients = {
			ready: new Map(),
			active: new Map(),
		};
	}

	acquire(): Promise<Client | Error> | undefined {
		for (let i = this.__clients.ready.size; i < this.__poolOptions.min; i++) {
			let client = new Client(this.__connectionParams);
			this.__clients.ready.set(client.id, client.open());
		}
		if (this.__clients.ready.size === 0) {
			return new Promise((resolve, reject) => reject(TypeError('Internal pool error, size = 0')));
		}
		let id = this.__clients.ready.keys().next().value;
		let client = this.__clients.ready.get(id);
		this.__clients.ready.delete(id);
		return client;
	}

	release(client: Client) {
		client.close(() => {
			this.__clients.ready.set(client.id, client.open());
		});
	}

	releaseAll() {
		this.__clients.ready.forEach(cp => {
			cp.then(client => {
				client.close(() => {});
			});
		});
		this.__clients.ready = new Map();
	}

	get status(): object {
		return {
			//active: this.__clients.active.size,
			ready: this.__clients.ready.size,
			options: this.__poolOptions,
		};
	}
}

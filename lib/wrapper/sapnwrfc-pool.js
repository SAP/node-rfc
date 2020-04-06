"use strict";
var __awaiter = (this && this.__awaiter) || function (thisArg, _arguments, P, generator) {
    function adopt(value) { return value instanceof P ? value : new P(function (resolve) { resolve(value); }); }
    return new (P || (P = Promise))(function (resolve, reject) {
        function fulfilled(value) { try { step(generator.next(value)); } catch (e) { reject(e); } }
        function rejected(value) { try { step(generator["throw"](value)); } catch (e) { reject(e); } }
        function step(result) { result.done ? resolve(result.value) : adopt(result.value).then(fulfilled, rejected); }
        step((generator = generator.apply(thisArg, _arguments || [])).next());
    });
};
Object.defineProperty(exports, "__esModule", { value: true });
const sapnwrfc_client_1 = require("./sapnwrfc-client");
class Mutex {
    constructor() {
        this._lock = Promise.resolve();
    }
    _acquire() {
        let release;
        const lock = (this._lock = new Promise((resolve) => {
            release = resolve;
        }));
        return release;
    }
    acquire() {
        const q = this._lock.then(() => release);
        const release = this._acquire();
        return q;
    }
}
class Pool {
    constructor(connectionParams, poolOptions = {
        min: 2,
        max: 50,
    }, clientOptions) {
        this.__mutex = new Mutex();
        this.__connectionParams = connectionParams;
        this.__poolOptions = poolOptions;
        this.__clientOptions = clientOptions;
        this.__clients = {
            ready: new Map(),
            active: new Set(),
        };
    }
    acquire() {
        return (() => __awaiter(this, void 0, void 0, function* () {
            const unlock = yield this.__mutex.acquire();
            for (let i = this.__clients.ready.size; i < this.__poolOptions.min; i++) {
                let client = this.__clientOptions
                    ? new sapnwrfc_client_1.Client(this.__connectionParams, this.__clientOptions)
                    : new sapnwrfc_client_1.Client(this.__connectionParams);
                this.__clients.ready.set(client.id, client.open());
            }
            if (this.__clients.ready.size === 0) {
                return new Promise((resolve, reject) => reject(TypeError("Internal pool error, size = 0")));
            }
            let id = this.__clients.ready.keys().next().value;
            let client = this.__clients.ready.get(id);
            this.__clients.ready.delete(id);
            this.__clients.active.add(id);
            unlock();
            return client;
        }))();
    }
    release(client) {
        return (() => __awaiter(this, void 0, void 0, function* () {
            yield client.close();
            const unlock = yield this.__mutex.acquire();
            if (this.__clients.ready.size < this.__poolOptions.min) {
                this.__clients.ready.set(client.id, client.open());
                this.__clients.active.delete(client.id);
            }
            unlock();
        }))();
    }
    releaseAll() {
        return (() => __awaiter(this, void 0, void 0, function* () {
            const unlock = yield this.__mutex.acquire();
            for (const [id, client] of this.__clients.ready) {
                this.__clients.ready.delete(id);
                client.then((c) => c.close());
            }
            unlock();
        }))();
    }
    get status() {
        return {
            active: this.__clients.active.size,
            ready: this.__clients.ready.size,
            options: this.__poolOptions,
        };
    }
}
exports.Pool = Pool;
//# sourceMappingURL=sapnwrfc-pool.js.map
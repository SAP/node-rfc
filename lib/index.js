"use strict";
var __createBinding = (this && this.__createBinding) || (Object.create ? (function(o, m, k, k2) {
    if (k2 === undefined) k2 = k;
    var desc = Object.getOwnPropertyDescriptor(m, k);
    if (!desc || ("get" in desc ? !m.__esModule : desc.writable || desc.configurable)) {
      desc = { enumerable: true, get: function() { return m[k]; } };
    }
    Object.defineProperty(o, k2, desc);
}) : (function(o, m, k, k2) {
    if (k2 === undefined) k2 = k;
    o[k2] = m[k];
}));
var __exportStar = (this && this.__exportStar) || function(m, exports) {
    for (var p in m) if (p !== "default" && !Object.prototype.hasOwnProperty.call(exports, p)) __createBinding(exports, m, p);
};
Object.defineProperty(exports, "__esModule", { value: true });
exports.languageSapToIso = exports.languageIsoToSap = exports.cancelClient = exports.sapnwrfcEvents = exports.loadCryptoLibrary = exports.reloadIniFile = exports.setIniFileDirectory = void 0;
const events_1 = require("events");
const worker_threads_1 = require("worker_threads");
__exportStar(require("./wrapper/noderfc-bindings"), exports);
__exportStar(require("./wrapper/sapnwrfc-client"), exports);
__exportStar(require("./wrapper/sapnwrfc-pool"), exports);
__exportStar(require("./wrapper/sapnwrfc-throughput"), exports);
__exportStar(require("./wrapper/sapnwrfc-server"), exports);
const noderfc_bindings_1 = require("./wrapper/noderfc-bindings");
function setIniFileDirectory(iniFileDirectory) {
    const path = require("path");
    const fullPath = path.join(iniFileDirectory, "sapnwrfc.ini");
    if (!require("fs").existsSync(fullPath)) {
        throw new Error(`sapnwrfc.ini not found in: ${iniFileDirectory}`);
    }
    noderfc_bindings_1.noderfc_binding.setIniFileDirectory(iniFileDirectory);
}
exports.setIniFileDirectory = setIniFileDirectory;
function reloadIniFile() {
    const err = noderfc_bindings_1.noderfc_binding.reloadIniFile();
    if (err && err.message) {
        throw new Error(err.message);
    }
}
exports.reloadIniFile = reloadIniFile;
function loadCryptoLibrary(libAbsolutePath) {
    if (!require("fs").existsSync(libAbsolutePath)) {
        throw new Error(`Crypto library not found: ${libAbsolutePath}`);
    }
    noderfc_bindings_1.noderfc_binding.loadCryptoLibrary(libAbsolutePath);
}
exports.loadCryptoLibrary = loadCryptoLibrary;
exports.sapnwrfcEvents = new events_1.EventEmitter();
function terminate(workerData) {
    return new Promise((resolve, reject) => {
        const terminator = new worker_threads_1.Worker(require("path").join(__dirname, "./wrapper/cancel.js"), { workerData });
        terminator.on("message", resolve);
        terminator.on("error", reject);
        terminator.on("exit", (code) => {
            if (code !== 0)
                reject(new Error(`Terminator stopped with exit code ${code}`));
        });
    });
}
function cancelClient(client, callback) {
    if (callback !== undefined && typeof callback !== "function") {
        throw new TypeError(`cancelClient 2nd argument, if provided, must be a Function. Received: ${typeof callback}`);
    }
    const old_handle = client.connectionHandle;
    const workerData = { connectionHandle: old_handle };
    if (typeof callback === "function") {
        return terminate(workerData)
            .then((res) => {
            exports.sapnwrfcEvents.emit("sapnwrfc:clientCancel", {
                id: client.id,
                connectionHandle: old_handle,
            });
            callback(undefined, res);
        })
            .catch((err) => {
            callback(err);
        });
    }
    else {
        return terminate(workerData);
    }
}
exports.cancelClient = cancelClient;
function languageIsoToSap(langIso) {
    const langSap = noderfc_bindings_1.noderfc_binding.languageIsoToSap(langIso);
    if (typeof langSap === "string") {
        return langSap;
    }
    throw new Error(`Language ISO code not found: ${langIso}`);
}
exports.languageIsoToSap = languageIsoToSap;
function languageSapToIso(langSap) {
    const langIso = noderfc_bindings_1.noderfc_binding.languageSapToIso(langSap);
    if (typeof langIso === "string") {
        return langIso;
    }
    throw new Error(`Language SAP code not found: ${langSap}`);
}
exports.languageSapToIso = languageSapToIso;
//# sourceMappingURL=index.js.map
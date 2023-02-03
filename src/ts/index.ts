/// <reference types="node" />

// SPDX-FileCopyrightText: 2014 SAP SE Srdjan Boskovic <srdjan.boskovic@sap.com>
//
// SPDX-License-Identifier: Apache-2.0

import { EventEmitter } from "events";

import { Worker } from "worker_threads";
import { Client } from "./wrapper/sapnwrfc-client";

export * from "./wrapper/noderfc-bindings";
export * from "./wrapper/sapnwrfc-client";
export * from "./wrapper/sapnwrfc-pool";
export * from "./wrapper/sapnwrfc-throughput";
export * from "./wrapper/sapnwrfc-server";

import { noderfc_binding } from "./wrapper/noderfc-bindings";

//
// Addon functions
//

export function setIniFileDirectory(iniFileDirectory: string) {
    const path = require("path");
    const fullPath = path.join(iniFileDirectory, "sapnwrfc.ini");
    if (!require("fs").existsSync(fullPath)) {
        throw new Error(`sapnwrfc.ini not found in: ${iniFileDirectory}`);
    }
    noderfc_binding.setIniFileDirectory(iniFileDirectory);
}

export function reloadIniFile() {
    const err = noderfc_binding.reloadIniFile();
    if (err && err.message) {
        throw new Error(err.message)
    }
}

export function loadCryptoLibrary(libAbsolutePath: string) {
    if (!require("fs").existsSync(libAbsolutePath)) {
        throw new Error(`Crypto library not found: ${libAbsolutePath}`);
    }
    noderfc_binding.loadCryptoLibrary(libAbsolutePath);
}

export const sapnwrfcEvents = new EventEmitter();

function terminate(workerData) {
    return new Promise((resolve, reject) => {
        const terminator = new Worker(
            require("path").join(__dirname, "./wrapper/cancel.js"),
            { workerData }
        );
        terminator.on("message", resolve);
        terminator.on("error", reject);
        terminator.on("exit", (code) => {
            if (code !== 0)
                reject(new Error(`Terminator stopped with exit code ${code}`));
        });
    });
}

export function cancelClient(
    client: Client,
    callback?: Function
): void | Promise<any> {
    if (callback !== undefined && typeof callback !== "function") {
        throw new TypeError(
            `cancelClient 2nd argument, if provided, must be a Function. Received: ${typeof callback}`
        );
    }
    const old_handle = client.connectionHandle;
    const workerData = { connectionHandle: old_handle };
    if (typeof callback === "function") {
        return terminate(workerData)
            .then((res) => {
                sapnwrfcEvents.emit("sapnwrfc:clientCancel", {
                    id: client.id,
                    connectionHandle: old_handle,
                });
                callback(undefined, res);
            })
            .catch((err) => {
                callback(err);
            });
    } else {
        return terminate(workerData);
    }
}

export function languageIsoToSap(langIso: string): string {
    const langSap = noderfc_binding.languageIsoToSap(langIso);
    if (typeof langSap === "string") {
        return langSap
    }
    throw new Error(`Language ISO code not found: ${langIso}`)
}

export function languageSapToIso(langSap: string): string {
    const langIso = noderfc_binding.languageSapToIso(langSap);
    if (typeof langIso === "string") {
        return langIso
    }
    throw new Error(`Language SAP code not found: ${langSap}`)
}

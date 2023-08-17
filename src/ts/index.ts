/// <reference types="node" />

// SPDX-FileCopyrightText: 2014 SAP SE Srdjan Boskovic <srdjan.boskovic@sap.com>
//
// SPDX-License-Identifier: Apache-2.0

import { EventEmitter } from "events";

import { Worker } from "worker_threads";
import { Client } from "./sapnwrfc-client";

import fs from "fs";
import path from "path";

export * from "./noderfc-bindings";
export * from "./sapnwrfc-client";
export * from "./sapnwrfc-pool";
export * from "./sapnwrfc-throughput";
export * from "./sapnwrfc-server";

import { noderfc_binding } from "./noderfc-bindings";

//
// Addon functions
//

export function setIniFileDirectory(iniFileDirectory: string) {
    const fullPath = path.join(iniFileDirectory, "sapnwrfc.ini");
    if (!fs.existsSync(fullPath)) {
        throw new Error(`sapnwrfc.ini not found in: ${iniFileDirectory}`);
    }
    noderfc_binding.setIniFileDirectory(iniFileDirectory);
}

export function reloadIniFile() {
    const err = noderfc_binding.reloadIniFile();
    if (err && err.message) {
        throw new Error(err.message);
    }
}

export function loadCryptoLibrary(libAbsolutePath: string) {
    if (!fs.existsSync(libAbsolutePath)) {
        throw new Error(`Crypto library not found: ${libAbsolutePath}`);
    }
    noderfc_binding.loadCryptoLibrary(libAbsolutePath);
}

export function setLogFilePath(filePath: string) {
    noderfc_binding.setLogFilePath(filePath);
}

export const sapnwrfcEvents = new EventEmitter();

function terminate(workerData: unknown) {
    return new Promise((resolve, reject) => {
        const terminator = new Worker(path.join(__dirname, "./cancel.js"), {
            workerData,
        });
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
): void | Promise<unknown> {
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
        return langSap;
    }
    throw new Error(`Language ISO code not found: ${langIso}`);
}

export function languageSapToIso(langSap: string): string {
    const langIso = noderfc_binding.languageSapToIso(langSap);
    if (typeof langIso === "string") {
        return langIso;
    }
    throw new Error(`Language SAP code not found: ${langSap}`);
}

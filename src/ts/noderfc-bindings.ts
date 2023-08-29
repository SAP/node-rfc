/// <reference types="node" />

// SPDX-FileCopyrightText: 2014 SAP SE Srdjan Boskovic <srdjan.boskovic@sap.com>
//
// SPDX-License-Identifier: Apache-2.0

import path from "path";
import os from "os";
import { Promise } from "bluebird";
import { RfcClientBinding } from "./sapnwrfc-client";
import { RfcPoolBinding } from "./sapnwrfc-pool";
import { RfcThroughputBinding } from "./sapnwrfc-throughput";
import { RfcServerBinding } from "./sapnwrfc-server";

export interface NodeRfcBindingVersions {
    version: string;
    nwrfcsdk: { major: number; minor: number; patchLevel: number };
}
export interface NodeRfcEnvironment {
    platform: object;
    env: object;
    noderfc: NodeRfcBindingVersions;
    versions: object;
}
export interface NWRfcSdkError {
    name: string;
    group: number;
    code: number;
    codeString: string;
    key: string;
    message: string;
}

export interface NWRfcBinding {
    Client: RfcClientBinding;
    Pool: RfcPoolBinding;
    Throughput: RfcThroughputBinding;
    Server: RfcServerBinding;
    bindingVersions: NodeRfcBindingVersions;
    environment: NodeRfcEnvironment;
    setIniFileDirectory(iniFileDirectory: string): unknown | undefined;
    loadCryptoLibrary(libAbsolutePath: string): unknown | undefined;
    languageIsoToSap(langIso: string): string | NWRfcSdkError;
    languageSapToIso(langSap: string): string | NWRfcSdkError;
    reloadIniFile(): undefined | NWRfcSdkError;
    setLogFilePath(filePath: string): unknown;
    verbose(): this;
}

// Environment w/o SAP NWRFC SDK

const E = {
    platform: {
        name: os.platform(),
        arch: os.arch(),
        release: os.release(),
    },
    env: {
        SAPNWRFC_HOME: process.env.SAPNWRFC_HOME || "",
        RFC_INI: process.env.RFC_INI || "",
    },
    versions: process.versions,
};

if (E.platform.name === "win32") {
    E.env["nwrfcsdk_lib_on_path"] = false;
    if (E.env.SAPNWRFC_HOME.length > 0) {
        const P = process.env.PATH;
        if (P !== undefined) {
            E.env["nwrfcsdk_lib_on_path"] =
                P.toUpperCase().indexOf(
                    `${E.env.SAPNWRFC_HOME}\\lib;`.toUpperCase()
                ) > -1;
        }
    }
}

let noderfc_binding: NWRfcBinding;

try {
    const hasEnvValue = typeof process.env.NODE_RFC_MODULE_PATH !== "undefined";
    const modulePath = hasEnvValue
        ? process.env.NODE_RFC_MODULE_PATH
        : path.resolve(__dirname, "..");

    // eslint-disable-next-line @typescript-eslint/no-unsafe-call, @typescript-eslint/no-var-requires
    noderfc_binding = require("node-gyp-build")(modulePath) as NWRfcBinding;
} catch (ex) {
    const err = ex as Error;
    err.message += `\nenvironment: ${JSON.stringify(E, null, 2)}\n`;
    throw err;
}

// environment with SAP NWRFC SDK
const environment = Object.assign({}, E, {
    noderfc: noderfc_binding.bindingVersions,
});

export { Promise, noderfc_binding, environment };

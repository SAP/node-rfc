/// <reference types="node" />

// SPDX-FileCopyrightText: 2014 SAP SE Srdjan Boskovic <srdjan.boskovic@sap.com>
//
// SPDX-License-Identifier: Apache-2.0

import os from "os";
const Promise = require("bluebird");
import { RfcClientBinding } from "./sapnwrfc-client";
import { RfcPoolBinding } from "./sapnwrfc-pool";
import { RfcThroughputBinding } from "./sapnwrfc-throughput";
import { RfcServerBinding } from "./sapnwrfc-server";

export const USAGE_URL = "https://github.com/SAP/node-rfc#usage";
export interface NodeRfcBindingVersions {
    version: string;
    nwrfcsdk: { major: number; minor: number; patchLevel: number };
}
export interface NodeRfcEnvironment {
    platform: Object;
    env: Object;
    noderfc: NodeRfcBindingVersions;
    versions: Object;
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
    setIniFileDirectory(iniFileDirectory: string): any | undefined;
    loadCryptoLibrary(libAbsolutePath: string): any | undefined;
    languageIsoToSap(langIso: string): string | NWRfcSdkError;
    languageSapToIso(langSap: string): string | NWRfcSdkError;
    reloadIniFile(): undefined | NWRfcSdkError;
    verbose(): this;
}

// environment w/o SAP NWRFC SDK
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
    noderfc_binding = require("../binding/sapnwrfc");
} catch (ex) {
    const err = ex as Error;
    if (err.message.indexOf("sapnwrfc.node") !== -1)
        err.message +=
            ["win32", "linux", "darwin"].indexOf(process.platform) !== -1
                ? "\n\n The SAP NW RFC SDK could not be loaded, check the installation: https://github.com/SAP/node-rfc/blob/master/doc/installation.md#sap-nwrfc-sdk-installation"
                : `\n\nPlatform not supported: ${process.platform}`;
    err.message += `\nenvironment: ${JSON.stringify(E, null, 2)}\n`;
    throw err;
}

// environment with SAP NWRFC SDK
const environment = Object.assign({}, E, {
    noderfc: noderfc_binding.bindingVersions,
});

export { Promise, noderfc_binding, environment };

// SPDX-FileCopyrightText: 2014 SAP SE Srdjan Boskovic <srdjan.boskovic@sap.com>
//
// SPDX-License-Identifier: Apache-2.0

import path from "path";
import os from "os";

import { version } from "../../package.json";
export { version, dependencies } from "../../package.json";

import * as binding from "../../lib";
export * as binding from "../../lib";
export {
    Client,
    Pool,
    Throughput,
    Promise,
    setIniFileDirectory,
    loadCryptoLibrary,
    cancelClient,
    RfcVariable,
    RfcArray,
    RfcTable,
    RfcStructure,
    NWRfcSdkError,
} from "../../lib";

import { abapSystem } from "./abapSystem";
export { abapSystem } from "./abapSystem";

export const UNICODETEST = "ทดสอบสร้างลูกค้าจากภายนอกครั้งที่".repeat(7);
export const UNICODETEST2 = "Hällü ßärÖÄ อกครั้งที่".repeat(3);

export const sapnwrfcIniPath = path.join(process.cwd(), "test");
export const CryptoLibPath =
    process.platform === "darwin"
        ? "/Applications/Secure Login Client.app/Contents/MacOS/lib/libsapcrypto.dylib"
        : process.platform === "linux"
        ? "/usr/local/sap/cryptolib/libsapcrypto.so"
        : process.platform === "win32"
        ? //"C:\\Program Files\\SAP\\FrontEnd\\SecureLogin\\libsapcrypto.dll",
          "C:\\Tools\\cryptolib\\sapcrypto.dll"
        : "";

export const ClientPSEPath = {
    darwin: "/Users/d037732/dotfiles/sec/rfctest.pse",
    linux: "/home/www-admin/sec/rfctest.pse",
    win32: "C:\\Tools\\sec\\rfctest.pse",
};

export const refEnvironment = {
    platform: {
        name: os.platform(),
        arch: os.arch(),
        release: os.release(),
    },
    env: {
        SAPNWRFC_HOME: process.env.SAPNWRFC_HOME || "",
        RFC_INI: process.env.RFC_INI || "",
    },
    noderfc: {
        //version: "Deactivate logging: LOG_RFC_CLIENT",
        version: version,
        nwrfcsdk: { major: 7500, minor: 0, patchLevel: 11 },
    },
    versions: process.versions,
};

export const CONNECTIONS = 0x20;

export function direct_client(system = "MME", options = {}) {
    return new binding.Client(abapSystem(system), options);
}

export const poolConfiguration = {
    connectionParameters: abapSystem(),
};

binding.setIniFileDirectory(sapnwrfcIniPath);
// binding.loadCryptoLibrary(_CryptoLibPath[process.platform]);

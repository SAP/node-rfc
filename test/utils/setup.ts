// SPDX-FileCopyrightText: 2014 SAP SE Srdjan Boskovic <srdjan.boskovic@sap.com>
//
// SPDX-License-Identifier: Apache-2.0

import path from "path";
import os from "os";

import { version } from "../../package.json";
export { version, dependencies } from "../../package.json";
import { RfcClientOptions, RfcLoggingLevel, Client } from "../../lib";
import * as addon from "../../lib";
export * as addon from "../../lib";

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
    RfcObject,
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
        version: version,
        nwrfcsdk: { major: 7500, minor: 0, patchLevel: 12 },
    },
    versions: process.versions,
};

export const CONNECTIONS = 0x20;

export function direct_client(
    system = "MME",
    options = { logLevel: RfcLoggingLevel.none } as RfcClientOptions
): Client {
    return new Client(abapSystem(system), options);
}

export const poolConfiguration = {
    connectionParameters: abapSystem(),
};

addon.setIniFileDirectory(sapnwrfcIniPath);
// addon.loadCryptoLibrary(_CryptoLibPath[process.platform]);

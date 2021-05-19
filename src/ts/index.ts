/// <reference types="node" />

// SPDX-FileCopyrightText: 2014 SAP SE Srdjan Boskovic <srdjan.boskovic@sap.com>
//
// SPDX-License-Identifier: Apache-2.0

export * from "./wrapper/noderfc-bindings";
export * from "./wrapper/sapnwrfc-client";
export * from "./wrapper/sapnwrfc-pool";
export * from "./wrapper/sapnwrfc-throughput";
export * from "./wrapper/sapnwrfc-server";

import { noderfc_binding } from "./wrapper/noderfc-bindings";

export function setIniFileDirectory(iniFileDirectory: string) {
    const path = require("path");
    const fullPath = path.join(iniFileDirectory, "sapnwrfc.ini");
    if (!require("fs").existsSync(fullPath)) {
        throw new Error(`sapnwrfc.ini not found in: ${iniFileDirectory}`);
    }
    noderfc_binding.setIniFileDirectory(iniFileDirectory);
}

export function loadCryptoLibrary(libAbsolutePath: string) {
    if (!require("fs").existsSync(libAbsolutePath)) {
        throw new Error(`Crypto library not found: ${libAbsolutePath}`);
    }
    noderfc_binding.loadCryptoLibrary(libAbsolutePath);
}

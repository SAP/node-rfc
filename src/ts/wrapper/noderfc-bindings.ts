/// <reference types="node" />
import os from "os";
const Promise = require("bluebird");
import { RfcClientBinding } from "./sapnwrfc-client";
import { RfcPoolBinding } from "./sapnwrfc-pool";
import { RfcThroughputBinding } from "./sapnwrfc-throughput";

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
export interface NWRfcBinding {
    Client: RfcClientBinding;
    Pool: RfcPoolBinding;
    Throughput: RfcThroughputBinding;
    verbose(): this;
    bindingVersions: NodeRfcBindingVersions;
    environment: NodeRfcEnvironment;
}

let noderfc_binding: NWRfcBinding;

try {
    noderfc_binding = require("../binding/sapnwrfc");
} catch (ex) {
    if (ex.message.indexOf("sapnwrfc.node") !== -1)
        ex.message +=
            ["win32", "linux", "darwin"].indexOf(process.platform) !== -1
                ? "\n\n The SAP NW RFC SDK could not be loaded, check the installation: http://sap.github.io/node-rfc/install.html#sap-nw-rfc-sdk-installation"
                : `\n\nPlatform not supported: ${process.platform}`;
    throw ex;
}

const environment = {
    platform: {
        name: os.platform(),
        arch: os.arch(),
        release: os.release(),
    },
    env: {
        SAPNWRFC_HOME: process.env.SAPNWRFC_HOME || "",
        RFC_INI: process.env.RFC_INI || "",
    },
    noderfc: noderfc_binding.bindingVersions,
    versions: process.versions,
};

export { Promise };
export { noderfc_binding };
export { environment };

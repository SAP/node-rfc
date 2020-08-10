/// <reference types="node" />
declare const Promise: any;
import { RfcClientBinding } from "./sapnwrfc-client";
import { RfcPoolBinding } from "./sapnwrfc-pool";
import { RfcThroughputBinding } from "./sapnwrfc-throughput";
import { RfcServerBinding } from "./sapnwrfc-server";
export declare const USAGE_URL = "https://github.com/SAP/node-rfc#usage";
export interface NodeRfcBindingVersions {
    version: string;
    nwrfcsdk: {
        major: number;
        minor: number;
        patchLevel: number;
    };
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
    Server: RfcServerBinding;
    verbose(): this;
    bindingVersions: NodeRfcBindingVersions;
    environment: NodeRfcEnvironment;
}
declare let noderfc_binding: NWRfcBinding;
declare const environment: {
    platform: {
        name: NodeJS.Platform;
        arch: string;
        release: string;
    };
    env: {
        SAPNWRFC_HOME: string;
        RFC_INI: string;
    };
    versions: NodeJS.ProcessVersions;
} & {
    noderfc: NodeRfcBindingVersions;
};
export { Promise };
export { noderfc_binding };
export { environment };

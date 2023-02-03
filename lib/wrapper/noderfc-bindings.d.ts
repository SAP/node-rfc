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
export { Promise, noderfc_binding, environment };

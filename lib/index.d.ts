/// <reference types="node" />
import { EventEmitter } from "events";
import { Client } from "./wrapper/sapnwrfc-client";
export * from "./wrapper/noderfc-bindings";
export * from "./wrapper/sapnwrfc-client";
export * from "./wrapper/sapnwrfc-pool";
export * from "./wrapper/sapnwrfc-throughput";
export * from "./wrapper/sapnwrfc-server";
export declare function setIniFileDirectory(iniFileDirectory: string): void;
export declare function reloadIniFile(): void;
export declare function loadCryptoLibrary(libAbsolutePath: string): void;
export declare const sapnwrfcEvents: EventEmitter;
export declare function cancelClient(client: Client, callback?: Function): void | Promise<any>;
export declare function languageIsoToSap(langIso: string): string;
export declare function languageSapToIso(langSap: string): string;

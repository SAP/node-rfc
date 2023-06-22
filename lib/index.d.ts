/// <reference types="node" />
import { EventEmitter } from "events";
import { Client } from "./sapnwrfc-client";
export * from "./noderfc-bindings";
export * from "./sapnwrfc-client";
export * from "./sapnwrfc-pool";
export * from "./sapnwrfc-throughput";
export * from "./sapnwrfc-server";
export declare function setIniFileDirectory(iniFileDirectory: string): void;
export declare function reloadIniFile(): void;
export declare function loadCryptoLibrary(libAbsolutePath: string): void;
export declare const sapnwrfcEvents: EventEmitter;
export declare function cancelClient(client: Client, callback?: Function): void | Promise<unknown>;
export declare function languageIsoToSap(langIso: string): string;
export declare function languageSapToIso(langSap: string): string;

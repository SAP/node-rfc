"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
exports.environment = exports.noderfc_binding = exports.Promise = exports.USAGE_URL = void 0;
const os_1 = __importDefault(require("os"));
const bluebird_1 = require("bluebird");
Object.defineProperty(exports, "Promise", { enumerable: true, get: function () { return bluebird_1.Promise; } });
exports.USAGE_URL = "https://github.com/SAP/node-rfc#usage";
const E = {
    platform: {
        name: os_1.default.platform(),
        arch: os_1.default.arch(),
        release: os_1.default.release(),
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
                P.toUpperCase().indexOf(`${E.env.SAPNWRFC_HOME}\\lib;`.toUpperCase()) > -1;
        }
    }
}
let noderfc_binding;
try {
    exports.noderfc_binding = noderfc_binding = require("node-gyp-build")();
}
catch (ex) {
    const err = ex;
    err.message += `\nenvironment: ${JSON.stringify(E, null, 2)}\n`;
    throw err;
}
const environment = Object.assign({}, E, {
    noderfc: noderfc_binding.bindingVersions,
});
exports.environment = environment;

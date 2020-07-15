"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
exports.environment = exports.noderfc_binding = exports.Promise = exports.USAGE_URL = void 0;
const os_1 = __importDefault(require("os"));
const Promise = require("bluebird");
exports.Promise = Promise;
exports.USAGE_URL = "https://github.com/SAP/node-rfc#usage";
let noderfc_binding;
exports.noderfc_binding = noderfc_binding;
try {
    exports.noderfc_binding = noderfc_binding = require("../binding/sapnwrfc");
}
catch (ex) {
    if (ex.message.indexOf("sapnwrfc.node") !== -1)
        ex.message +=
            ["win32", "linux", "darwin"].indexOf(process.platform) !== -1
                ? "\n\n The SAP NW RFC SDK could not be loaded, check the installation: http://sap.github.io/node-rfc/install.html#sap-nw-rfc-sdk-installation"
                : `\n\nPlatform not supported: ${process.platform}`;
    throw ex;
}
const environment = {
    platform: {
        name: os_1.default.platform(),
        arch: os_1.default.arch(),
        release: os_1.default.release(),
    },
    env: {
        SAPNWRFC_HOME: process.env.SAPNWRFC_HOME || "",
        RFC_INI: process.env.RFC_INI || "",
    },
    noderfc: noderfc_binding.bindingVersions,
    versions: process.versions,
};
exports.environment = environment;
//# sourceMappingURL=noderfc-bindings.js.map
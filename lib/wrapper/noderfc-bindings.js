"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
exports.cancelClient = exports.environment = exports.noderfc_binding = exports.Promise = exports.USAGE_URL = void 0;
const os_1 = __importDefault(require("os"));
const Promise = require("bluebird");
exports.Promise = Promise;
const worker_threads_1 = require("worker_threads");
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
exports.noderfc_binding = noderfc_binding;
try {
    exports.noderfc_binding = noderfc_binding = require("../binding/sapnwrfc");
}
catch (ex) {
    if (ex.message.indexOf("sapnwrfc.node") !== -1)
        ex.message +=
            ["win32", "linux", "darwin"].indexOf(process.platform) !== -1
                ? "\n\n The SAP NW RFC SDK could not be loaded, check the installation: https://github.com/SAP/node-rfc/blob/master/doc/installation.md#sap-nwrfc-sdk-installation"
                : `\n\nPlatform not supported: ${process.platform}`;
    ex.message += `\nenvironment: ${JSON.stringify(E, null, 2)}\n`;
    throw ex;
}
const environment = Object.assign({}, E, {
    noderfc: noderfc_binding.bindingVersions,
});
exports.environment = environment;
function terminate(workerData) {
    return new Promise((resolve, reject) => {
        const terminator = new worker_threads_1.Worker(require("path").join(__dirname, "./noderfc-cancel.js"), { workerData });
        terminator.on("message", resolve);
        terminator.on("error", reject);
        terminator.on("exit", (code) => {
            if (code !== 0)
                reject(new Error(`Terminator stopped with exit code ${code}`));
        });
    });
}
function cancelClient(client, callback) {
    if (callback !== undefined && typeof callback !== "function") {
        throw new TypeError(`cancelClient 2nd argument, if provided, must be a Function. Received: ${typeof callback}`);
    }
    const workerData = { connectionHandle: client.connectionHandle };
    if (typeof callback === "function") {
        return terminate(workerData)
            .then((res) => {
            callback(undefined, res);
        })
            .catch((err) => {
            callback(err);
        });
    }
    else {
        return terminate(workerData);
    }
}
exports.cancelClient = cancelClient;
//# sourceMappingURL=noderfc-bindings.js.map
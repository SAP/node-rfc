"use strict";
var __createBinding = (this && this.__createBinding) || (Object.create ? (function(o, m, k, k2) {
    if (k2 === undefined) k2 = k;
    Object.defineProperty(o, k2, { enumerable: true, get: function() { return m[k]; } });
}) : (function(o, m, k, k2) {
    if (k2 === undefined) k2 = k;
    o[k2] = m[k];
}));
var __exportStar = (this && this.__exportStar) || function(m, exports) {
    for (var p in m) if (p !== "default" && !exports.hasOwnProperty(p)) __createBinding(exports, m, p);
};
Object.defineProperty(exports, "__esModule", { value: true });
exports.setIniFileDirectory = void 0;
__exportStar(require("./wrapper/noderfc-bindings"), exports);
__exportStar(require("./wrapper/sapnwrfc-client"), exports);
__exportStar(require("./wrapper/sapnwrfc-pool"), exports);
__exportStar(require("./wrapper/sapnwrfc-throughput"), exports);
__exportStar(require("./wrapper/sapnwrfc-server"), exports);
const noderfc_bindings_1 = require("./wrapper/noderfc-bindings");
function setIniFileDirectory(iniFileDirectory) {
    const path = require("path");
    const fullPath = path.join(iniFileDirectory, "sapnwrfc.ini");
    if (!require("fs").existsSync(fullPath)) {
        throw new Error(`sapnwrfc.ini not found in: ${iniFileDirectory}`);
    }
    noderfc_bindings_1.noderfc_binding.setIniFileDirectory(iniFileDirectory);
}
exports.setIniFileDirectory = setIniFileDirectory;
//# sourceMappingURL=index.js.map
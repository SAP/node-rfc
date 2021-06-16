"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
const noderfc_bindings_1 = require("./noderfc-bindings");
const worker_threads_1 = require("worker_threads");
const err = noderfc_bindings_1.noderfc_binding.cancel(worker_threads_1.workerData);
if (err) {
    worker_threads_1.workerData.error = err;
}
else {
    worker_threads_1.workerData.result = "cancelled";
}
worker_threads_1.parentPort && worker_threads_1.parentPort.postMessage(worker_threads_1.workerData);
//# sourceMappingURL=cancel.js.map
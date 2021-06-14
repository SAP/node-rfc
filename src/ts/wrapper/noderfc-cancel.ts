import { noderfc_binding } from "./noderfc-bindings";
import { parentPort, workerData } from "worker_threads";
const err = noderfc_binding.cancel(workerData);
if (err) {
    workerData.error = err;
} else {
    workerData.result = "cancelled";
}

parentPort && parentPort.postMessage(workerData);

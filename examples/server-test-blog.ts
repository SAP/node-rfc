import { RfcLoggingLevel, Server } from "../lib/index.js";
import { authHandler, my_stfc_structure } from "./server_functions.ts";

// Create server instance, initially inactive
const server = new Server({
  serverConnection: { dest: "MME_GATEWAY" },
  clientConnection: { dest: "MME" },
  // Server options are optional
  serverOptions: {
    logLevel: RfcLoggingLevel.error,
    authHandler: authHandler,
  },
});

(async () => {
  try {
    // Register server function
    server.addFunction("STFC_STRUCTURE", my_stfc_structure);
    console.log(
      `[js] Node.js function '${my_stfc_structure.name}'`,
      "registered as ABAP 'STFC_STRUCTURE' function"
    );
    // Start the server
    await server.start();
    console.log(
      `[js] Server alive: ${server.alive} client handle: ${server.client_connection}`,
      `server handle: ${server.server_connection}`
    );
  } catch (ex) {
    // Catch errors, if any
    console.error(ex);
  }
})();

// Close the server after X seconds
let seconds = 10;

const tick = setInterval(() => {
  console.log("tick", --seconds);
  if (seconds <= 0) {
    server.stop(() => {
      clearInterval(tick);
      console.log("bye!");
    });
  }
}, 1000);

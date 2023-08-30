import { RFC_RC, RfcLoggingLevel, Server } from "../lib";
import {
  authHandler,
  my_stfc_structure,
  my_stfc_connection,
} from "./server_functions";

console.log("ok");
// Create server instance, initially inactive
const server = new Server({
  serverConnection: { dest: "MME_GATEWAY" },
  clientConnection: { dest: "MME" },
  // Server options are optional
  serverOptions: {
    logLevel: RfcLoggingLevel.all,
    authHandler: authHandler,
    bgRfcHandlers: {
      check: () => {
        return RFC_RC.RFC_OK;
      },
    },
  },
});

(async () => {
  try {
    // Register JS server functions to be exposed as ABAP functions
    await server.addFunction("STFC_CONNECTION", my_stfc_connection);
    console.log(
      `[js] Node.js function '${my_stfc_connection.name}'`,
      "registered as ABAP 'STFC_CONNECTION' function"
    );

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

// Close the server after 10 seconds
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

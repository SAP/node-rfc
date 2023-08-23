import { RfcLoggingLevel, Server } from "../lib/index.js";

// Create server instance, initially inactive
const server = new Server({
  serverConnection: { dest: "MME_GATEWAY" },
  clientConnection: { dest: "MME" },
  // Server options are optional
  serverOptions: {
    logLevel: RfcLoggingLevel.info,
  },
});

// Define couple of server functions
// One returning the promise
async function my_stfc_connection(request_context, abap_input) {
  const attr = request_context["connection_attributes"];
  console.log(
    "[js] my_stfc_connection context:",
    attr["sysId"],
    attr["client"],
    attr["user"],
    attr["progName"]
  );
  console.log("[js] my_stfc_conneciton input :", abap_input);

  const abap_output = {
    REQUTEXT: abap_input.REQUTEXT,
    ECHOTEXT: abap_input.REQUTEXT,
    RESPTEXT: `~~~ ${abap_input.REQUTEXT} ~~~`,
  };
  console.log(`[js] my_stfc_connection response: ${abap_output.RESPTEXT}`);

  return new Promise((resolve) => resolve(abap_output));
}

// Another one returning data
function my_stfc_structure(request_context, abap_input) {
  const connection_attributes = request_context["connection_attributes"];
  console.log(
    "[js] my_stfc_structure context:",
    connection_attributes["sysId"],
    connection_attributes["client"],
    connection_attributes["user"],
    connection_attributes["progName"]
  );
  console.log("[js] my_stfc_structure input:", abap_input.IMPORTSTRUCT);
  const echostruct = abap_input.IMPORTSTRUCT;
  echostruct.RFCINT1 = 2 * echostruct.RFCINT1;
  echostruct.RFCINT2 = 3 * echostruct.RFCINT2;
  echostruct.RFCINT4 = 4 * echostruct.RFCINT4;
  const abap_output = {
    ECHOSTRUCT: echostruct,
    RESPTEXT: `~~~ Node server here ~~~`,
  };

  console.log("[js] my_stfc_structure response:", abap_output);
  return abap_output;
}

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

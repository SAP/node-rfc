import { RfcLoggingLevel, Client, Pool, Server } from "../lib/index.js";

const server = new Server({
  serverConnection: { dest: "MME_GATEWAY" },
  clientConnection: { dest: "MME" },
  serverOptions: {
    logLevel: RfcLoggingLevel.debug,
  },
});

const delay = (seconds = 1) =>
  new Promise((resolve) => setTimeout(resolve, seconds * 1000));

async function my_stfc_connection(request_context, abap_input) {
  const connection_attributes = request_context["connection_attributes"];
  console.log(
    "[js] stfc context :",
    connection_attributes["sysId"],
    connection_attributes["client"],
    connection_attributes["user"],
    connection_attributes["cpicConvId"],
    connection_attributes["progName"]
  );

  const abap_output = {
    REQUTEXT: abap_input.REQUTEXT,
    ECHOTEXT: abap_input.REQUTEXT,
    RESPTEXT: `~~~ ${abap_input.REQUTEXT} ~~~`,
  };

  const respWait = abap_input.REQUTEXT[0];
  console.log("[js] stfc request :", abap_input, " wait", respWait);

  for (let ii = 0; ii < respWait; ii++) {
    await delay();
  }

  console.log(`[js] stfc_connection response: ${abap_output.RESPTEXT}`);
  return abap_output;
}

function my_stfc_structure(request_context, abap_input) {
  const connection_attributes = request_context["connection_attributes"];
  console.log(
    "[js] stfc context :",
    connection_attributes["sysId"],
    connection_attributes["client"],
    connection_attributes["user"],
    connection_attributes["cpicConvId"],
    connection_attributes["progName"]
  );
  console.log("[js] stfc request :"); // , abap_input);
  const echostruct = abap_input.IMPORTSTRUCT;
  echostruct.RFCINT1 = 2 * echostruct.RFCINT1;
  echostruct.RFCINT2 = 3 * echostruct.RFCINT2;
  echostruct.RFCINT4 = 4 * echostruct.RFCINT4;
  const abap_output = {
    ECHOSTRUCT: echostruct,
    RESPTEXT: `~~~ Node server here ~~~`,
  };

  console.log("[js] stfc_structure response:"); // , abap_output);
  return abap_output;
}

server.start((err) => {
  if (err) return console.error("error:", err);
  console.log(
    `[js] Server alive: ${server.alive} client handle: ${server.client_connection} server handle: ${server.server_connection}`
  );

  // Expose the my_stfc_connection function as RFM with STFC_CONNECTION pararameters (function definition)
  const RFM1 = "STFC_CONNECTION";
  server.addFunction(RFM1, my_stfc_connection, (err) => {
    if (err) return console.error(`error adding ${RFM1}: ${err}`);
    console.log(
      `[js] Node.js function '${my_stfc_connection.name}' registered as ABAP '${RFM1}' function`
    );
  });

  // Expose the my_stfc_connection function as RFM with STFC_CONNECTION pararameters (function definition)
  const RFM2 = "STFC_STRUCTURE";
  server.addFunction(RFM2, my_stfc_structure, (err) => {
    if (err) return console.error(`error adding ${RFM2}: ${err}`);
    console.log(
      `[js] Node.js function '${my_stfc_structure.name}' registered as ABAP '${RFM2}' function`
    );
  });
});

let i = 0;

const si = setInterval(() => {
  console.log("tick", i++);
}, 1000);

setTimeout(() => {
  server.stop(() => {
    clearInterval(si);
    console.log("bye!");
  });
}, 10 * 1000);

// my_stfc_connection({}, {});

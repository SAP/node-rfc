const addon = require("../lib/index.js");
const Server = addon.Server;
const server = new Server({
  serverConnection: { dest: "MME_GATEWAY" },
  clientConnection: { dest: "MME" },
  // serverOptions: {},
});
return;
const delay = (seconds) =>
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
  console.log("[js] stfc request :", abap_input);

  abap_output = {
    REQUTEXT: abap_input.REQUTEXT,
    ECHOTEXT: abap_input.REQUTEXT,
    RESPTEXT: `~~~ ${abap_input.REQUTEXT} ~~~`,
  };

  const respType = abap_input.REQUTEXT[0];
  const respWait = parseInt(abap_input.REQUTEXT[1]);
  if (respType == "P") {
    await delay(respWait);
  } else {
    for (let i = 1; i < 1000000000; i++) x = i / 3;
  }

  if (respType == "E") {
    throw new Error("some error");
  }

  console.log("[js] stfc_onnection response:", abap_output);
  return abap_output;
}

async function my_stfc_structure(request_context, abap_input) {
  const connection_attributes = request_context["connection_attributes"];
  console.log(
    "[js] stfc context :",
    connection_attributes["sysId"],
    connection_attributes["client"],
    connection_attributes["user"],
    connection_attributes["cpicConvId"],
    connection_attributes["progName"]
  );
  console.log("[js] stfc request :", abap_input);
  const echostruct = abap_input.IMPORTSTRUCT;
  echostruct.RFCINT1 = 2 * echostruct.RFCINT1;
  echostruct.RFCINT2 = 3 * echostruct.RFCINT2;
  echostruct.RFCINT4 = 4 * echostruct.RFCINT4;
  abap_output = {
    ECHOSTRUCT: echostruct,
    RESPTEXT: `~~~ Node server here ~~~`,
  };

  console.log("[js] stfc_structure response:", abap_output);
  return abap_output;
}

(async () => {
  await server.start();
  console.log(
    `[js] Server alive: ${server.alive} client handle: ${server.client_connection} server handle: ${server.server_connection}`
  );

  // Register JS server functions as ABAP functions
  const serverFunctions = [
    { abap: "STFC_CONNECTION", js: my_stfc_connection },
    { abap: "STFC_STRUCTURE", js: my_stfc_structure },
  ];
  serverFunctions.forEach(async (func_name) => {
    await server.addFunction(func_name.abap, func_name.js);
    console.log(
      `[js] '${func_name.js.name}' registered as ABAP function '${func_name.abap}'`
    );
  });

  i = 0;

  const si = setInterval(() => {
    console.log("tick", i++);
  }, 1000);

  setTimeout(() => {
    server.stop(() => {
      clearInterval(si);
      console.log("bye!");
    });
  }, 10 * 1000);
})();
// my_stfc_connection({}, {});

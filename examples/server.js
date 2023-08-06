const addon = require("../lib/index.js");
const Server = addon.Server;
const server = new Server({ dest: "MME_GATEWAY" }, { dest: "MME" });

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
  //for (let i = 1; i < 1000000000; i++) x = i / 3;
  await delay(10);
  abap_output = {
    REQUTEXT: abap_input.REQUTEXT,
    ECHOTEXT: abap_input.REQUTEXT,
    RESPTEXT: `~~~ ${abap_input.REQUTEXT} ~~~`,
  };
  console.log("[js] stfc response:", abap_output);
  //throw new Error("some error");
  return abap_output;
}

server.start((err) => {
  if (err) return console.error("error:", err);
  console.log(
    `[js] Server alive: ${server.alive} client handle: ${server.client_connection} server handle: ${server.server_connection}`
  );

  // Expose the my_stfc_connection function as RFM with STFC_CONNECTION pararameters (function definition)
  const RFM_NAME = "STFC_CONNECTION";
  server.addFunction(RFM_NAME, my_stfc_connection, (err) => {
    if (err) return console.error(`error adding ${RFM_NAME}: ${err}`);
    console.log(
      `[js] Node.js function '${my_stfc_connection.name}' registered as ABAP '${RFM_NAME}' function`
    );
  });
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
}, 30 * 1000);

// my_stfc_connection({}, {});

import { RfcSecurityAttributes, RfcAuthHandlerResponse } from "../lib";

// Server authorization handler
export function authHandler(
  context: RfcSecurityAttributes
): RfcAuthHandlerResponse {
  // To grant authorization return:
  //   - nothing (undefined)
  //   - true, empty string or RFC_OK
  // To deny authorization return:
  //   - false or non-empty string (passed as auth error message to ABAP)
  //   - or throw Exception error message
  console.log("[js] auth handler", context);
  // Here you can check any combination of function name, system ID, client,
  // calling ABAP report and username, as well as the backend’s SNC
  // credentials.
  let authorized = false;
  if (context.user == "ADMIN") {
    // User ADMIN is allowed to call function modules FUNCTION_1 FUNCTION_3
    if (
      ["FUNCTION_1", "FUNCTION_2", "FUNCTION_3"].includes(
        context.abapFunctionName
      )
    )
      authorized = true;
  } else if (context.user === "BUSINESS_USER") {
    // User BUSINESS_USER is allowed to call function modules  BAPI_A and BAPI_B.
    if (["BAPI_A", "BAPI_B"].includes(context.abapFunctionName))
      authorized = true;
  } else if (["DEMO", "D037732"].includes(context.user)) {
    if (
      ["STFC_CONNECTION", "STFC_STRUCTURE"].includes(context.abapFunctionName)
    )
      authorized = true;
  }

  const ret = authorized;
  // const ret = new Promise<boolean>((resolve) => resolve(authorized));
  //   const ret = new Promise<string>((resolve) =>
  //     resolve("unauthorized by Node.js")
  //   );
  // throw new Error(""); // ~~~ unauthorized by Node.js ~~~");
  console.log("[js] auth handler returns:", ret);
  return ret;
}

// JavaScript handler for ABAP STFC_CONNECTION function
export async function my_stfc_connection(request_context, abap_input) {
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

// JavaScript handler for ABAP STFC_STRUCTURE function
export function my_stfc_structure(request_context, abap_input) {
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

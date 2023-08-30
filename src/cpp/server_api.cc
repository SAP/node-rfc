// SPDX-FileCopyrightText: 2014 SAP SE Srdjan Boskovic <srdjan.boskovic@sap.com>
//
// SPDX-License-Identifier: Apache-2.0

#include "server_api.h"

namespace node_rfc {

//
// HandlerFunction
//

// When JS function handler is registered in Server::AddFunction,
// the HandlerFunction instance is created, with TSFN reference
// to JS function handler

HandlerFunction::HandlerFunction(Server* server,
                                 const std::string& _abap_func_name,
                                 RFC_ABAP_NAME _abap_func_name_sapuc,
                                 RFC_FUNCTION_DESC_HANDLE func_desc_handle,
                                 ServerRequestTsfn tsfnRequest,
                                 const std::string& jsFunctionName)
    : server(server),
      abap_func_name(_abap_func_name),
      func_desc_handle(func_desc_handle),
      tsfnRequest(tsfnRequest),
      jsFunctionName(jsFunctionName) {
  strncpyU(
      abap_func_name_sapuc, _abap_func_name_sapuc, _abap_func_name.length());
}

HandlerFunction::~HandlerFunction() {
  tsfnRequest.Release();
}

// get request id for request baton
std::string HandlerFunction::next_request_id() {
  return server->next_request_id();
}

// Register JS handler function
Napi::Value HandlerFunction::add_function(Server* server,
                                          Napi::Env env,
                                          Napi::String abapFunctionName,
                                          Napi::Function jsFunction) {
  // Obtain ABAP function description from ABAP system
  RFC_ERROR_INFO errorInfo;
  RFC_CHAR* abap_func_name_sapuc = setString(abapFunctionName);
  RFC_FUNCTION_DESC_HANDLE func_desc_handle = RfcGetFunctionDesc(
      server->client_conn_handle, abap_func_name_sapuc, &errorInfo);

  if (errorInfo.code != RFC_OK) {
    delete[] abap_func_name_sapuc;
    return rfcSdkError(&errorInfo);
  }

  // Create thread-safe JS function for genericRequestHandler
  std::string jsFunctionName =
      jsFunction.As<Napi::Object>().Get("name").As<Napi::String>().Utf8Value();

  ServerRequestTsfn tsfn =
      ServerRequestTsfn::New(env,
                             jsFunction,           // JavaScript server function
                             "ServerRequestTsfn",  // Resource name
                             0,                    // Unlimited queue
                             1  // Only one thread will use this initially
      );

  _log.info(logClass::server,
            "Function description ",
            (pointer_t)func_desc_handle,
            " added for JS function '" + jsFunctionName +
                "' as ABAP function '" + abapFunctionName.Utf8Value() + "'");

  HandlerFunction::installed_functions[abapFunctionName.Utf8Value()] =
      new HandlerFunction(server,
                          abapFunctionName.Utf8Value(),
                          abap_func_name_sapuc,
                          func_desc_handle,
                          tsfn,
                          jsFunctionName);

  delete[] abap_func_name_sapuc;
  return env.Undefined();
}

// Called by genericRequestHandler, to find JS handler function reference
HandlerFunction* HandlerFunction::get_function(
    RFC_CONNECTION_HANDLE conn_handle,
    RFC_FUNCTION_HANDLE func_handle,
    RFC_ERROR_INFO* errorInfo) {
  UNUSED(conn_handle);

  // Obtain ABAP function name
  RFC_FUNCTION_DESC_HANDLE func_desc =
      RfcDescribeFunction(func_handle, errorInfo);
  if (errorInfo->code != RFC_OK) {
    return nullptr;
  }
  RFC_ABAP_NAME abap_func_name_sapuc;
  RFC_RC rc = RfcGetFunctionName(func_desc, abap_func_name_sapuc, errorInfo);
  if (rc != RFC_OK || errorInfo->code != RFC_OK) {
    return nullptr;
  }

  // Find installed function
  for (const auto& [key, value] : installed_functions) {
    if (strcmpU(abap_func_name_sapuc, value->abap_func_name_sapuc) == 0) {
      _log.info(logClass::server,
                "JS function found: '" + value->jsFunctionName,
                "' for function handle ",
                (uintptr_t)func_handle,
                " of ABAP function '" + key + "'");
      return value;
    }
  }

  _log.error(logClass::server,
             "JS function not found for function handle ",
             (uintptr_t)func_handle,
             " of ABAP function ");
  _log.record_uc(logClass::server, logLevel::error, abap_func_name_sapuc);

  return nullptr;
}

// Called by metadataLookup RFC SDK server interface, to find
// function description of ABAP function requested by ABAP client
RFC_RC HandlerFunction::set_function_handle(
    SAP_UC const* abap_func_name,
    RFC_ATTRIBUTES rfc_attributes,
    RFC_FUNCTION_DESC_HANDLE* func_desc_handle) {
  UNUSED(rfc_attributes);

  for (const auto& [key, value] : installed_functions) {
    if (strcmpU(abap_func_name, value->abap_func_name_sapuc) == 0) {
      *func_desc_handle = value->func_desc_handle;

      _log.info(logClass::server,
                "metadataLookup: Function description set ",
                (pointer_t)*func_desc_handle,
                " for ABAP function '" + key + "'");

      return RFC_OK;
    }
  }

  _log.error(logClass::server,
             "metadataLookup: Function description not set for ABAP function ");
  _log.record_uc(logClass::server, logLevel::error, abap_func_name);

  return RFC_NOT_FOUND;
}

// Un-register JS handler function
Napi::Value HandlerFunction::remove_function(Napi::Function jsFunction) {
  std::string jsFunctionName =
      jsFunction.As<Napi::Object>().Get("name").As<Napi::String>().Utf8Value();

  for (const auto& [abap_func_name, value] : installed_functions) {
    if (jsFunctionName == value->jsFunctionName) {
      HandlerFunction::installed_functions.erase(abap_func_name);
      _log.info(logClass::server,
                "JS function removed " + jsFunctionName,
                " with ABAP function " + abap_func_name,
                " description: ",
                (pointer_t)value->func_desc_handle);
      return jsFunction.Env().Undefined();
    }
  }

  // Log and return error
  std::string errmsg =
      "Server removeFunction() did not find function: " + jsFunctionName;
  _log.error(logClass::server, errmsg);

  return nodeRfcError(errmsg);
}

// Called by Server destructor, to clean-up TSFN instances
void HandlerFunction::release(Server* server) {
  // release JS handler functions
  for (const auto& [abap_func_name, value] :
       HandlerFunction::installed_functions) {
    if (server == value->server) {
      value->tsfnRequest.Unref(server->env);
      _log.debug(logClass::server,
                 "unref '" + value->jsFunctionName,
                 "' with ABAP function '" + abap_func_name + "'");
    }
  }
}

// JS handler functions registered by server
std::unordered_map<std::string, HandlerFunction*>
    HandlerFunction::installed_functions;

//
// Authorization handler
//

AuthRequestHandler::AuthRequestHandler() : jsHandlerError("") {}

bool AuthRequestHandler::is_registered() {
  return tsfn_created;
}

void AuthRequestHandler::unregister() {
  if (tsfn_created) {
    _log.debug(logClass::server, "stop: auth handler unref");
    tsfn.Unref(__env);
    tsfn_created = false;
  }
}

void AuthRequestHandler::registerJsHandler(Napi::FunctionReference& func) {
  tsfn = AuthRequestTsfn::New(func.Env(),
                              func.Value(),       // JavaScript auth function
                              "AuthRequestTsfn",  // Resource name
                              0,                  // Unlimited queue
                              1  // Only one thread will use this initially)
  );
  tsfn_created = true;
  _log.debug(logClass::server, "stop: auth handler registered");
}

void AuthRequestHandler::start(RFC_ERROR_INFO* errorInfo) {
  RfcInstallAuthorizationCheckHandler(sapnwrfcServerAPI::authHandler,
                                      errorInfo);
}

// ABAP request data to JS
Napi::Value AuthRequestHandler::getRequestData() {
  Napi::EscapableHandleScope scope(__env);
  Napi::Object data = Napi::Object::New(__env);
  data.Set("connectionHandle",
           Napi::Number::New(__env, (pointer_t)auth_connection_handle));
  data.Set("abapFunctionName", wrapString(secAttributes->functionName));
  data.Set("sysId", wrapString(secAttributes->sysId));
  data.Set("client", wrapString(secAttributes->client));
  data.Set("user", wrapString(secAttributes->user));
  data.Set("progName", wrapString(secAttributes->progName));
  if (secAttributes->ssoTicket != nullptr)
    data.Set("ssoTicket", wrapString(secAttributes->ssoTicket));
  if (secAttributes->sncAclKeyLength > 0 && secAttributes->sncAclKey != nullptr)
    data.Set("sncAclKey",
             Napi::Buffer<SAP_RAW>::New(node_rfc::__env,
                                        this->secAttributes->sncAclKey,
                                        this->secAttributes->sncAclKeyLength));
  return scope.Escape(data);
}

// JS response data to ABAP
void AuthRequestHandler::setResponseData(Napi::Value jsAuthResponse) {
  // unauthorized by default
  errorInfo->code = RFC_AUTHORIZATION_FAILURE;

  if (jsAuthResponse.IsUndefined()) {
    // undefined -> authorized
    errorInfo->code = RFC_OK;
  } else if (jsAuthResponse.IsString()) {
    std::string message = jsAuthResponse.As<Napi::String>().Utf8Value();
    if (message.length() == 0) {
      // empty string -> authorized
      errorInfo->code = RFC_OK;
    } else {
      // non-empty string ->unauthorized, pass it as error message to ABAP
      strncpyU(errorInfo->message, setString(message), message.length());
    }
  } else if (jsAuthResponse.IsBoolean()) {
    // true -> authorized
    if (jsAuthResponse.As<Napi::Boolean>().Value()) {
      errorInfo->code = RFC_OK;
    }
  }

  // ABAP response is ready
  done();
};

RFC_RC SAP_API
AuthRequestHandler::callJS(RFC_CONNECTION_HANDLE conn_handle,
                           RFC_SECURITY_ATTRIBUTES* secAttributes,
                           RFC_ERROR_INFO* errorInfo) {
  this->auth_connection_handle = conn_handle;
  this->secAttributes = secAttributes;
  this->errorInfo = errorInfo;

  tsfn.NonBlockingCall(this);

  wait();  // for done() from JSAuthCall, when the ABAP response is ready

  return errorInfo->code;
}

void AuthRequestHandler::wait() {
  js_call_completed = false;
  std::unique_lock<std::mutex> lock(js_call_mutex);
  js_call_condition.wait(lock, [this] { return js_call_completed; });
}

void AuthRequestHandler::done(const std::string& errorObj) {
  js_call_completed = true;
  js_call_condition.notify_one();
  _log.record(logClass::server,
              (errorInfo->code != RFC_OK) ? logLevel::error : logLevel::debug,
              "JS auth handler call done: ",
              errorInfo->code,
              (errorInfo->code != RFC_OK) ? ", error: '" + errorObj : "'");
}

//
// GenericFunctionHandler
//

class GenericFunctionHandler {
 private:
  bool server_call_completed = false;
  std::mutex server_call_mutex;
  std::condition_variable server_call_condition;

 public:
  // Parameters from genericRequestHandler
  RFC_CONNECTION_HANDLE request_connection_handle;
  RFC_FUNCTION_HANDLE func_handle;
  RFC_ERROR_INFO* errorInfo = nullptr;
  // JS server function to handle the request
  HandlerFunction* handlerFunction = nullptr;
  // JS server function result
  std::string jsHandlerError;

  RfmErrorPath errorPath;

  ClientOptionsStruct client_options;

  // Server request id
  std::string request_id;

  GenericFunctionHandler() {}

  bool is_registered() { return true; }

  RFC_RC SAP_API callJS(RFC_CONNECTION_HANDLE conn_handle,

                        RFC_FUNCTION_HANDLE func_handle,
                        RFC_ERROR_INFO* errorInfo) {
    UNUSED(conn_handle);
    UNUSED(func_handle);
    UNUSED(errorInfo);
    request_id = this->handlerFunction->next_request_id();
    errorPath = RfmErrorPath();
    jsHandlerError = "";

    _log.info(logClass::server,
              "Client request [" + request_id + "]",
              " for JS function '",
              handlerFunction->jsFunctionName,
              "' using function handle ",
              (uintptr_t)func_handle,
              " and descriptor ",
              (uintptr_t)handlerFunction->func_desc_handle,
              "\n\tClient connection ",
              (uintptr_t)conn_handle,
              ", ABAP function ",
              handlerFunction->abap_func_name);
    return RFC_OK;
  }

  void wait() {
    _log.debug(logClass::server,
               "JS function call [" + request_id,
               "] lock '",
               handlerFunction->jsFunctionName,
               "' with ABAP function handle ",
               (uintptr_t)func_handle,
               " ",
               server_call_completed);
    std::unique_lock<std::mutex> lock(server_call_mutex);
    server_call_condition.wait(lock, [this] { return server_call_completed; });
    _log.debug(logClass::server,
               "JS function call [" + request_id,
               "] unlock '",
               handlerFunction->jsFunctionName,
               "' with ABAP function handle ",
               (uintptr_t)func_handle,
               " ",
               server_call_completed);
  }

  void done(const std::string& errorObj) {
    jsHandlerError = errorObj;
    _log.record(
        logClass::server,
        (jsHandlerError.length() > 0) ? logLevel::error : logLevel::debug,
        "JS function call [" + request_id,
        "] done '",
        handlerFunction->jsFunctionName,
        (jsHandlerError.length() > 0) ? "', error: " + jsHandlerError : "'",
        " ",
        server_call_completed);

    server_call_completed = true;
    server_call_condition.notify_one();
  }

  // Transform JavaScript parameters' data to ABAP
  void setResponseData(Napi::Env env, Napi::Value jsResult) {
    Napi::Value errorObj = env.Undefined();
    Napi::Object params = jsResult.As<Napi::Object>();
    Napi::Array paramNames = params.GetPropertyNames();
    uint_t paramCount = paramNames.Length();
    for (uint_t ii = 0; ii < paramCount; ii++) {
      Napi::String name = paramNames.Get(ii).ToString();
      Napi::Value value = params.Get(name);

      // _log.record(logClass::server, logLevel::debug, name, value);
      errorObj = setRfmParameter(handlerFunction->func_desc_handle,
                                 func_handle,
                                 name,
                                 value,
                                 &errorPath,
                                 &client_options);

      if (!errorObj.IsUndefined()) {
        break;
      }
    }

    // genericRequestHandler will check for error and return result to ABAP
    done(errorObj.IsUndefined() ? "" : errorObj.ToString().Utf8Value());
  }
};

//
// Server API
//

AuthRequestHandler sapnwrfcServerAPI::authorizationHandler =
    AuthRequestHandler();

GenericFunctionHandler sapnwrfcServerAPI::genericFunctionHandler =
    GenericFunctionHandler();

//
// ServerRequestBaton
//

// Created in genericRequestHandler, to wait for JS handler response,
// check for errors and send response to ABAP system

ServerRequestBaton::ServerRequestBaton(RFC_CONNECTION_HANDLE conn_handle,
                                       RFC_FUNCTION_HANDLE func_handle,
                                       RFC_ERROR_INFO* errorInfo,
                                       HandlerFunction* handlerFunction)
    : request_connection_handle(conn_handle),
      func_handle(func_handle),
      errorInfo(errorInfo),
      handlerFunction(handlerFunction) {
  request_id = this->handlerFunction->next_request_id();
  errorPath = RfmErrorPath();
  jsHandlerError = "";

  _log.info(logClass::server,
            "Client request [" + request_id + "]",
            " for JS function '",
            handlerFunction->jsFunctionName,
            "' using function handle ",
            (uintptr_t)func_handle,
            " and descriptor ",
            (uintptr_t)handlerFunction->func_desc_handle,
            "\n\tClient connection ",
            (uintptr_t)conn_handle,
            ", ABAP function ",
            handlerFunction->abap_func_name);
}

void ServerRequestBaton::wait() {
  _log.debug(logClass::server,
             "JS function call [" + request_id,
             "] lock '",
             handlerFunction->jsFunctionName,
             "' with ABAP function handle ",
             (uintptr_t)func_handle,
             " ",
             server_call_completed);
  std::unique_lock<std::mutex> lock(server_call_mutex);
  server_call_condition.wait(lock, [this] { return server_call_completed; });
  _log.debug(logClass::server,
             "JS function call [" + request_id,
             "] unlock '",
             handlerFunction->jsFunctionName,
             "' with ABAP function handle ",
             (uintptr_t)func_handle,
             " ",
             server_call_completed);
}

void ServerRequestBaton::done(const std::string& errorObj) {
  jsHandlerError = errorObj;
  _log.record(
      logClass::server,
      (jsHandlerError.length() > 0) ? logLevel::error : logLevel::debug,
      "JS function call [" + request_id,
      "] done '",
      handlerFunction->jsFunctionName,
      (jsHandlerError.length() > 0) ? "', error: " + jsHandlerError : "'",
      " ",
      server_call_completed);

  server_call_completed = true;
  server_call_condition.notify_one();
}

Napi::Value ServerRequestBaton::getServerRequestContext() {
  const std::string call_type[4] = {
      "synchronous", "transactional", "queued", "background_unit"};

  Napi::Object requestContext = Napi::Object::New(node_rfc::__env);

  requestContext.Set(
      "client_connection",
      Napi::Number::New(node_rfc::__env, (uintptr_t)request_connection_handle));
  requestContext.Set(
      "connection_attributes",
      getConnectionAttributes(node_rfc::__env, request_connection_handle));

  RFC_SERVER_CONTEXT context;

  RFC_RC rc =
      RfcGetServerContext(request_connection_handle, &context, errorInfo);

  if (rc != RFC_OK || errorInfo->code != RFC_OK) {
    _log.error(logClass::server, "Request context not set", requestContext);
  }

  requestContext.Set("callType", call_type[context.type]);
  requestContext.Set("isStateful",
                     Napi::Boolean::New(__env, context.isStateful != 0));

  return requestContext;
}

// Transform JavaScript parameters' data to ABAP
void ServerRequestBaton::setResponseData(Napi::Env env, Napi::Value jsResult) {
  Napi::Value errorObj = env.Undefined();
  Napi::Object params = jsResult.As<Napi::Object>();
  Napi::Array paramNames = params.GetPropertyNames();
  uint_t paramCount = paramNames.Length();
  for (uint_t ii = 0; ii < paramCount; ii++) {
    Napi::String name = paramNames.Get(ii).ToString();
    Napi::Value value = params.Get(name);

    // _log.record(logClass::server, logLevel::debug, name, value);
    errorObj = setRfmParameter(handlerFunction->func_desc_handle,
                               func_handle,
                               name,
                               value,
                               &errorPath,
                               &client_options);

    if (!errorObj.IsUndefined()) {
      break;
    }
  }

  // genericRequestHandler will check for error and return result to ABAP
  done(errorObj.IsUndefined() ? "" : errorObj.ToString().Utf8Value());
}

//
// SAP NW RFC SDK Server API
//

// Thread unsafe. Invoked by ABAP client to check if function description
// is available so that generic request handler can do the call
RFC_RC SAP_API
sapnwrfcServerAPI::metadataLookup(SAP_UC const* abap_func_name,
                                  RFC_ATTRIBUTES rfc_attributes,
                                  RFC_FUNCTION_DESC_HANDLE* func_desc_handle) {
  return HandlerFunction::set_function_handle(
      abap_func_name, rfc_attributes, func_desc_handle);
}

// Thread unsafe. Invoked by ABAP client, with client connection
// handle and function handle - a handle to function module data
// container.
// Here we obtain a function module name and look for TSFN object
// in handlerFunctions global map.
// When TSFN object found, the
RFC_RC SAP_API
sapnwrfcServerAPI::genericRequestHandler(RFC_CONNECTION_HANDLE conn_handle,
                                         RFC_FUNCTION_HANDLE func_handle,
                                         RFC_ERROR_INFO* errorInfo) {
  if (sapnwrfcServerAPI::genericFunctionHandler.is_registered()) {
  }
  // if (sapnwrfcServerAPI::genericFunctionHandler.registered()) {
  //   return sapnwrfcServerAPI::genericFunctionHandler.callJS(
  //       conn_handle, func_handle, errorInfo);
  // }
  // return RFC_EXTERNAL_FAILURE;

  // Check if JS handler function registered
  HandlerFunction* handlerFunction =
      HandlerFunction::get_function(conn_handle, func_handle, errorInfo);

  // Create request "baton" for TSFN call, to pass ABAP data
  // and wait until JS handler function completed.
  // The JS handler is invoked via thread safe JSHandlerCall
  ServerRequestBaton* requestBaton = new ServerRequestBaton(
      conn_handle, func_handle, errorInfo, handlerFunction);

  // Call JS handler function
  handlerFunction->tsfnRequest.NonBlockingCall(requestBaton);

  // Wait for JS functionn return and mutex release in JSHandlerCall
  requestBaton->wait();

  if (requestBaton->jsHandlerError.length() > 0) {
    // return the error message to ABAP
    // ref Table 5-A at pg. 48 of SAP NW RFC SDK Programming Guide
    strncpyU(errorInfo->message, setString(requestBaton->jsHandlerError), 512);
    return RFC_EXTERNAL_FAILURE;
  }

  return RFC_OK;
}

RFC_RC SAP_API
sapnwrfcServerAPI::authHandler(RFC_CONNECTION_HANDLE rfcHandle,
                               RFC_SECURITY_ATTRIBUTES* secAttributes,
                               RFC_ERROR_INFO* errorInfo) {
  if (sapnwrfcServerAPI::authorizationHandler.is_registered()) {
    // Auth handler is registered
    return sapnwrfcServerAPI::authorizationHandler.callJS(
        rfcHandle, secAttributes, errorInfo);
  }
  // otherwise grant the authorization
  return RFC_OK;
}

RFC_RC SAP_API sapnwrfcServerAPI::bgRfcCheck(
    RFC_CONNECTION_HANDLE rfcHandle, const RFC_UNIT_IDENTIFIER* identifier) {
  UNUSED(rfcHandle);
  UNUSED(identifier);
  return RFC_OK;
};
RFC_RC SAP_API sapnwrfcServerAPI::bgRfcCommit(
    RFC_CONNECTION_HANDLE rfcHandle, const RFC_UNIT_IDENTIFIER* identifier) {
  UNUSED(rfcHandle);
  UNUSED(identifier);
  return RFC_OK;
};
RFC_RC SAP_API sapnwrfcServerAPI::bgRfcRollback(
    RFC_CONNECTION_HANDLE rfcHandle, const RFC_UNIT_IDENTIFIER* identifier) {
  UNUSED(rfcHandle);
  UNUSED(identifier);
  return RFC_OK;
};
RFC_RC SAP_API sapnwrfcServerAPI::bgRfcConfirm(
    RFC_CONNECTION_HANDLE rfcHandle, const RFC_UNIT_IDENTIFIER* identifier) {
  UNUSED(rfcHandle);
  UNUSED(identifier);
  return RFC_OK;
};
RFC_RC SAP_API
sapnwrfcServerAPI::bgRfcGetState(RFC_CONNECTION_HANDLE rfcHandle,
                                 const RFC_UNIT_IDENTIFIER* identifier,
                                 RFC_UNIT_STATE* unitState) {
  UNUSED(rfcHandle);
  UNUSED(identifier);
  UNUSED(unitState);
  return RFC_OK;
};

//
// Tsfn
//

// Thread safe call of JavaScript handler.
// Request "baton" is used to pass ABAP data and
// wait until JavaScript handler processing completed
void JSHandlerCall(Napi::Env env,
                   Napi::Function callback,
                   std::nullptr_t* context,
                   DataType requestBaton) {
  UNUSED(context);

  // s=get server context
  Napi::Value requestContext = requestBaton->getServerRequestContext();

  // Transform ABAP parameters' data to JavaScript
  ValuePair jsParameters =
      getRfmParameters(requestBaton->handlerFunction->func_desc_handle,
                       requestBaton->func_handle,
                       &requestBaton->errorPath,
                       &requestBaton->client_options);

  Napi::Value errorObj = jsParameters.first.As<Napi::Object>();
  Napi::Object abapArgs = jsParameters.second.As<Napi::Object>();

  if (!errorObj.IsUndefined()) {
    // error in getting ABAP RFM parameters, unlikely to happen ...
    requestBaton->done(errorObj.ToString().Utf8Value());
    return;
  }

  // Call JavaScript handler
  _log.info(logClass::server,
            "JS function call [" + requestBaton->request_id,
            "] start '",
            requestBaton->handlerFunction->jsFunctionName,
            "' with ABAP function handle ",
            (uintptr_t)requestBaton->func_handle);

  Napi::Value jsResult;
  try {
    jsResult = callback.Call({requestContext, abapArgs});
  } catch (const Error& e) {
    requestBaton->done(e.Message());
    return;
  }

  _log.info(logClass::server,
            "JS function call [" + requestBaton->request_id,
            "] end '",
            requestBaton->handlerFunction->jsFunctionName,
            "' with ABAP function handle ",
            (uintptr_t)requestBaton->func_handle,
            " returned ",
            (jsResult.IsPromise()) ? "promise" : "data");

  // Check if JS handler result is promise or data
  if (jsResult.IsPromise()) {
    Napi::Promise jsPromise = jsResult.As<Napi::Promise>();
    Napi::Function jsThen = jsPromise.Get("then").As<Napi::Function>();
    jsThen.Call(jsPromise,
                {Napi::Function::New(env,
                                     [=](const CallbackInfo& info) {
                                       Napi::Object jsResult =
                                           info[0].As<Napi::Object>();
                                       requestBaton->setResponseData(env,
                                                                     jsResult);
                                     }),
                 Napi::Function::New(env,
                                     [=](const CallbackInfo& info) {
                                       std::string jsHandlerError =
                                           "Server handler function failed";
                                       if (info.Length() > 0) {
                                         jsHandlerError =
                                             info[0].ToString().Utf8Value();
                                       }
                                       requestBaton->done(jsHandlerError);
                                     })

                });

  } else {
    requestBaton->setResponseData(env, jsResult);
  }
}

// Thread safe JavaScript auth handler call
// Request "baton" is used to pass ABAP data and
// wait until JavaScript auth handler processing completed
void JSAuthCall(Napi::Env env,
                Napi::Function callback,
                std::nullptr_t* context,
                DataTypeAuth requestBaton) {
  UNUSED(context);

  // get JS request data
  Napi::Value requestData = requestBaton->getRequestData();

  // call JS auth handler
  Napi::Value jsResult;
  try {
    jsResult = callback.Call({requestData});
  } catch (const Error& e) {
    requestBaton->setResponseData(Napi::String::New(env, e.Message()));
    return;
  }

  _log.info(logClass::server,
            "JS auth handler call end returned ",
            jsResult.IsPromise() ? "promise" : "data");

  // set ABAP response data
  if (jsResult.IsPromise()) {
    Napi::Promise jsPromise = jsResult.As<Napi::Promise>();
    Napi::Function jsThen = jsPromise.Get("then").As<Napi::Function>();
    jsThen.Call(jsPromise,
                {Napi::Function::New(env,
                                     [=](const CallbackInfo& info) {
                                       Napi::Object jsResult =
                                           info[0].As<Napi::Object>();
                                       requestBaton->setResponseData(jsResult);
                                     }),
                 Napi::Function::New(env, [=](const CallbackInfo& info) {
                   // if error message not empty, send back to ABAP
                   if (info.Length() > 0) {
                     if (info[0].ToString().Utf8Value() != "Error") {
                       return requestBaton->setResponseData(info[0].ToString());
                     }
                   }
                   // otherwise send the default error message
                   requestBaton->setResponseData(Napi::String::New(
                       env, "Authorization denied by Node.js"));
                 })});

  } else {
    requestBaton->setResponseData(jsResult);
  }
}

}  // namespace node_rfc
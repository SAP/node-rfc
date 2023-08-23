// SPDX-FileCopyrightText: 2014 SAP SE Srdjan Boskovic <srdjan.boskovic@sap.com>
//
// SPDX-License-Identifier: Apache-2.0

#include "Server.h"
#include <node_api.h>

namespace node_rfc {

uint_t Server::_id = 1;
uint_t Server::request_id = 0;

class sapnwrfcServerAPI;

using DataType = ServerRequestBaton*;
void JSHandlerCall(Napi::Env env,
                   Napi::Function callback,
                   std::nullptr_t* context,
                   DataType requestBaton);

using ServerRequestTsfn = Napi::
    TypedThreadSafeFunction<std::nullptr_t, ServerRequestBaton, JSHandlerCall>;

// When JS function handler is registered in Server::AddFunction,
// the HandlerFunction instance is created, with TSFN reference
// to JS function handler
class HandlerFunction {
 public:
  Server* server;
  // Parameters from metadataLookup
  std::string abap_func_name;
  RFC_ABAP_NAME abap_func_name_sapuc;
  RFC_FUNCTION_DESC_HANDLE func_desc_handle;
  ServerRequestTsfn tsfnRequest;
  std::string jsFunctionName;
  // Installed JS handler functions
  static std::unordered_map<std::string, HandlerFunction*> installed_functions;
  // Installed JS auth handler function
  static Napi::FunctionReference authHandlerJS;

  HandlerFunction(Server* server,
                  std::string _abap_func_name,
                  RFC_ABAP_NAME _abap_func_name_sapuc,
                  RFC_FUNCTION_DESC_HANDLE func_desc_handle,
                  ServerRequestTsfn tsfnRequest,
                  const std::string& jsFunctionName)
      : server(server),
        abap_func_name(_abap_func_name),
        func_desc_handle(func_desc_handle),
        tsfnRequest(tsfnRequest),
        jsFunctionName(jsFunctionName) {
    strcpyU(abap_func_name_sapuc, _abap_func_name_sapuc);
  }
  ~HandlerFunction() { tsfnRequest.Release(); }

  // get request id for request baton
  std::string next_request_id() { return server->next_request_id(); }

  // Called by metadataLookup RFC SDK server interface, to find
  // function description of ABAP function requested by ABAP client
  static RFC_RC set_function_handle(
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
                  " for ABAP function '",
                  key,
                  "'");

        return RFC_OK;
      }
    }

    _log.error(
        logClass::server,
        "metadataLookup: Function description not set for ABAP function: ");
    _log.record_uc(logClass::server, logLevel::error, abap_func_name);

    return RFC_NOT_FOUND;
  }

  // Called by genericRequestHandler, to find JS handler function reference
  static HandlerFunction* get_function(RFC_CONNECTION_HANDLE conn_handle,
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

  // Register JS handler function
  static Napi::Value add_function(Server* server,
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

    // ABAP function description found.
    // Create thread-safe JS function for genericRequestHandler
    std::string jsFunctionName = jsFunction.As<Napi::Object>()
                                     .Get("name")
                                     .As<Napi::String>()
                                     .Utf8Value();

    ServerRequestTsfn tsfnRequest =
        ServerRequestTsfn::New(env,
                               jsFunction,  // JavaScript server function
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
                            tsfnRequest,
                            jsFunctionName);

    delete[] abap_func_name_sapuc;
    return env.Undefined();
  }

  // Un-register JS handler function
  static Napi::Value remove_function(Napi::Function jsFunction) {
    std::string jsFunctionName = jsFunction.As<Napi::Object>()
                                     .Get("name")
                                     .As<Napi::String>()
                                     .Utf8Value();

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
  static void release(Server* server) {
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
};

// JS handler functions registered by server
std::unordered_map<std::string, HandlerFunction*>
    HandlerFunction::installed_functions;

//
// Authorization handler
//

using DataTypeAuth = AuthRequestBaton*;
void JSAuthCall(Napi::Env env,
                Napi::Function callback,
                std::nullptr_t* context,
                DataTypeAuth requestBaton);

using AuthRequestTsfn =
    Napi::TypedThreadSafeFunction<std::nullptr_t, AuthRequestBaton, JSAuthCall>;

class AuthRequestBaton {
 private:
  // set when auth handler registered
  AuthRequestTsfn tsfn;
  // set by SAP NW RFC SDK API
  RFC_CONNECTION_HANDLE auth_connection_handle;
  RFC_SECURITY_ATTRIBUTES* secAttributes;
  RFC_ERROR_INFO* errorInfo;
  // set when JS auth handler registered
  bool tsfn_created = false;

  // js handler synchronization
  bool js_call_completed = false;
  std::mutex js_call_mutex;
  std::condition_variable js_call_condition;

  std::string jsHandlerError;

 public:
  AuthRequestBaton() { jsHandlerError = ""; }

  bool is_registered() { return tsfn_created; }

  void unregister() {
    if (tsfn_created) {
      _log.debug(logClass::server, "stop: auth handler unref");
      tsfn.Unref(__env);
      tsfn_created = false;
    }
  }

  void registerJsHandler(Napi::FunctionReference& func) {
    tsfn = AuthRequestTsfn::New(func.Env(),
                                func.Value(),       // JavaScript auth function
                                "AuthRequestTsfn",  // Resource name
                                0,                  // Unlimited queue
                                1  // Only one thread will use this initially)
    );
    tsfn_created = true;
    _log.debug(logClass::server, "stop: auth handler registered");
  }

  // ABAP request data to JS
  Napi::Value getRequestData() {
    Napi::EscapableHandleScope scope(__env);
    Napi::Object data = Napi::Object::New(__env);
    data.Set("connectionHandle",
             Napi::Number::New(__env, (pointer_t)auth_connection_handle));
    data.Set("AbapFunctionName", wrapString(secAttributes->functionName));
    data.Set("sysId", wrapString(secAttributes->sysId));
    data.Set("client", wrapString(secAttributes->client));
    data.Set("user", wrapString(secAttributes->user));
    data.Set("progName", wrapString(secAttributes->progName));
    if (secAttributes->ssoTicket != nullptr)
      data.Set("ssoTicket", wrapString(secAttributes->ssoTicket));
    if (secAttributes->sncAclKeyLength > 0 &&
        secAttributes->sncAclKey != nullptr)
      data.Set(
          "sncAclKey",
          Napi::Buffer<SAP_RAW>::New(node_rfc::__env,
                                     this->secAttributes->sncAclKey,
                                     this->secAttributes->sncAclKeyLength));
    return scope.Escape(data);
  }

  // JS response data to ABAP
  void setResponseData(Napi::Value jsAuthResponse) {
    if (jsAuthResponse.IsUndefined()) {
      errorInfo->code = RFC_OK;
    } else {
      errorInfo->code = RFC_AUTHORIZATION_FAILURE;
      strcpyU(errorInfo->message, cU("Unauthorized"));
    }

    // ABAP response is ready
    done();
  };

  RFC_RC SAP_API callJS(RFC_CONNECTION_HANDLE conn_handle,
                        RFC_SECURITY_ATTRIBUTES* secAttributes,
                        RFC_ERROR_INFO* errorInfo) {
    this->auth_connection_handle = conn_handle;
    this->secAttributes = secAttributes;
    this->errorInfo = errorInfo;

    tsfn.NonBlockingCall(this);

    wait();  // for done() from JSAuthCall, when the ABAP response is ready

    return errorInfo->code;
  }

  void wait() {
    js_call_completed = false;
    std::unique_lock<std::mutex> lock(js_call_mutex);
    js_call_condition.wait(lock, [this] { return js_call_completed; });
  }

  void done(const std::string& errorObj = "") {
    jsHandlerError = errorObj;
    js_call_completed = true;
    js_call_condition.notify_one();
    _log.record(
        logClass::server,
        (jsHandlerError.length() > 0) ? logLevel::error : logLevel::debug,
        "JS auth handler call done ",
        (jsHandlerError.length() > 0) ? "', error: " + jsHandlerError : "'");
  }
};

class sapnwrfcServerAPI {
 public:
  static AuthRequestBaton authorizationHandler;

  // ~sapnwrfcServerAPI() {
  //   if (sapnwrfcServerAPI::authorizationHandler != nullptr) {
  //     sapnwrfcServerAPI::authorizationHandler.Release();
  //   }
  // }
};

AuthRequestBaton sapnwrfcServerAPI::authorizationHandler = AuthRequestBaton();

// Created in genericRequestHandler, to wait for JS handler response,
// check for errors and send response to ABAP system
class ServerRequestBaton {
 private:
  bool server_call_completed = false;
  std::mutex server_call_mutex;
  std::condition_variable server_call_condition;

 public:
  // Parameters from genericRequestHandler
  RFC_CONNECTION_HANDLE request_connection_handle;
  RFC_FUNCTION_HANDLE func_handle;
  RFC_ERROR_INFO* errorInfo;
  // JS server function to handle the request
  HandlerFunction* handlerFunction;
  // JS server function result
  std::string jsHandlerError;

  RfmErrorPath errorPath;

  ClientOptionsStruct client_options;

  // Server request id
  std::string request_id;

  ServerRequestBaton(RFC_CONNECTION_HANDLE conn_handle,
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

  Napi::Value getServerRequestContext() {
    const std::string call_type[4] = {
        "synchronous", "transactional", "queued", "background_unit"};

    Napi::Object requestContext = Napi::Object::New(node_rfc::__env);

    requestContext.Set("client_connection",
                       Napi::Number::New(node_rfc::__env,
                                         (uintptr_t)request_connection_handle));
    requestContext.Set(
        "connection_attributes",
        getConnectionAttributes(node_rfc::__env, request_connection_handle));

    RFC_SERVER_CONTEXT context;

    RFC_RC rc =
        RfcGetServerContext(request_connection_handle, &context, errorInfo);

    if (rc != RFC_OK || errorInfo->code != RFC_OK) {
      _log.error(logClass::server, "Request context not set", requestContext);
    }
    return requestContext;
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
// Server
//

void getServerOptions(Napi::Object serverOptions,
                      ServerOptions* server_options) {
  Napi::Array optionNames = serverOptions.GetPropertyNames();
  uint_t optionCount = optionNames.Length();
  for (uint_t ii = 0; ii < optionCount; ii++) {
    std::string name = optionNames.Get(ii).ToString().Utf8Value();
    Napi::Value value = serverOptions.Get(name);
    if (name == SRV_OPTION_LOG_LEVEL) {
      _log.set_log_level(logClass::server, value);
    } else if (name == SRV_OPTION_AUTH) {
      if (!value.IsFunction()) {
        Napi::TypeError::New(node_rfc::__env,
                             "Server option '" + name + "' must be JS function")
            .ThrowAsJavaScriptException();
        return;
      }
      server_options->log_severity = logLevel::none;
      server_options->authHandlerJS =
          Napi::Persistent(value.As<Napi::Function>());

      // Register authorization handler
      sapnwrfcServerAPI::authorizationHandler.registerJsHandler(
          server_options->authHandlerJS);

    } else {
      Napi::TypeError::New(node_rfc::__env,
                           "Server option not allowed: '" + name + "'")
          .ThrowAsJavaScriptException();
      return;
    }
  }
}

Napi::Object Server::Init(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);

  Napi::Function func =
      DefineClass(env,
                  "Server",
                  {
                      InstanceAccessor("_id", &Server::IdGetter, nullptr),
                      InstanceAccessor("_alive", &Server::AliveGetter, nullptr),
                      InstanceAccessor("_server_conn_handle",
                                       &Server::ServerConnectionHandleGetter,
                                       nullptr),
                      InstanceAccessor("_client_conn_handle",
                                       &Server::ClientConnectionHandleGetter,
                                       nullptr),
                      InstanceMethod("start", &Server::Start),
                      InstanceMethod("stop", &Server::Stop),
                      InstanceMethod("addFunction", &Server::AddFunction),
                      InstanceMethod("removeFunction", &Server::RemoveFunction),
                      InstanceMethod("getFunctionDescription",
                                     &Server::GetFunctionDescription),
                  });

  Napi::FunctionReference* constructor = new Napi::FunctionReference();
  *constructor = Napi::Persistent(func);
  constructor->SuppressDestruct();

  exports.Set("Server", func);
  return exports;
}

Napi::Value Server::IdGetter(const Napi::CallbackInfo& info) {
  return Napi::Number::New(info.Env(), id);
}

Napi::Value Server::AliveGetter(const Napi::CallbackInfo& info) {
  return Napi::Boolean::New(info.Env(), server_conn_handle != nullptr);
}

Napi::Value Server::ServerConnectionHandleGetter(
    const Napi::CallbackInfo& info) {
  return Napi::Number::New(
      info.Env(), (double)(unsigned long long)this->server_conn_handle);
}
Napi::Value Server::ClientConnectionHandleGetter(
    const Napi::CallbackInfo& info) {
  return Napi::Number::New(
      info.Env(), (double)(unsigned long long)this->client_conn_handle);
}

Server::Server(const Napi::CallbackInfo& info)
    : Napi::ObjectWrap<Server>(info) {
  RFC_ERROR_INFO errorInfo;
  char errmsg[ERRMSG_LENGTH] =
      "Server configuration parameters must be an object";

  if (info.Length() == 0) {
    Napi::TypeError::New(Env(), errmsg).ThrowAsJavaScriptException();
    return;
  }
  if (!info[0].IsObject()) {
    Napi::TypeError::New(Env(), errmsg).ThrowAsJavaScriptException();
    return;
  }

  init(info.Env());

  Napi::Object infoObj = info[0].As<Napi::Object>();

  if (!infoObj.Get("serverConnection").IsObject()) {
    Napi::TypeError::New(Env(),
                         "Server connection parameters must be an object  "
                         "'serverConnection'")
        .ThrowAsJavaScriptException();
    return;
  }

  if (!infoObj.Get("clientConnection").IsObject()) {
    Napi::TypeError::New(Env(),
                         "Client connection parameters must be an object "
                         "'clientConnection'")
        .ThrowAsJavaScriptException();
    return;
  }

  serverConfigurationRef = Napi::Persistent(info[0].As<Napi::Object>());

  Napi::Array paramNames = infoObj.GetPropertyNames();
  for (uint_t ii = 0; ii < paramNames.Length(); ii++) {
    std::string key = paramNames.Get(ii).ToString().Utf8Value();
    Napi::Object value = infoObj.Get(key).As<Napi::Object>();

    if (key == std::string("serverConnection")) {
      getConnectionParams(value, &server_params);
    } else if (key == std::string("clientConnection")) {
      getConnectionParams(value, &client_params);
    } else if (key == std::string("serverOptions")) {
      getServerOptions(value, &server_options);
      //
    } else {
      Napi::TypeError::New(node_rfc::__env,
                           "Server parameter not allowed: '" + key + "'")
          .ThrowAsJavaScriptException();
      return;
    }
  }

  // open client connection
  client_conn_handle = RfcOpenConnection(
      client_params.connectionParams, client_params.paramSize, &errorInfo);
  if (errorInfo.code != RFC_OK) {
    Napi::Error::New(info.Env(), rfcSdkError(&errorInfo).ToString())
        .ThrowAsJavaScriptException();
    return;
  }

  // create server
  serverHandle = RfcCreateServer(server_params.connectionParams, 1, &errorInfo);
  if (errorInfo.code != RFC_OK) {
    Napi::Error::New(info.Env(), rfcSdkError(&errorInfo).ToString())
        .ThrowAsJavaScriptException();
    return;
  }
  _log.info(logClass::server,
            "created: server handle ",
            (uintptr_t)serverHandle,
            " client connection ",
            (uintptr_t)client_conn_handle);
};

Napi::Value wrapUnitIdentifier(RFC_UNIT_IDENTIFIER* uIdentifier) {
  Napi::Object unitIdentifier = Napi::Object::New(node_rfc::__env);
  unitIdentifier.Set("queued", wrapString(&uIdentifier->unitType, 1));
  unitIdentifier.Set("id", wrapString(uIdentifier->unitID));
  return unitIdentifier;
}

Napi::Value wrapUnitAttributes(RFC_UNIT_ATTRIBUTES* u_attr) {
  Napi::Env env = node_rfc::__env;
  Napi::Object unitAttr = Napi::Object::New(node_rfc::__env);

  unitAttr.Set("kernel_trace",
               Napi::Boolean::New(env, u_attr->kernelTrace != 0));
  unitAttr.Set("sat_trace", Napi::Boolean::New(env, u_attr->satTrace != 0));
  unitAttr.Set("unit_history",
               Napi::Boolean::New(env, u_attr->unitHistory != 0));
  unitAttr.Set("lock", Napi::Boolean::New(env, u_attr->lock != 0));
  unitAttr.Set("no_commit_check",
               Napi::Boolean::New(env, u_attr->noCommitCheck != 0));
  unitAttr.Set("user", wrapString(u_attr->user, 12));
  unitAttr.Set("client", wrapString(u_attr->client, 3));
  unitAttr.Set("t_code", wrapString(u_attr->tCode, 20));
  unitAttr.Set("program", wrapString(u_attr->program, 40));
  unitAttr.Set("hostname", wrapString(u_attr->hostname, 40));
  unitAttr.Set("sending_date", wrapString(u_attr->sendingDate, 8));
  unitAttr.Set("sending_time", wrapString(u_attr->sendingTime, 6));

  return unitAttr;
}

Napi::Value getServerRequestContext(ServerRequestBaton* requestBaton) {
  const std::string call_type[4] = {
      "synchronous", "transactional", "queued", "background_unit"};

  Napi::Object requestContext = Napi::Object::New(node_rfc::__env);

  requestContext.Set(
      "client_connection",
      Napi::Number::New(node_rfc::__env,
                        (uintptr_t)requestBaton->request_connection_handle));
  requestContext.Set(
      "connection_attributes",
      getConnectionAttributes(node_rfc::__env,
                              requestBaton->request_connection_handle));

  RFC_SERVER_CONTEXT context;

  RFC_RC rc = RfcGetServerContext(requestBaton->request_connection_handle,
                                  &context,
                                  requestBaton->errorInfo);

  if (rc != RFC_OK || requestBaton->errorInfo->code != RFC_OK) {
    _log.error(logClass::server, "Request context not set", requestContext);
    return requestContext;
  }

  requestContext.Set(
      "callType", Napi::String::New(node_rfc::__env, call_type[context.type]));
  requestContext.Set(
      "isStateful",
      Napi::Boolean::New(node_rfc::__env, context.isStateful != 0));
  if (context.type != RFC_SYNCHRONOUS) {
    requestContext.Set("unitIdentifier",
                       wrapUnitIdentifier(context.unitIdentifier));
  }
  if (context.type == RFC_BACKGROUND_UNIT) {
    requestContext.Set("unitAttr", wrapUnitAttributes(context.unitAttributes));
  }
  return requestContext;
}

// Thread unsafe. Invoked by ABAP client to check if function description
// is available so that generic request handler can do the call
RFC_RC SAP_API metadataLookup(SAP_UC const* abap_func_name,
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
RFC_RC SAP_API genericRequestHandler(RFC_CONNECTION_HANDLE conn_handle,
                                     RFC_FUNCTION_HANDLE func_handle,
                                     RFC_ERROR_INFO* errorInfo) {
  // Check if JS handler function registered
  HandlerFunction* handlerFunction =
      HandlerFunction::get_function(conn_handle, func_handle, errorInfo);
  if (handlerFunction == nullptr) {
    return RFC_EXTERNAL_FAILURE;
  }

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

RFC_RC SAP_API authorizationHandler(RFC_CONNECTION_HANDLE rfcHandle,
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

class StartAsync : public Napi::AsyncWorker {
 public:
  StartAsync(Napi::Function& callback, Server* server)
      : Napi::AsyncWorker(callback), server(server) {}
  ~StartAsync() {}

  void Execute() {
    errorInfo.code = RFC_OK;
    server->LockMutex();
    _log.debug(logClass::server, "StartAsync locked");
    server->server_thread = std::thread(&Server::_start, server, &errorInfo);
    server->UnlockMutex();
    _log.debug(logClass::server, "StartAsync unlocked");
  }

  void OnOK() {
    Napi::EscapableHandleScope scope(Env());
    if (errorInfo.code != RFC_OK) {
      // Callback().Call({Env().Undefined()});
      Callback().Call({rfcSdkError(&errorInfo)});
    } else {
      Callback().Call({});
    }
    Callback().Reset();
  }

 private:
  Server* server;
  RFC_ERROR_INFO errorInfo;
};

class StopAsync : public Napi::AsyncWorker {
 public:
  StopAsync(Napi::Function& callback, Server* server)
      : Napi::AsyncWorker(callback), server(server) {}
  ~StopAsync() {}

  void Execute() {
    server->LockMutex();
    _log.info(logClass::server,
              "stop: server handle ",
              (pointer_t)server->serverHandle);
    server->_stop();

    server->UnlockMutex();
  }

  void OnOK() {
    Napi::EscapableHandleScope scope(Env());
    if (errorInfo.code != RFC_OK) {
      // Callback().Call({Env().Undefined()});
      Callback().Call({rfcSdkError(&errorInfo)});
    } else {
      Callback().Call({});
    }
    Callback().Reset();
  }

 private:
  Server* server;
  RFC_ERROR_INFO errorInfo;
};

class GetFunctionDescAsync : public Napi::AsyncWorker {
 public:
  GetFunctionDescAsync(Napi::Function& callback,
                       Server* server,
                       Napi::String functionName)
      : Napi::AsyncWorker(callback),
        server(server),
        functionName(functionName) {}
  ~GetFunctionDescAsync() {}

  void Execute() {
    server->LockMutex();
    func_name = setString(functionName);
    // Obtain ABAP function description from ABAP system
    func_desc_handle =
        RfcGetFunctionDesc(server->client_conn_handle, func_name, &errorInfo);
    delete[] func_name;
    server->UnlockMutex();
  }

  void OnOK() {
    if (errorInfo.code != RFC_OK) {
      Callback().Call({rfcSdkError(&errorInfo)});
    } else {
      Callback().Call({});
    }
    Callback().Reset();
  }

 private:
  Server* server;
  Napi::String functionName;
  SAP_UC* func_name = nullptr;
  RFC_FUNCTION_DESC_HANDLE func_desc_handle = nullptr;
  RFC_ERROR_INFO errorInfo;
};

Napi::Value Server::Start(const Napi::CallbackInfo& info) {
  if (!info[0].IsFunction()) {
    Napi::TypeError::New(info.Env(),
                         "Server start() requires a callback function")
        .ThrowAsJavaScriptException();
    return info.Env().Undefined();
  }

  Napi::Function callback = info[0].As<Napi::Function>();

  (new StartAsync(callback, this))->Queue();

  return info.Env().Undefined();
};

void Server::_start(RFC_ERROR_INFO* errorInfo) {
  // authorization handler
  if (sapnwrfcServerAPI::authorizationHandler.is_registered()) {
    RfcInstallAuthorizationCheckHandler(authorizationHandler, errorInfo);
    if (errorInfo->code != RFC_OK) {
      _log.error(
          logClass::server,
          "start: authorization handler not installed, ABAP error group: ",
          errorInfo->group,
          " code: ",
          errorInfo->code);
      return;
    }
    _log.info(logClass::server, "start: authorization handler installed");
  }

  // generic server function
  RfcInstallGenericServerFunction(
      genericRequestHandler, metadataLookup, errorInfo);
  if (errorInfo->code != RFC_OK) {
    _log.error(
        logClass::server,
        "start: generic request handler not installed, ABAP error group: ",
        errorInfo->group,
        " code: ",
        errorInfo->code);
    return;
  }
  _log.info(logClass::server, "start: generic request handler installed");

  // launch server
  RfcLaunchServer(serverHandle, errorInfo);
  if (errorInfo->code != RFC_OK) {
    _log.error(logClass::server,
               "start: launch failed, ABAP error group: ",
               errorInfo->group,
               " code: ",
               errorInfo->code);
    return;
  }
  _log.info(logClass::server,
            "start: launched server handle ",
            (pointer_t)serverHandle);
}

void Server::_stop() {
  if (serverHandle == nullptr) {
    _log.debug(logClass::server, "stop: already stopped");
    return;
  }

  if (sapnwrfcServerAPI::authorizationHandler.is_registered()) {
    sapnwrfcServerAPI::authorizationHandler.unregister();
  }

  _log.info(logClass::server,
            "stop: shutdown server handle ",
            (pointer_t)serverHandle);
  RfcShutdownServer(serverHandle, 60, nullptr);
  RfcDestroyServer(serverHandle, nullptr);
  serverHandle = nullptr;

  if (client_conn_handle != nullptr) {
    _log.info(logClass::server,
              "stop: close client connection ",
              (pointer_t)client_conn_handle);
    RfcCloseConnection(client_conn_handle, nullptr);
    client_conn_handle = nullptr;
  }

  if (!serverConfigurationRef.IsEmpty()) {
    // serverConfigurationRef.Unref();
  };

  // release registered tsfn functions
  HandlerFunction::release(this);

  if (server_thread.joinable()) {
    _log.debug(
        logClass::server, "stop: server thread join ", server_thread.get_id());
    server_thread.join();
  }
}

Napi::Value Server::Stop(const Napi::CallbackInfo& info) {
  if (!info[0].IsFunction()) {
    Napi::TypeError::New(info.Env(),
                         "Server stop() requires a callback function")
        .ThrowAsJavaScriptException();
    return info.Env().Undefined();
  }

  Napi::Function callback = info[0].As<Napi::Function>();

  (new StopAsync(callback, this))->Queue();

  return info.Env().Undefined();
};

// Obtain a function description of ABAP function, to be used as
// interface for JavaScript handler function.
// Create thread safe function to call JavaScript,
// when async client request received by genericRequestHandler.
// Save the reference to server instance, to be available in
// genericRequestHandler, if needed there.
// handlerFunctions global map is updated as follows
// - key:  ABAP function name as std::string
// - value:
//    server instance,
//    ABAP function name as SAP unicode
//    ABAP function description handle
//    JS function TSFN object
Napi::Value Server::AddFunction(const Napi::CallbackInfo& info) {
  if (!info[0].IsString()) {
    Napi::TypeError::New(info.Env(),
                         "Server addFunction() requires ABAP RFM name")
        .ThrowAsJavaScriptException();
    return info.Env().Undefined();
  }

  Napi::String abapFunctionName = info[0].As<Napi::String>();

  if (abapFunctionName.Utf8Value().length() == 0 ||
      abapFunctionName.Utf8Value().length() > 30) {
    Napi::TypeError::New(info.Env(),
                         "Server addFunction() accepts max. 30 characters "
                         "long ABAP RFM name")
        .ThrowAsJavaScriptException();
    return info.Env().Undefined();
  }

  if (!info[1].IsFunction()) {
    Napi::TypeError::New(
        info.Env(), "Server addFunction() requires a NodeJS function argument")
        .ThrowAsJavaScriptException();
    return info.Env().Undefined();
  }

  if (!info[2].IsFunction()) {
    Napi::TypeError::New(
        info.Env(),
        "Server addFunction() requires a callback function argument")
        .ThrowAsJavaScriptException();
    return info.Env().Undefined();
  }

  if (client_conn_handle == nullptr) {
    Napi::TypeError::New(
        info.Env(), "Server addFunction() requires an open client connection")
        .ThrowAsJavaScriptException();
    return info.Env().Undefined();
  }

  Napi::Function jsFunction = info[1].As<Napi::Function>();
  Napi::Function callback = info[2].As<Napi::Function>();

  Napi::Value errorInfo = HandlerFunction::add_function(
      this, info.Env(), abapFunctionName, jsFunction);

  callback.Call({errorInfo});
  return info.Env().Undefined();
};

Napi::Value Server::RemoveFunction(const Napi::CallbackInfo& info) {
  if (!info[0].IsFunction()) {
    Napi::TypeError::New(
        info.Env(),
        "Server removeFunction() requires JavaScript function argument")
        .ThrowAsJavaScriptException();
    return info.Env().Undefined();
  }

  Napi::Function jsFunction = info[0].As<Napi::Function>();

  Napi::Function callback = info[1].As<Napi::Function>();

  Napi::Value rc = HandlerFunction::remove_function(jsFunction);

  if (rc.IsUndefined()) {
    callback.Call({});
  } else {
    callback.Call({rc});
  }
  return info.Env().Undefined();
};

Napi::Value Server::GetFunctionDescription(const Napi::CallbackInfo& info) {
  if (!info[0].IsString()) {
    Napi::TypeError::New(
        info.Env(), "Server getFunctionDescription() requires ABAP RFM name")
        .ThrowAsJavaScriptException();
    return info.Env().Undefined();
  }

  if (!info[0].IsFunction()) {
    Napi::TypeError::New(
        info.Env(),
        "Server getFunctionDescription() requires a callback function")
        .ThrowAsJavaScriptException();
    return info.Env().Undefined();
  }

  Napi::String functionName = info[0].As<Napi::String>();
  Napi::Function callback = info[1].As<Napi::Function>();

  (new GetFunctionDescAsync(callback, this, functionName))->Queue();

  return info.Env().Undefined();
};

Server::~Server(void) {
  //  this->_stop();
  _log.debug(logClass::server, "~Server");
}

void Server::LockMutex() {
  // uv_sem_wait(&invocationMutex);
}

void Server::UnlockMutex() {
  // uv_sem_post(&invocationMutex);
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
    requestBaton->done(e.Message());
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
                 Napi::Function::New(env,
                                     [=](const CallbackInfo& info) {
                                       std::string jsHandlerError =
                                           "internal error";
                                       if (info.Length() > 0) {
                                         jsHandlerError =
                                             info[0].ToString().Utf8Value();
                                       }
                                       requestBaton->done(jsHandlerError);
                                     })

                });

  } else {
    requestBaton->setResponseData(jsResult);
  }
}

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
    jsThen.Call(
        jsPromise,
        {Napi::Function::New(env,
                             [=](const CallbackInfo& info) {
                               Napi::Object jsResult =
                                   info[0].As<Napi::Object>();
                               requestBaton->setResponseData(env, jsResult);
                             }),
         Napi::Function::New(env,
                             [=](const CallbackInfo& info) {
                               std::string jsHandlerError = "internal error";
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

}  // namespace node_rfc
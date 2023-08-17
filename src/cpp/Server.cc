// SPDX-FileCopyrightText: 2014 SAP SE Srdjan Boskovic <srdjan.boskovic@sap.com>
//
// SPDX-License-Identifier: Apache-2.0

#include "Server.h"
#include <node_api.h>

namespace node_rfc {

uint_t Server::_id = 1;
uint_t Server::request_id = 0;

using DataType = ServerRequestBaton*;

void JSFunctionCall(Napi::Env env,
                    Napi::Function callback,
                    std::nullptr_t* context,
                    DataType requestBaton);

using ServerRequestTsfn = Napi::
    TypedThreadSafeFunction<std::nullptr_t, ServerRequestBaton, JSFunctionCall>;

// When JS function handler is registered in Server::AddFunction,
// the ServerFunction instance is created, with TSFN reference
// to JS function handler
class ServerFunction {
 public:
  Server* server;
  // Parameters from metadataLookup
  RFC_ABAP_NAME abap_func_name_sapuc;
  RFC_FUNCTION_DESC_HANDLE func_desc_handle;
  ServerRequestTsfn tsfnRequest;
  std::string jsFunctionName;
  static std::unordered_map<std::string, ServerFunction*> installed_functions;

  ServerFunction(Server* server,
                 RFC_ABAP_NAME func_name,
                 RFC_FUNCTION_DESC_HANDLE func_desc_handle,
                 ServerRequestTsfn tsfnRequest,
                 const std::string& jsFunctionName)
      : server(server),
        func_desc_handle(func_desc_handle),
        tsfnRequest(tsfnRequest),
        jsFunctionName(jsFunctionName) {
    strcpyU(abap_func_name_sapuc, func_name);
  }
  ~ServerFunction() { tsfnRequest.Release(); }

  // get request id for request baton
  std::string next_request_id() { return server->next_request_id(); }

  // Called by metadataLookup handler, to find function description
  // for ABAP function requested by ABAP client
  static RFC_RC set_function_handle(
      SAP_UC const* abap_func_name,
      RFC_ATTRIBUTES rfc_attributes,
      RFC_FUNCTION_DESC_HANDLE* func_desc_handle) {
    UNUSED(rfc_attributes);

    for (const auto& [key, value] : installed_functions) {
      if (strcmpU(abap_func_name, value->abap_func_name_sapuc) == 0) {
        *func_desc_handle = value->func_desc_handle;

        _log.record(logClass::server,
                    logLevel::info,
                    "metadataLookup: Function description set ",
                    (pointer_t)*func_desc_handle,
                    " for ABAP function '",
                    key,
                    "'");

        return RFC_OK;
      }
    }

    _log.record(
        logClass::server,
        logLevel::error,
        "metadataLookup: Function description not set for ABAP function: ");
    _log.record_uc(logClass::server, logLevel::error, abap_func_name);

    return RFC_NOT_FOUND;
  }

  // Called by genericRequestHandler, to find JS handler function reference
  static ServerFunction* get_function(RFC_CONNECTION_HANDLE conn_handle,
                                      RFC_FUNCTION_HANDLE func_handle,
                                      RFC_ERROR_INFO* errorInfo) {
    UNUSED(conn_handle);

    // Obtain ABAP function name
    RFC_FUNCTION_DESC_HANDLE func_desc =
        RfcDescribeFunction(func_handle, errorInfo);
    if (errorInfo->code != RFC_OK) {
      return nullptr;
    }
    RFC_ABAP_NAME abap_func_name;
    RFC_RC rc = RfcGetFunctionName(func_desc, abap_func_name, errorInfo);
    if (rc != RFC_OK || errorInfo->code != RFC_OK) {
      return nullptr;
    }

    // Find installed function
    for (const auto& [key, value] : installed_functions) {
      if (strcmpU(abap_func_name, value->abap_func_name_sapuc) == 0) {
        _log.record(logClass::server,
                    logLevel::info,
                    "genericRequestHandler: JS function '",
                    value->jsFunctionName,
                    "' found for function handle ",
                    (uintptr_t)func_handle,
                    " of ABAP function '" + key + "'");
        return value;
      }
    }

    _log.record(logClass::server,
                logLevel::error,
                "genericRequestHandler: JS function not found for "
                "function handle ",
                (uintptr_t)func_handle,
                " of ABAP function ");
    _log.record_uc(logClass::server, logLevel::error, abap_func_name);

    return nullptr;
  }

  // Register JS handler function
  static Napi::Value add_function(Server* server,
                                  Napi::Env env,
                                  Napi::String abapFunctionName,
                                  Napi::Function jsFunction) {
    // Obtain ABAP function description from ABAP system
    RFC_ERROR_INFO errorInfo;
    RFC_CHAR* abap_func_name = setString(abapFunctionName);
    RFC_FUNCTION_DESC_HANDLE func_desc_handle = RfcGetFunctionDesc(
        server->client_conn_handle, abap_func_name, &errorInfo);

    if (errorInfo.code != RFC_OK) {
      delete[] abap_func_name;
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
                               jsFunction,           // JavaScript server
                               "ServerRequestTsfn",  // Resource name
                               0,                    // Unlimited queue
                               1  // Only one thread will use this initially
        );

    _log.record(logClass::server,
                logLevel::info,
                "Function description ",
                (pointer_t)func_desc_handle,
                " added for JS function '",
                jsFunctionName,
                "' as ABAP function '",
                abapFunctionName.Utf8Value(),
                "'");

    ServerFunction::installed_functions[abapFunctionName.Utf8Value()] =
        new ServerFunction(server,
                           abap_func_name,
                           func_desc_handle,
                           tsfnRequest,
                           jsFunctionName);

    delete[] abap_func_name;
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
        ServerFunction::installed_functions.erase(abap_func_name);
        _log.record(logClass::server,
                    logLevel::info,
                    "JS function removed " + jsFunctionName,
                    " with ABAP function " + abap_func_name,
                    " description: ",
                    (pointer_t)value->func_desc_handle);
        return jsFunction.Env().Undefined();
      }
    }

    // Log and return error
    std::string errmsg =
        "Server removeFunction() did not find registered function: " +
        jsFunctionName;
    _log.record(logClass::server, logLevel::error, errmsg);

    return nodeRfcError(errmsg);
  }

  // Called by Server destructor, to clean-up TSFN instances
  static void release(Server* server) {
    for (const auto& [abap_func_name, value] :
         ServerFunction::installed_functions) {
      if (server == value->server) {
        value->tsfnRequest.Unref(server->env);
        _log.record(logClass::server,
                    logLevel::debug,
                    "unref '" + value->jsFunctionName,
                    "' with ABAP function '" + abap_func_name + "'");
      }
    }
  }
};

// JS handler functions registered by Server instances
std::unordered_map<std::string, ServerFunction*>
    ServerFunction::installed_functions;

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
  ServerFunction* serverFunction;
  // JS server function result
  std::string jsHandlerError;

  RfmErrorPath errorPath;

  ClientOptionsStruct client_options;

  // Server request id
  std::string request_id;

  ServerRequestBaton(RFC_CONNECTION_HANDLE conn_handle,
                     RFC_FUNCTION_HANDLE func_handle,
                     RFC_ERROR_INFO* errorInfo,
                     ServerFunction* serverFunction)
      : request_connection_handle(conn_handle),
        func_handle(func_handle),
        errorInfo(errorInfo),
        serverFunction(serverFunction) {
    request_id = this->serverFunction->next_request_id();
    errorPath = RfmErrorPath();
    jsHandlerError = "";

    _log.record(logClass::server,
                logLevel::info,
                "Client request [" + request_id + "]",
                " for JS function '",
                serverFunction->jsFunctionName,
                "' using function handle ",
                (uintptr_t)func_handle,
                " and descriptor ",
                (uintptr_t)serverFunction->func_desc_handle,
                "\n\tClient connection ",
                (uintptr_t)conn_handle,
                ", ABAP function ");
    _log.record_uc(
        logClass::server, logLevel::info, serverFunction->abap_func_name_sapuc);
  }

  void wait() {
    _log.record(logClass::server,
                logLevel::debug,
                "JS function call [" + request_id,
                "] lock '",
                serverFunction->jsFunctionName,
                "' with ABAP function handle ",
                (uintptr_t)func_handle,
                " ",
                server_call_completed);
    std::unique_lock<std::mutex> lock(server_call_mutex);
    server_call_condition.wait(lock, [this] { return server_call_completed; });
    _log.record(logClass::server,
                logLevel::debug,
                "JS function call [" + request_id,
                "] unlock '",
                serverFunction->jsFunctionName,
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
        " done '",
        serverFunction->jsFunctionName,
        (jsHandlerError.length() > 0) ? "', error: " + jsHandlerError : "'",
        " ",
        server_call_completed);

    server_call_completed = true;
    server_call_condition.notify_one();
  }

  // Transform JavaScript parameters' data to ABAP
  void setABAPParameters(Napi::Env env, Napi::Value jsResult) {
    Napi::Value errorObj = env.Undefined();
    Napi::Object params = jsResult.As<Napi::Object>();
    Napi::Array paramNames = params.GetPropertyNames();
    uint_t paramCount = paramNames.Length();
    for (uint_t ii = 0; ii < paramCount; ii++) {
      Napi::String name = paramNames.Get(ii).ToString();
      Napi::Value value = params.Get(name);

      // _log.record(logClass::server, logLevel::debug, name, value);
      errorObj = setRfmParameter(serverFunction->func_desc_handle,
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
  UNUSED(server_options);
  char errmsg[ERRMSG_LENGTH];
  Napi::Array optionNames = serverOptions.GetPropertyNames();
  uint_t optionCount = optionNames.Length();
  for (uint_t ii = 0; ii < optionCount; ii++) {
    std::string name = optionNames.Get(ii).ToString().Utf8Value();
    Napi::Value value = serverOptions.Get(name);
    if (name == LOG_LEVEL_KEY) {
      _log.set_log_level(logClass::server, value);
    } else {
      snprintf(errmsg,
               ERRMSG_LENGTH - 1,
               "Server option not allowed: \"%s\"",
               name.c_str());
      Napi::TypeError::New(node_rfc::__env, errmsg)
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
      snprintf(errmsg,
               ERRMSG_LENGTH - 1,
               "Server parameter not allowed: \"%s\"",
               key.c_str());
      Napi::TypeError::New(node_rfc::__env, errmsg)
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
  _log.record(logClass::server,
              logLevel::info,
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
    _log.record(logClass::server,
                logLevel::error,
                "Request context not set",
                requestContext);
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
  return ServerFunction::set_function_handle(
      abap_func_name, rfc_attributes, func_desc_handle);
}

// Thread unsafe. Invoked by ABAP client, with client connection
// handle and function handle - a handle to function module data
// container.
// Here we obtain a function module name and look for TSFN object
// in serverFunctions global map.
// When TSFN object found, the
RFC_RC SAP_API genericRequestHandler(RFC_CONNECTION_HANDLE conn_handle,
                                     RFC_FUNCTION_HANDLE func_handle,
                                     RFC_ERROR_INFO* errorInfo) {
  // Check if JS handler function registered
  ServerFunction* serverFunction =
      ServerFunction::get_function(conn_handle, func_handle, errorInfo);
  if (serverFunction == nullptr) {
    return RFC_EXTERNAL_FAILURE;
  }

  // Create request "baton" for TSFN call, to pass ABAP data
  // and wait until JS handler function completed.
  // The JS handler is invoked via thread safe JSFunctionCall
  ServerRequestBaton* requestBaton = new ServerRequestBaton(
      conn_handle, func_handle, errorInfo, serverFunction);

  // Call JS handler function
  serverFunction->tsfnRequest.NonBlockingCall(requestBaton);

  // Wait for JS handler function done and mutex released in JSFunctionCall
  requestBaton->wait();

  if (requestBaton->jsHandlerError.length() > 0) {
    // return the error message to ABAP
    strncpyU(errorInfo->message, setString(requestBaton->jsHandlerError), 512);
    return RFC_EXTERNAL_FAILURE;
  }

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
    _log.record(logClass::server, logLevel::debug, "StartAsync locked");
    server->server_thread = std::thread(&Server::_start, server, &errorInfo);
    server->UnlockMutex();
    _log.record(logClass::server, logLevel::debug, "StartAsync unlocked");
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
    _log.record(logClass::server,
                logLevel::info,
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
  RfcInstallGenericServerFunction(
      genericRequestHandler, metadataLookup, errorInfo);
  if (errorInfo->code != RFC_OK) {
    return;
  }
  _log.record(logClass::server,
              logLevel::info,
              "start: generic request handler installed");

  RfcLaunchServer(serverHandle, errorInfo);
  if (errorInfo->code != RFC_OK) {
    return;
  }
  _log.record(logClass::server,
              logLevel::info,
              "start: launched server handle ",
              (pointer_t)serverHandle);
}

void Server::_stop() {
  if (serverHandle != nullptr) {
    _log.record(logClass::server,
                logLevel::info,
                "stop: shutdown server handle ",
                (pointer_t)serverHandle);
    RfcShutdownServer(serverHandle, 60, nullptr);
    RfcDestroyServer(serverHandle, nullptr);
    serverHandle = nullptr;
  }

  if (client_conn_handle != nullptr) {
    _log.record(logClass::server,
                logLevel::info,
                "stop: close client connection ",
                (pointer_t)client_conn_handle);
    RfcCloseConnection(client_conn_handle, nullptr);
    client_conn_handle = nullptr;
  }

  if (!serverConfigurationRef.IsEmpty()) {
    // serverConfigurationRef.Reset();
  };

  // release registered tsfn functions
  ServerFunction::release(this);

  if (server_thread.joinable()) {
    _log.record(logClass::server,
                logLevel::debug,
                "stop: server thread join ",
                server_thread.get_id());
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
// serverFunctions global map is updated as follows
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
    Napi::TypeError::New(
        info.Env(),
        "Server addFunction() accepts max. 30 characters long ABAP RFM name")
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

  Napi::Value errorInfo = ServerFunction::add_function(
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

  Napi::Value rc = ServerFunction::remove_function(jsFunction);

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
  this->_stop();
}

void Server::LockMutex() {
  // uv_sem_wait(&invocationMutex);
}

void Server::UnlockMutex() {
  // uv_sem_post(&invocationMutex);
}

// using DataType = ServerRequestBaton*;

// Thread safe call of JavaScript handler.
// Request "baton" is used to pass ABAP data and
// wait until JavaScript handler processing completed
void JSFunctionCall(Napi::Env env,
                    Napi::Function callback,
                    std::nullptr_t* context,
                    DataType requestBaton) {
  UNUSED(context);

  // set server context
  Napi::Value requestContext = getServerRequestContext(requestBaton);

  // Transform ABAP parameters' data to JavaScript
  ValuePair jsParameters =
      getRfmParameters(requestBaton->serverFunction->func_desc_handle,
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
  _log.record(logClass::server,
              logLevel::info,
              "JS function call [" + requestBaton->request_id,
              "] start '",
              requestBaton->serverFunction->jsFunctionName,
              "' with ABAP function handle ",
              (uintptr_t)requestBaton->func_handle);

  Napi::Value jsResult;
  try {
    jsResult = callback.Call({requestContext, abapArgs});
  } catch (const Error& e) {
    requestBaton->done(e.Message());
    return;
  }

  _log.record(logClass::server,
              logLevel::info,
              "JS function call [" + requestBaton->request_id,
              "] end '",
              requestBaton->serverFunction->jsFunctionName,
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
                               requestBaton->setABAPParameters(env, jsResult);
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
    requestBaton->setABAPParameters(env, jsResult);
  }
}

}  // namespace node_rfc
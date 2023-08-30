// SPDX-FileCopyrightText: 2014 SAP SE Srdjan Boskovic <srdjan.boskovic@sap.com>
//
// SPDX-License-Identifier: Apache-2.0

#include "Server.h"
#include <napi.h>
#include "server_api.h"

namespace node_rfc {

uint_t Server::_id = 1;
uint_t Server::request_id = 0;

//
// Server
//

void Server::getServerOptions(Napi::Object serverOptions,
                              ServerOptions* server_options) {
  Napi::Array optionNames = serverOptions.GetPropertyNames();
  for (uint_t ii = 0; ii < optionNames.Length(); ii++) {
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

    } else if (name == SRV_OPTION_BGRFC) {
      if (!value.IsObject()) {
        Napi::TypeError::New(
            node_rfc::__env,
            "Server option '" + name + "' must be an JS object")
            .ThrowAsJavaScriptException();
        return;
      }
      Napi::Array handlerNames = value.As<Napi::Object>().GetPropertyNames();
      for (uint_t jj = 0; jj < handlerNames.Length(); jj++) {
        std::string handler_name = handlerNames.Get(jj).ToString().Utf8Value();
        Napi::Value handlerFunction =
            value.As<Napi::Object>().Get(handler_name);
        if (!handlerFunction.IsFunction()) {
          Napi::TypeError::New(
              node_rfc::__env,
              "bgRFC handler '" + name + "' must be JS function")
              .ThrowAsJavaScriptException();
          return;
        }
        if (handler_name == SRV_OPTION_BGRFC_CHECK) {
          server_options->bgRfcHandlerCheck =
              Napi::Persistent(handlerFunction.As<Napi::Function>());
        } else if (handler_name == SRV_OPTION_BGRFC_COMMIT) {
          server_options->bgRfcHandlerCommit =
              Napi::Persistent(handlerFunction.As<Napi::Function>());
        } else if (handler_name == SRV_OPTION_BGRFC_CONFIRM) {
          server_options->bgRfcHandlerConfirm =
              Napi::Persistent(handlerFunction.As<Napi::Function>());
        } else if (handler_name == SRV_OPTION_BGRFC_ROLLBACK) {
          server_options->bgRfcHandlerRollback =
              Napi::Persistent(handlerFunction.As<Napi::Function>());
        } else if (handler_name == SRV_OPTION_BGRFC_GET_STATE) {
          server_options->bgRfcHandlerGetState =
              Napi::Persistent(handlerFunction.As<Napi::Function>());
        } else {
          Napi::TypeError::New(
              node_rfc::__env,
              "bgRFC handler name '" + handler_name + "' is not supported")
              .ThrowAsJavaScriptException();
          return;
        }
      }
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

  // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks)
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
  unitIdentifier.Set("queued", uIdentifier->unitType == cU("Q")[0]);
  unitIdentifier.Set("id", wrapString(uIdentifier->unitID));
  return unitIdentifier;
}

Napi::Value wrapUnitAttributes(const RFC_UNIT_ATTRIBUTES* u_attr) {
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
    sapnwrfcServerAPI::authorizationHandler.start(errorInfo);
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

  // generic server functions
  RfcInstallGenericServerFunction(sapnwrfcServerAPI::genericRequestHandler,
                                  sapnwrfcServerAPI::metadataLookup,
                                  errorInfo);
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

  // unregister API handlers
  if (sapnwrfcServerAPI::authorizationHandler.is_registered()) {
    sapnwrfcServerAPI::authorizationHandler.unregister();
  }
  _log.info(logClass::server,
            "stop: shutdown server handle ",
            (pointer_t)serverHandle);

  // shutdown server
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

}  // namespace node_rfc
// SPDX-FileCopyrightText: 2014 SAP SE Srdjan Boskovic <srdjan.boskovic@sap.com>
//
// SPDX-License-Identifier: Apache-2.0

#include "Server.h"
#include <node_api.h>

namespace node_rfc {
extern Napi::Env __env;

uint_t Server::_id = 1;
ServerFunctionsMap serverFunctions;

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

  init();

  DEBUG("Server::Server ", id);

  if (!info[0].IsObject()) {
    Napi::TypeError::New(Env(), "Server constructor requires server parameters")
        .ThrowAsJavaScriptException();
    return;
  }

  serverParamsRef = Napi::Persistent(info[0].As<Napi::Object>());
  getConnectionParams(serverParamsRef.Value(), &server_params);

  if (!info[1].IsObject()) {
    Napi::TypeError::New(Env(), "Server constructor requires client parameters")
        .ThrowAsJavaScriptException();
    return;
  }

  clientParamsRef = Napi::Persistent(info[1].As<Napi::Object>());
  getConnectionParams(clientParamsRef.Value(), &client_params);

  if (!info[2].IsUndefined()) {
    if (!info[2].IsObject()) {
      Napi::TypeError::New(
          Env(), "Server constructor client options must be an object")
          .ThrowAsJavaScriptException();
      return;
    }
    clientOptionsRef = Napi::Persistent(info[2].As<Napi::Object>());
    checkClientOptions(clientOptionsRef.Value(), &client_options);
  }

  // open client connection

  client_conn_handle = RfcOpenConnection(
      client_params.connectionParams, client_params.paramSize, &errorInfo);
  if (errorInfo.code != RFC_OK) {
    Napi::Error::New(info.Env(), rfcSdkError(&errorInfo).ToString())
        .ThrowAsJavaScriptException();
  }
  DEBUG("Server:: client connection open:", (uintptr_t)client_conn_handle);

  // create server
  serverHandle = RfcCreateServer(server_params.connectionParams, 1, &errorInfo);
  if (errorInfo.code != RFC_OK) {
    Napi::Error::New(info.Env(), rfcSdkError(&errorInfo).ToString())
        .ThrowAsJavaScriptException();
  }
  DEBUG("Server:: created ", (uintptr_t)serverHandle);
};

RFC_RC SAP_API metadataLookup(SAP_UC const* func_name,
                              RFC_ATTRIBUTES rfc_attributes,
                              RFC_FUNCTION_DESC_HANDLE* func_desc_handle) {
  UNUSED(rfc_attributes);

  RFC_RC rc = RFC_NOT_FOUND;

  ServerFunctionsMap::iterator it = serverFunctions.begin();
  while (it != serverFunctions.end()) {
    if (strcmpU(func_name, it->second->func_name) == 0) {
      *func_desc_handle = it->second->func_desc_handle;
      rc = RFC_OK;
      break;
    }
    ++it;
  }

  if (rc == RFC_OK) {
    DEBUG("Function found ",
          it->first,
          " descriptor ",
          (pointer_t)*func_desc_handle);
  } else {
    // todo
    DEBUG("[error] Function not found: ");
    printfU(func_name);
  }

  return rc;
}

Napi::Value wrapUnitIdentifier(RFC_UNIT_IDENTIFIER* uIdentifier) {
  Napi::Object unitIdentifier = Napi::Object::New(node_rfc::__env);
  unitIdentifier.Set("queued", wrapString(&uIdentifier->unitType, 1));
  unitIdentifier.Set("id", wrapString(uIdentifier->unitID));
  return unitIdentifier;
}

Napi::Value getServerRequestContext(ServerRequestBaton* requestBaton) {
  const std::string call_type[4] = {
      "synchronous", "transactional", "queued", "background_unit"};

  if (requestBaton->request_connection_handle == nullptr) {
    // todo error ...
  }

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

  if (rc != RFC_OK || requestBaton->errorInfo->code != RFC_OK)
    return requestContext;

  requestContext.Set(
      "callType", Napi::String::New(node_rfc::__env, call_type[context.type]));
  requestContext.Set(
      "isStateful",
      Napi::Boolean::New(node_rfc::__env, context.isStateful != 0));
  if (context.type != RFC_SYNCHRONOUS) {
    requestContext.Set("unitIdentifier",
                       wrapUnitIdentifier(context.unitIdentifier));
  }
  // if (context.type == RFC_BACKGROUND_UNIT) {
  //   requestContext.Set("unitAttributes",
  //       wrapUnitAttributes(context.unitAttributes);
  // }
  return requestContext;
}

RFC_RC SAP_API genericRequestHandler(RFC_CONNECTION_HANDLE conn_handle,
                                     RFC_FUNCTION_HANDLE func_handle,
                                     RFC_ERROR_INFO* errorInfo) {
  RFC_RC rc = RFC_NOT_FOUND;

  RFC_FUNCTION_DESC_HANDLE func_desc =
      RfcDescribeFunction(func_handle, errorInfo);
  if (errorInfo->code != RFC_OK) {
    return errorInfo->code;
  }
  RFC_ABAP_NAME func_name;
  RfcGetFunctionName(func_desc, func_name, errorInfo);
  if (errorInfo->code != RFC_OK) {
    return errorInfo->code;
  }

  ServerFunctionsMap::iterator it = serverFunctions.begin();
  while (it != serverFunctions.end()) {
    if (strcmpU(func_name, it->second->func_name) == 0) {
      break;
    }
    ++it;
  }

  if (it == serverFunctions.end()) {
    printf("not found!\n");
    return rc;
  }

  ServerRequestBaton* requestBaton = new ServerRequestBaton();
  auto errorPath = node_rfc::RfmErrorPath();

  requestBaton->request_connection_handle = conn_handle;
  requestBaton->client_options = it->second->server->client_options;
  requestBaton->func_handle = func_handle;
  requestBaton->func_desc_handle = it->second->func_desc_handle;
  requestBaton->errorInfo = errorInfo;
  requestBaton->errorPath = errorPath;

  DEBUG("Generic request handler: ",
        it->first,
        " function handle ",
        (uintptr_t)func_handle,
        " descriptor: ",
        (uintptr_t)requestBaton->func_desc_handle,
        " from client connection: ",
        (uintptr_t)conn_handle);

  it->second->tsfnRequest.NonBlockingCall(requestBaton);

  requestBaton->wait();

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
    DEBUG("StartAsync locked");

    server->st = std::thread(&Server::_start, server, &errorInfo);

    server->UnlockMutex();
    DEBUG("StartAsync unlocked");
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
    DEBUG("Server::stop ", (pointer_t)server->serverHandle);
    server->_stop();
    DEBUG("Server:: stopped")

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
  GetFunctionDescAsync(Napi::Function& callback, Server* server)
      : Napi::AsyncWorker(callback), server(server) {}
  ~GetFunctionDescAsync() {}

  void Execute() {
    server->LockMutex();
    server->server_conn_handle =
        RfcRegisterServer(server->server_params.connectionParams,
                          server->server_params.paramSize,
                          &errorInfo);
    server->UnlockMutex();
  }

  void OnOK() {
    if (server->server_conn_handle == nullptr) {
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

Napi::Value Server::Start(const Napi::CallbackInfo& info) {
  DEBUG("Server::Serve");

  std::ostringstream errmsg;

  if (!info[0].IsFunction()) {
    errmsg << "Server start() requires a callback function";
    Napi::TypeError::New(info.Env(), errmsg.str()).ThrowAsJavaScriptException();
    return info.Env().Undefined();
  }

  Napi::Function callback = info[0].As<Napi::Function>();

  (new StartAsync(callback, this))->Queue();

  return info.Env().Undefined();
};

// std::thread Server::server_thread(RFC_ERROR_INFO* errorInfo) {
//   return std::thread([=] { _start(errorInfo); });
// };

void Server::_start(RFC_ERROR_INFO* errorInfo) {
  RfcInstallGenericServerFunction(
      genericRequestHandler, metadataLookup, errorInfo);
  if (errorInfo->code != RFC_OK) {
    return;
  }
  DEBUG("Server:: generic request handler installed");

  RfcLaunchServer(serverHandle, errorInfo);
  if (errorInfo->code != RFC_OK) {
    return;
  }
  DEBUG("Server:: launched ", (pointer_t)serverHandle)
}

void Server::_stop() {
  if (serverHandle != nullptr) {
    DEBUG("Server stop: destroy ", (pointer_t)serverHandle);
    RfcShutdownServer(serverHandle, 60, nullptr);
    RfcDestroyServer(serverHandle, nullptr);
    serverHandle = nullptr;
  }

  if (client_conn_handle != nullptr) {
    DEBUG("Server stop: close client connection ",
          (pointer_t)client_conn_handle);
    RfcCloseConnection(client_conn_handle, nullptr);
    client_conn_handle = nullptr;
  }

  // release tsfn server functions
  ServerFunctionsMap::iterator it = serverFunctions.begin();
  while (it != serverFunctions.end()) {
    if (it->second->tsfnRequest) {
      it->second->tsfnRequest.Unref(node_rfc::__env);
    }
    ++it;
  }

  if (st.joinable()) {
    DEBUG("Server stop: serve() thread join")
    st.join();
  }
}

Napi::Value Server::Stop(const Napi::CallbackInfo& info) {
  DEBUG("Server::Stop");

  std::ostringstream errmsg;

  if (!info[0].IsFunction()) {
    errmsg << "Server stop() requires a callback function";
    Napi::TypeError::New(info.Env(), errmsg.str()).ThrowAsJavaScriptException();
    return info.Env().Undefined();
  }

  Napi::Function callback = info[0].As<Napi::Function>();

  (new StopAsync(callback, this))->Queue();

  return info.Env().Undefined();
};

Napi::Value Server::AddFunction(const Napi::CallbackInfo& info) {
  std::ostringstream errmsg;

  if (!info[0].IsString()) {
    errmsg << "Server addFunction() requires ABAP RFM name";
    Napi::TypeError::New(info.Env(), errmsg.str()).ThrowAsJavaScriptException();
    return info.Env().Undefined();
  }

  Napi::String functionName = info[0].As<Napi::String>();

  if (functionName.Utf8Value().length() == 0 ||
      functionName.Utf8Value().length() > 30) {
    errmsg << "Server addFunction() accepts max. 30 characters long ABAP RFM "
              "name";
    Napi::TypeError::New(info.Env(), errmsg.str()).ThrowAsJavaScriptException();
    return info.Env().Undefined();
  }

  if (!info[1].IsFunction()) {
    errmsg << "Server addFunction() requires a NodeJS handler function";
    Napi::TypeError::New(info.Env(), errmsg.str()).ThrowAsJavaScriptException();
    return info.Env().Undefined();
  }

  if (!info[2].IsFunction()) {
    errmsg << "Server addFunction() requires a callback function";
    Napi::TypeError::New(info.Env(), errmsg.str()).ThrowAsJavaScriptException();
    return info.Env().Undefined();
  }

  if (client_conn_handle == nullptr) {
    errmsg << "Server addFunction() requires an open client connection";
    Napi::TypeError::New(info.Env(), errmsg.str()).ThrowAsJavaScriptException();
    return info.Env().Undefined();
  }

  Napi::Function jsFunction = info[1].As<Napi::Function>();
  Napi::Function callback = info[2].As<Napi::Function>();

  // Install function
  RFC_ERROR_INFO errorInfo;

  SAP_UC* func_name = setString(functionName);

  // Obtain ABAP function description from ABAP system
  RFC_FUNCTION_DESC_HANDLE func_desc_handle =
      RfcGetFunctionDesc(client_conn_handle, func_name, &errorInfo);

  if (errorInfo.code != RFC_OK) {
    delete[] func_name;
    callback.Call({rfcSdkError(&errorInfo)});
    return info.Env().Undefined();
  }

  // ABAP function description found.
  // Create thread-safe JS function to be called by genericRequestHandler
  serverFunctions[functionName.Utf8Value()] = new ServerFunctionStruct(
      this,
      func_name,
      func_desc_handle,
      ServerRequestTsfn::New(info.Env(),
                             jsFunction,           // JavaScript server
                             "ServerRequestTsfn",  // Resource name
                             0,                    // Unlimited queue
                             1  // Only one thread will use this initially
                             )

  );

  // Napi::ThreadSafeFunction tsfn = Napi::ThreadSafeFunction::New(
  //     info.Env(),
  //     jsFunction,           // JavaScript server
  //     "ServerRequestTsfn",  // Resource name
  //     0,                    // Unlimited queue
  //     1                     // Only one thread will use this initially
  // );

  // ServerRequestBaton* sbn = new ServerRequestBaton();
  // tsfn.NonBlockingCall((void*)sbn);

  delete[] func_name;
  DEBUG("Function added ",
        functionName.Utf8Value(),
        " descriptor: ",
        (pointer_t)func_desc_handle);

  callback.Call({});
  return info.Env().Undefined();
};

Napi::Value Server::RemoveFunction(const Napi::CallbackInfo& info) {
  std::ostringstream errmsg;

  if (!info[0].IsString()) {
    errmsg << "Server removeFunction() requires ABAP RFM name";
    Napi::TypeError::New(info.Env(), errmsg.str()).ThrowAsJavaScriptException();
    return info.Env().Undefined();
  }

  Napi::String functionName = info[0].As<Napi::String>();

  if (functionName.Utf8Value().length() == 0 ||
      functionName.Utf8Value().length() > 30) {
    errmsg << "Server removeFunction() accepts max. 30 characters long ABAP "
              "RFM name";
    Napi::TypeError::New(info.Env(), errmsg.str()).ThrowAsJavaScriptException();
    return info.Env().Undefined();
  }

  DEBUG("Server::RemoveFunction ", functionName.Utf8Value());

  if (!info[1].IsFunction()) {
    errmsg << "Server removeFunction() requires a callback function";
    Napi::TypeError::New(info.Env(), errmsg.str()).ThrowAsJavaScriptException();
    return info.Env().Undefined();
  }

  Napi::Function callback = info[1].As<Napi::Function>();

  // Remove function

  SAP_UC* func_name = setString(functionName);

  ServerFunctionsMap::iterator it = serverFunctions.begin();
  while (it != serverFunctions.end()) {
    if (strcmpU(func_name, it->second->func_name) == 0) {
      break;
    }
    ++it;
  }
  delete[] func_name;

  if (it == serverFunctions.end()) {
    errmsg << "Server removeFunction() did not find function: "
           << functionName.Utf8Value();
    callback.Call({nodeRfcError(errmsg.str())});
    return info.Env().Undefined();
  }

  DEBUG("Server::RemoveFunction removed ",
        functionName.Utf8Value(),
        ": ",
        (pointer_t)it->second->func_desc_handle);
  serverFunctions.erase(it);

  callback.Call({});
  return info.Env().Undefined();
};

Napi::Value Server::GetFunctionDescription(const Napi::CallbackInfo& info) {
  DEBUG("Server::GetFunctionDescription");

  std::ostringstream errmsg;

  if (!info[0].IsString()) {
    errmsg << "Server getFunctionDescription() requires ABAP RFM name";
    Napi::TypeError::New(info.Env(), errmsg.str()).ThrowAsJavaScriptException();
    return info.Env().Undefined();
  }

  if (!info[0].IsFunction()) {
    errmsg << "Server getFunctionDescription() requires a callback function";
    Napi::TypeError::New(info.Env(), errmsg.str()).ThrowAsJavaScriptException();
    return info.Env().Undefined();
  }

  Napi::Function callback = info[1].As<Napi::Function>();

  (new GetFunctionDescAsync(callback, this))->Queue();

  return info.Env().Undefined();
};

Server::~Server(void) {
  DEBUG("~ Server ", id);
  this->_stop();
}

void Server::LockMutex() {
  // uv_sem_wait(&invocationMutex);
}

void Server::UnlockMutex() {
  // uv_sem_post(&invocationMutex);
}

void ServerCallJs(Napi::Env env,
                  Napi::Function callback,
                  std::nullptr_t* context,
                  ServerRequestBaton* requestBaton) {
  UNUSED(context);

  // set server context
  Napi::Value requestContext = getServerRequestContext(requestBaton);

  // ABAP -> JS parameters

  ValuePair jsParameters = getRfmParameters(requestBaton->func_desc_handle,
                                            requestBaton->func_handle,
                                            &requestBaton->errorPath,
                                            &requestBaton->client_options);

  Napi::Value errorObj = jsParameters.first.As<Napi::Object>();
  Napi::Object abapArgs = jsParameters.second.As<Napi::Object>();

  if (!errorObj.IsUndefined()) {
    // todo
  }

  // set parameter count and names in request baton
  RfcGetParameterCount(requestBaton->func_desc_handle,
                       &requestBaton->paramCount,
                       requestBaton->errorInfo);
  requestBaton->paramNames = Napi::Persistent(abapArgs.GetPropertyNames());

  DEBUG("NodeJS function call start: ", (uintptr_t)requestBaton->func_handle);
  Napi::Value result = callback.Call({requestContext, abapArgs});
  DEBUG("NodeJS function call end: ", (uintptr_t)requestBaton->func_handle);

  // JS -> ABAP parameters

  Napi::Object params = result.ToObject();
  Napi::Array paramNames = params.GetPropertyNames();
  uint_t paramSize = paramNames.Length();

  errorObj = env.Undefined();
  for (uint_t i = 0; i < paramSize; i++) {
    Napi::String name = paramNames.Get(i).ToString();
    Napi::Value value = params.Get(name);
    // DEBUG(name, value);
    errorObj = setRfmParameter(requestBaton->func_desc_handle,
                               requestBaton->func_handle,
                               name,
                               value,
                               &requestBaton->errorPath,
                               &requestBaton->client_options);

    if (!errorObj.IsUndefined()) {
      break;
    }
  }

  requestBaton->done();
}

}  // namespace node_rfc
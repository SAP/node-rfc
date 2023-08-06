// SPDX-FileCopyrightText: 2014 SAP SE Srdjan Boskovic <srdjan.boskovic@sap.com>
//
// SPDX-License-Identifier: Apache-2.0

#include "Server.h"
#include <node_api.h>

namespace node_rfc {
extern Napi::Env __env;

uint_t Server::_id = 1;
uint_t Server::request_id = 0;

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

  // create server
  serverHandle = RfcCreateServer(server_params.connectionParams, 1, &errorInfo);
  if (errorInfo.code != RFC_OK) {
    Napi::Error::New(info.Env(), rfcSdkError(&errorInfo).ToString())
        .ThrowAsJavaScriptException();
  }
  _log(logClass::server,
       logSeverity::info,
       "created; server handle ",
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
    _log(logClass::server,
         logSeverity::error,
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
    _log(logClass::server,
         logSeverity::info,
         "Function found ",
         it->first,
         " description ",
         (pointer_t)*func_desc_handle);
  } else {
    _log(logClass::server,
         logSeverity::error,
         "Function not found ",
         it->first,
         " description ",
         (pointer_t)*func_desc_handle);
    // No access to JS and wrapString and ABAP function name
    // can't be converted to unicode here.
    // Write therefore to stdout
    printf("\nserver [error] Function not found: ");
    printfU(func_name, "\n");
  }

  return rc;
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
  RFC_RC rc = RFC_NOT_FOUND;

  // Obtain ABAP function name
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

  // Obtain TSFN object for ABAP function name
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

  // Create request "baton" for TSFN call, to pass ABAP data
  // and wait until JS handler processing completed.
  // The (Non)BlockingCall invokes the thread safe JSFunctionCall
  ServerRequestBaton* requestBaton = new ServerRequestBaton();
  auto errorPath = node_rfc::RfmErrorPath();

  requestBaton->request_connection_handle = conn_handle;
  requestBaton->client_options = it->second->server->client_options;
  requestBaton->func_handle = func_handle;
  requestBaton->func_desc_handle = it->second->func_desc_handle;
  requestBaton->errorInfo = errorInfo;
  requestBaton->errorPath = errorPath;
  requestBaton->jsFunctionName = it->second->jsFunctionName;
  requestBaton->request_id = it->second->server->get_request_id();

  _log(logClass::server,
       logSeverity::info,
       "Client request ",
       " [",
       requestBaton->request_id,
       "] ",
       " for JS function ",
       it->second->jsFunctionName,
       " as ABAP function ",
       it->first,
       " using function handle ",
       (uintptr_t)func_handle,
       ", descriptor ",
       (uintptr_t)requestBaton->func_desc_handle,
       " client connection: ",
       (uintptr_t)conn_handle);

  it->second->tsfnRequest.NonBlockingCall(requestBaton);

  // Wait here until the JavaScript handler processing done
  // and mutex released in JSFunctionCall
  requestBaton->wait();

  // In case of error
  if (requestBaton->jsHandlerError.length() > 0) {
    _log(logClass::server,
         logSeverity::error,
         "JS handler ",
         requestBaton->jsFunctionName,
         " returned error ",
         requestBaton->jsHandlerError);
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
    _log(logClass::server,
         logSeverity::info,
         "stop: ",
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

void Server::_start(RFC_ERROR_INFO* errorInfo) {
  RfcInstallGenericServerFunction(
      genericRequestHandler, metadataLookup, errorInfo);
  if (errorInfo->code != RFC_OK) {
    return;
  }
  _log(logClass::server,
       logSeverity::info,
       "start: generic request handler installed");

  RfcLaunchServer(serverHandle, errorInfo);
  if (errorInfo->code != RFC_OK) {
    return;
  }
  _log(logClass::server,
       logSeverity::info,
       "start: launched ",
       (pointer_t)serverHandle);
}

void Server::_stop() {
  if (serverHandle != nullptr) {
    _log(logClass::server,
         logSeverity::info,
         "stop: shutdown and destroy ",
         (pointer_t)serverHandle);
    RfcShutdownServer(serverHandle, 60, nullptr);
    RfcDestroyServer(serverHandle, nullptr);
    serverHandle = nullptr;
  }

  if (client_conn_handle != nullptr) {
    _log(logClass::server,
         logSeverity::info,
         "stop: close client connection ",
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
    _log(logClass::server, logSeverity::info, "stop: serve() thread join");
    st.join();
  }
}

Napi::Value Server::Stop(const Napi::CallbackInfo& info) {
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
  std::string jsFunctionName =
      jsFunction.As<Napi::Object>().Get("name").As<Napi::String>().Utf8Value();

  serverFunctions[functionName.Utf8Value()] = new ServerFunctionStruct(
      this,
      func_name,
      func_desc_handle,
      ServerRequestTsfn::New(info.Env(),
                             jsFunction,           // JavaScript server
                             "ServerRequestTsfn",  // Resource name
                             0,                    // Unlimited queue
                             1  // Only one thread will use this initially
                             ),
      // Save JS function name for server logging later on
      jsFunctionName);

  delete[] func_name;
  _log(logClass::server,
       logSeverity::info,
       "Function added ",
       jsFunctionName,
       " as ABAP function ",
       functionName.Utf8Value(),
       " description: ",
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

  if (!info[1].IsFunction()) {
    errmsg << "Server removeFunction() requires a callback function";
    Napi::TypeError::New(info.Env(), errmsg.str()).ThrowAsJavaScriptException();
    return info.Env().Undefined();
  }

  Napi::Function callback = info[1].As<Napi::Function>();

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

  _log(logClass::server,
       logSeverity::info,
       "Function removed ",
       it->second->jsFunctionName,
       " as ABAP function ",
       functionName.Utf8Value(),
       " description: ",
       (pointer_t)it->second->func_desc_handle);
  serverFunctions.erase(it);

  callback.Call({});
  return info.Env().Undefined();
};

Napi::Value Server::GetFunctionDescription(const Napi::CallbackInfo& info) {
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

using DataType = ServerRequestBaton*;

// Transform JavaScript parameters' data to ABAP
void setABAPParameters(Napi::Env env,
                       Napi::Value jsResult,
                       DataType requestBaton) {
  Napi::Value errorObj = env.Undefined();
  Napi::Object params = jsResult.As<Napi::Object>();
  Napi::Array paramNames = params.GetPropertyNames();
  uint_t paramSize = paramNames.Length();

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

  // genericRequestHandler will check for error and return result to ABAP
  requestBaton->done(errorObj.IsUndefined() ? ""
                                            : errorObj.ToString().Utf8Value());
}

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
  ValuePair jsParameters = getRfmParameters(requestBaton->func_desc_handle,
                                            requestBaton->func_handle,
                                            &requestBaton->errorPath,
                                            &requestBaton->client_options);

  Napi::Value errorObj = jsParameters.first.As<Napi::Object>();
  Napi::Object abapArgs = jsParameters.second.As<Napi::Object>();

  if (!errorObj.IsUndefined()) {
    // todo
  }

  // Save parameter count and names in request baton
  RfcGetParameterCount(requestBaton->func_desc_handle,
                       &requestBaton->paramCount,
                       requestBaton->errorInfo);
  requestBaton->paramNames = Napi::Persistent(abapArgs.GetPropertyNames());

  // Call JavaScript handler
  _log(logClass::server,
       logSeverity::info,
       "JS function call [",
       requestBaton->request_id,
       "] start ",
       requestBaton->jsFunctionName,
       " with ABAP function handle ",
       (uintptr_t)requestBaton->func_handle,
       " start");

  Napi::Value jsResult;
  try {
    jsResult = callback.Call({requestContext, abapArgs});
  } catch (const Error& e) {
    requestBaton->done(e.Message());
    return;
  }

  _log(logClass::server,
       logSeverity::info,
       "JS function call [",
       requestBaton->request_id,
       "] end ",
       requestBaton->jsFunctionName,
       " with ABAP function handle ",
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
                               Napi::Object result = info[0].As<Napi::Object>();
                               setABAPParameters(env, result, requestBaton);
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
    setABAPParameters(env, jsResult, requestBaton);
  }
}

}  // namespace node_rfc
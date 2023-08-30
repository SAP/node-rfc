// SPDX-FileCopyrightText: 2014 SAP SE Srdjan Boskovic <srdjan.boskovic@sap.com>
//
// SPDX-License-Identifier: Apache-2.0

#include "Client.h"
#include <mutex>
#include <thread>
#include <tuple>
#include "Pool.h"

namespace node_rfc {

uint_t Client::_id = 1;

ErrorPair connectionCheckErrorInit() {
  RFC_ERROR_INFO errorInfo;
  errorInfo.code = RFC_OK;
  return ErrorPair(errorInfo, "");
}

Napi::Value Client::getOperationError(bool conn_closed,
                                      const char* operation,
                                      ErrorPair connectionCheckError,
                                      RFC_ERROR_INFO* errorInfo,
                                      Napi::Env env) {
  Napi::EscapableHandleScope scope(env);
  Napi::Value error = Env().Undefined();
  if (conn_closed) {
    error = connectionClosedError(operation);
  } else if (connectionCheckError.first.code != RFC_OK) {
    error = rfcSdkError(&connectionCheckError.first);
  } else if (connectionCheckError.second.length() > 0) {
    error = nodeRfcError(connectionCheckError.second);
  } else if (errorInfo->code != RFC_OK) {
    error = rfcSdkError(errorInfo);
  }
  return scope.Escape(error);
}

Napi::Object Client::Init(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);

  Napi::Function func = DefineClass(
      env,
      "Client",
      {
          InstanceAccessor("_id", &Client::IdGetter, nullptr),
          InstanceAccessor("_alive", &Client::AliveGetter, nullptr),
          InstanceAccessor(
              "_connectionHandle", &Client::ConnectionHandleGetter, nullptr),
          InstanceAccessor("_pool_id", &Client::PoolIdGetter, nullptr),
          InstanceAccessor("_config", &Client::ConfigGetter, nullptr),
          // InstanceMethod("setIniPath", &Client::SetIniPath),
          InstanceMethod("connectionInfo", &Client::ConnectionInfo),
          InstanceMethod("open", &Client::Open),
          InstanceMethod("close", &Client::Close),
          InstanceMethod("cancel", &Client::Cancel),
          InstanceMethod("release", &Client::Release),
          InstanceMethod("resetServerContext", &Client::ResetServerContext),
          InstanceMethod("ping", &Client::Ping),
          InstanceMethod("invoke", &Client::Invoke),
      });

  Napi::FunctionReference* constructor = new Napi::FunctionReference();
  *constructor = Napi::Persistent(func);
  constructor->SuppressDestruct();
  env.SetInstanceData(constructor);

  // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks)
  exports.Set("Client", func);
  return exports;
}

Napi::Value Client::IdGetter(const Napi::CallbackInfo& info) {
  return Napi::Number::New(info.Env(), id);
}

Napi::Value Client::AliveGetter(const Napi::CallbackInfo& info) {
  return Napi::Boolean::New(info.Env(), connectionHandle != nullptr);
}

Napi::Value Client::ConnectionHandleGetter(const Napi::CallbackInfo& info) {
  return Napi::Number::New(info.Env(), (uintptr_t)this->connectionHandle);
}

Napi::Value Client::ConfigGetter(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::EscapableHandleScope scope(env);
  Napi::Object config = Napi::Object::New(env);
  if (pool != nullptr) {
    if (!pool->connectionParameters.IsEmpty()) {
      config.Set(POOL_KEY_CONNECTION_PARAMS,
                 pool->connectionParameters.Value());
    }
    config.Set(POOL_KEY_CLIENT_OPTIONS,
               pool->client_options._Value(info.Env()));
  } else {
    if (!clientParamsRef.IsEmpty()) {
      config.Set(POOL_KEY_CONNECTION_PARAMS, clientParamsRef.Value());
    }
    config.Set(POOL_KEY_CLIENT_OPTIONS, client_options._Value(info.Env()));
  }

  return scope.Escape(config);
}

Napi::Value Client::PoolIdGetter(const Napi::CallbackInfo& info) {
  if (pool == nullptr) {
    return Number::New(info.Env(), 0);
  }
  return Napi::Number::New(info.Env(), pool->id);
}

Napi::Value Client::ConnectionInfo(const Napi::CallbackInfo& info) {
  if (connectionHandle == nullptr) {
    return connectionClosedError("connectionInfo");
  }

  return getConnectionAttributes(info.Env(), connectionHandle);
}

Client::Client(const Napi::CallbackInfo& info)
    : Napi::ObjectWrap<Client>(info) {
  init();

  if (!info[0].IsUndefined() && (info[0].IsFunction() || !info[0].IsObject())) {
    Napi::TypeError::New(Env(), "Client connection parameters missing")
        .ThrowAsJavaScriptException();
    return;
  }

  if (info.Length() > 0) {
    clientParamsRef = Napi::Persistent(info[0].As<Napi::Object>());
    getConnectionParams(clientParamsRef.Value(), &client_params);
  }

  if (!info[1].IsUndefined()) {
    clientOptionsRef = Napi::Persistent(info[1].As<Napi::Object>());
    checkClientOptions(clientOptionsRef.Value(), &client_options);
  }

  if (info.Length() > 2) {
    char errmsg[ERRMSG_LENGTH];
    snprintf(errmsg,
             ERRMSG_LENGTH - 1,
             "Client constructor requires max. two arguments, received %zu",
             info.Length());
    Napi::TypeError::New(node_rfc::__env, errmsg).ThrowAsJavaScriptException();
  }

  _log.info(logClass::client, "Client created: ", id);
};

Client::~Client(void) {
  if (pool == nullptr) {
    // Close own connection
    if (connectionHandle != nullptr) {
      RFC_ERROR_INFO errorInfo;
      _log.info(logClass::client,
                log_id() + " closing connection ",
                (pointer_t)connectionHandle);
      RFC_RC rc = RfcCloseConnection(connectionHandle, &errorInfo);
      if (rc != RFC_OK) {
        _log.warning(logClass::client,
                     log_id() + " error closing direct connection handle ",
                     (pointer_t)(connectionHandle));
      }
    } else {
      _log.warning(logClass::client,
                   log_id() + " connection ",
                   (uintptr_t)connectionHandle,
                   " already closed");
    }

    // Unref client config
    if (!clientParamsRef.IsEmpty()) {
      clientParamsRef.Reset();
    }
    if (!clientOptionsRef.IsEmpty()) {
      clientOptionsRef.Reset();
    }
  } else {
    if (connectionHandle != nullptr) {
      pool->releaseClient(connectionHandle);
    }
  }
}

Napi::Value Client::connectionClosedError(const char* suffix) {
  return nodeRfcError("RFM client request over closed connection: " +
                      std::string(suffix));
}

Napi::Object Client::NewInstance(Napi::Env env) {
  Napi::EscapableHandleScope scope(env);
  Napi::Object obj = env.GetInstanceData<Napi::FunctionReference>()->New({});
  return scope.Escape(napi_value(obj)).ToObject();
}

class OpenAsync : public Napi::AsyncWorker {
 public:
  OpenAsync(Napi::Function& callback, Client* client)
      : Napi::AsyncWorker(callback), client(client) {}
  ~OpenAsync() {}

  // cppcheck-suppress unusedFunction
  void Execute() {
    client->LockMutex();
    client->connectionHandle =
        RfcOpenConnection(client->client_params.connectionParams,
                          client->client_params.paramSize,
                          &errorInfo);
    if (errorInfo.code != RFC_OK) {
      client->connectionHandle = nullptr;
    }
    client->UnlockMutex();
  }

  // cppcheck-suppress unusedFunction
  void OnOK() {
    if (errorInfo.code != RFC_OK) {
      Callback().Call({rfcSdkError(&errorInfo)});
    } else {
      Callback().Call({});
    }
    Callback().Reset();
  }

 private:
  Client* client;
  RFC_ERROR_INFO errorInfo;
};

class CloseAsync : public Napi::AsyncWorker {
 public:
  CloseAsync(Napi::Function& callback, Client* client)
      : Napi::AsyncWorker(callback), client(client) {}
  ~CloseAsync() {}

  void Execute() {
    client->LockMutex();
    conn_closed = (client->connectionHandle == nullptr);
    if (!conn_closed) {
      RfcCloseConnection(client->connectionHandle, &errorInfo);
      client->connectionHandle = nullptr;
    }
    client->UnlockMutex();
  }

  void OnOK() {
    Napi::HandleScope scope(Env());

    if (conn_closed) {
      Callback().Call({client->connectionClosedError("close()")});
    } else if (errorInfo.code != RFC_OK) {
      Callback().Call({rfcSdkError(&errorInfo)});
    } else {
      Callback().Call({});
    }
    Callback().Reset();
  }

 private:
  Client* client;
  RFC_ERROR_INFO errorInfo;
  bool conn_closed = false;
};

class ResetServerAsync : public Napi::AsyncWorker {
 public:
  ResetServerAsync(Napi::Function& callback, Client* client)
      : Napi::AsyncWorker(callback), client(client) {}
  ~ResetServerAsync() {}

  void Execute() {
    client->LockMutex();
    conn_closed = (client->connectionHandle == nullptr);
    if (!conn_closed) {
      RfcResetServerContext(client->connectionHandle, &errorInfo);
      if (errorInfo.code != RFC_OK) {
        connectionCheckError = client->connectionCheck(&errorInfo);
      }
    }
    client->UnlockMutex();
  }

  void OnOK() {
    Napi::HandleScope scope(Env());
    Napi::Value error = client->getOperationError(conn_closed,
                                                  "resetServerContext()",
                                                  connectionCheckError,
                                                  &errorInfo,
                                                  Env());
    Callback().Call({error});
    Callback().Reset();
  }

 private:
  Client* client;
  RFC_ERROR_INFO errorInfo;
  bool conn_closed = false;
  ErrorPair connectionCheckError = connectionCheckErrorInit();
};

class PingAsync : public Napi::AsyncWorker {
 public:
  PingAsync(Napi::Function& callback, Client* client)
      : Napi::AsyncWorker(callback), client(client) {}
  ~PingAsync() {}

  void Execute() {
    client->LockMutex();
    conn_closed = (client->connectionHandle == nullptr);
    if (!conn_closed) {
      RfcPing(client->connectionHandle, &errorInfo);
      if (errorInfo.code != RFC_OK) {
        connectionCheckError = client->connectionCheck(&errorInfo);
      }
    }
    client->UnlockMutex();
  }

  void OnOK() {
    Napi::HandleScope scope(Env());
    Napi::Value error = client->getOperationError(
        conn_closed, "ping()", connectionCheckError, &errorInfo, Env());
    Callback().Call({error, Napi::Boolean::New(Env(), error.IsUndefined())});
    Callback().Reset();
  }

 private:
  Client* client;
  bool conn_closed = false;
  RFC_ERROR_INFO errorInfo;
  ErrorPair connectionCheckError = connectionCheckErrorInit();
};

class InvokeAsync : public Napi::AsyncWorker {
 public:
  InvokeAsync(Napi::Function& callback,
              Client* client,
              RFC_FUNCTION_HANDLE functionHandle,
              RFC_FUNCTION_DESC_HANDLE functionDescHandle)
      : Napi::AsyncWorker(callback),
        client(client),
        functionHandle(functionHandle),
        functionDescHandle(functionDescHandle) {}
  ~InvokeAsync() {}

  void Execute() {
    client->LockMutex();
    conn_closed = (client->connectionHandle == nullptr);
    if (!conn_closed) {
      RfcInvoke(client->connectionHandle, functionHandle, &errorInfo);
      if (errorInfo.code != RFC_OK) {
        connectionCheckError = client->connectionCheck(&errorInfo);
      }
    }
  }

  void OnOK() {
    Napi::HandleScope scope(Env());

    std::string closed_errmsg =
        "invoke() " + wrapString(client->errorPath.functionName)
                          .As<Napi::String>()
                          .Utf8Value();
    ValuePair result =
        ValuePair(client->getOperationError(conn_closed,
                                            closed_errmsg.c_str(),
                                            connectionCheckError,
                                            &errorInfo,
                                            Env()),
                  Env().Undefined());

    if (result.first.IsUndefined()) {
      result = getRfmParameters(functionDescHandle,
                                functionHandle,
                                &client->errorPath,
                                &client->client_options);
    }

    RfcDestroyFunction(functionHandle, nullptr);
    client->UnlockMutex();

    Callback().Call({result.first, result.second});
    Callback().Reset();
  }

 private:
  Client* client;
  RFC_FUNCTION_HANDLE functionHandle;
  RFC_FUNCTION_DESC_HANDLE functionDescHandle;
  RFC_ERROR_INFO errorInfo;
  bool conn_closed = false;
  ErrorPair connectionCheckError = connectionCheckErrorInit();
};

class PrepareAsync : public Napi::AsyncWorker {
 public:
  PrepareAsync(Napi::Function& callback,
               Client* client,
               Napi::String rfmName,
               Napi::Array& notRequestedParameters,
               Napi::Object& rfmParams)
      : Napi::AsyncWorker(callback),
        client(client),
        notRequested(Napi::Persistent(notRequestedParameters)),
        rfmParams(Napi::Persistent(rfmParams)) {
    funcName = setString(rfmName);
    client->errorPath.setFunctionName(funcName);
  }
  ~PrepareAsync() { delete[] funcName; }

  void Execute() {
    client->LockMutex();
    conn_closed = (client->connectionHandle == nullptr);
    if (!conn_closed) {
      functionDescHandle =
          RfcGetFunctionDesc(client->connectionHandle, funcName, &errorInfo);
    }
  }

  void OnOK() {
    client->UnlockMutex();
    RFC_FUNCTION_HANDLE functionHandle = nullptr;
    Napi::Value argv[2] = {Env().Undefined(), Env().Undefined()};

    if (conn_closed) {
      std::string errmsg =
          "invoke() " + wrapString(client->errorPath.functionName)
                            .As<Napi::String>()
                            .Utf8Value();
      argv[0] = client->connectionClosedError(errmsg.c_str());
    } else if (functionDescHandle == nullptr || errorInfo.code != RFC_OK) {
      argv[0] = rfcSdkError(&errorInfo);
    } else {
      // function descriptor handle created, proceed with function handle
      functionHandle = RfcCreateFunction(functionDescHandle, &errorInfo);

      if (errorInfo.code != RFC_OK) {
        argv[0] = rfcSdkError(&errorInfo);
      } else {
        for (uint_t i = 0; i < notRequested.Value().Length(); i++) {
          Napi::String name = notRequested.Value().Get(i).ToString();
          SAP_UC* paramName = setString(name);
          RFC_RC rc =
              RfcSetParameterActive(functionHandle, paramName, 0, &errorInfo);
          delete[] paramName;
          if (rc != RFC_OK) {
            argv[0] = rfcSdkError(&errorInfo);
            break;
          }
        }
      }
    }

    notRequested.Reset();

    if (argv[0].IsUndefined()) {
      Napi::Object params = rfmParams.Value();
      Napi::Array paramNames = params.GetPropertyNames();
      uint_t paramSize = paramNames.Length();

      for (uint_t i = 0; i < paramSize; i++) {
        Napi::String name = paramNames.Get(i).ToString();
        Napi::Value value = params.Get(name);
        argv[0] = setRfmParameter(functionDescHandle,
                                  functionHandle,
                                  name,
                                  value,
                                  &client->errorPath,
                                  &client->client_options);

        if (!argv[0].IsUndefined()) {
          break;
        }
      }
    }

    rfmParams.Reset();

    if (argv[0].IsUndefined()) {
      Napi::Function callbackFunction = Callback().Value().As<Napi::Function>();
      (new InvokeAsync(
           callbackFunction, client, functionHandle, functionDescHandle))
          ->Queue();
    } else {
      Callback().Call({argv[0], argv[1]});
      Callback().Reset();
    }
  }

 private:
  Client* client;
  SAP_UC* funcName;

  Napi::Reference<Napi::Array> notRequested;
  Napi::Reference<Napi::Object> rfmParams;

  RFC_FUNCTION_DESC_HANDLE functionDescHandle;
  RFC_ERROR_INFO errorInfo;
  bool conn_closed = false;
};

ErrorPair Client::connectionCheck(RFC_ERROR_INFO* errorInfo) {
  RFC_ERROR_INFO errorInfoOpen;

  errorInfoOpen.code = RFC_OK;

  if (
      // error code check
      errorInfo->code == RFC_COMMUNICATION_FAILURE ||  // Error in Network &
                                                       // Communication layer.
      errorInfo->code ==
          RFC_ABAP_RUNTIME_FAILURE ||  // SAP system runtime error
                                       // (SYSTEM_FAILURE): Shortdump on the
                                       // backend side.
      errorInfo->code == RFC_ABAP_MESSAGE ||  // The called function module
                                              // raised an E-, A- or X-Message.
      errorInfo->code ==
          RFC_EXTERNAL_FAILURE ||  // Problems in the RFC runtime of the
                                   // external program (i.e "this" library)
      // error group check, for even more robustness here
      errorInfo->group ==
          ABAP_RUNTIME_FAILURE ||  // ABAP Message raised in ABAP function
                                   // modules or in ABAP runtime of the backend
                                   // (e.g Kernel)
      errorInfo->group ==
          LOGON_FAILURE ||  // Error message raised when logon fails
      errorInfo->group ==
          COMMUNICATION_FAILURE ||  // Problems with the network connection (or
                                    // backend broke down and killed the
                                    // connection)
      errorInfo->group ==
          EXTERNAL_RUNTIME_FAILURE  // Problems in the RFC runtime of the
                                    // external program (i.e "this" library)
      )                             // closed
  {
    if (errorInfo->code == RFC_CANCELED) {
      _log.info(logClass::client,
                "Connection cancelled ",
                (pointer_t)this->connectionHandle);
    }

    RFC_CONNECTION_HANDLE new_handle;
    RFC_CONNECTION_HANDLE old_handle = this->connectionHandle;
    this->connectionHandle = nullptr;
    if (pool == nullptr) {
      new_handle = RfcOpenConnection(client_params.connectionParams,
                                     client_params.paramSize,
                                     &errorInfoOpen);
    } else {
      new_handle = RfcOpenConnection(pool->client_params.connectionParams,
                                     pool->client_params.paramSize,
                                     &errorInfoOpen);
    }
    if (errorInfoOpen.code != RFC_OK) {
      // error getting a new handle
      return ErrorPair(errorInfoOpen, "");
    }

    if (pool != nullptr) {
      std::string updateError =
          pool->updateLeasedHandle(old_handle, new_handle);
      if (updateError.length() > 0) {
        // pool update failed
        return ErrorPair(errorInfoOpen, updateError);
      }

      _log.info(logClass::pool,
                "Connection closed ",
                (uintptr_t)old_handle,
                " replaced with ",
                (uintptr_t)new_handle);
      this->connectionHandle = new_handle;
    } else {
      // assign new handle to direct client
      this->connectionHandle = new_handle;
    }
    _log.error(logClass::client,
               "Critical ABAP error: group ",
               errorInfo->group,
               " code ",
               errorInfo->code,
               " Closed connection ",
               (pointer_t)old_handle,
               " replaced with ",
               (pointer_t)new_handle);
  } else {
    _log.error(logClass::client,
               "ABAP error: group ",
               errorInfo->group,
               " code ",
               errorInfo->code,
               " for connection ",
               (pointer_t)this->connectionHandle);
  }

  return ErrorPair(errorInfoOpen, "");
}

Napi::Value Client::Release(const Napi::CallbackInfo& info) {
  if (!info[1].IsFunction()) {
    Napi::TypeError::New(info.Env(),
                         "Client release() requires a callback function")
        .ThrowAsJavaScriptException();
    return info.Env().Undefined();
  }

  Napi::Function callback = info[1].As<Napi::Function>();

  if (pool == nullptr) {
    callback.Call({nodeRfcError("Client release() method is for managed "
                                "clients only, use \"close()\" instead")});
    return info.Env().Undefined();
  }

  // the rest of arguments check done in Pool::Release
  pool->Release(info);

  return info.Env().Undefined();
}

void cancelConnection(RFC_RC* rc,
                      RFC_CONNECTION_HANDLE connectionHandle,
                      RFC_ERROR_INFO* errorInfo) {
  _log.info(
      logClass::client, "Connection cancelled ", (pointer_t)connectionHandle);
  *rc = RfcCancel(connectionHandle, errorInfo);
}

Napi::Value Client::Cancel(const Napi::CallbackInfo& info) {
  if (!info[0].IsFunction()) {
    Napi::TypeError::New(info.Env(),
                         "Client cancel() requires a callback function")
        .ThrowAsJavaScriptException();
    return info.Env().Undefined();
  }

  Napi::Function callback = info[0].As<Napi::Function>();

  RFC_RC rc = RFC_OK;
  RFC_ERROR_INFO errorInfo;

  std::thread tcancel(cancelConnection, &rc, connectionHandle, &errorInfo);
  tcancel.join();

  if (rc == RFC_OK && errorInfo.code == RFC_OK) {
    callback.Call({});
  } else {
    callback.Call({rfcSdkError(&errorInfo)});
  }
  return info.Env().Undefined();
}

Napi::Value Client::Open(const Napi::CallbackInfo& info) {
  if (!info[0].IsFunction()) {
    Napi::TypeError::New(info.Env(),
                         "Client open() requires a callback function")
        .ThrowAsJavaScriptException();
    return info.Env().Undefined();
  }

  Napi::Function callback = info[0].As<Napi::Function>();

  if (pool != nullptr) {
    callback.Call({nodeRfcError("Client \"open()\" not allowed for managed "
                                "clients, , use \"acquire()\" instead")});
    return info.Env().Undefined();
  }

  (new OpenAsync(callback, this))->Queue();

  return info.Env().Undefined();
}

Napi::Value Client::Close(const Napi::CallbackInfo& info) {
  if (!info[0].IsFunction()) {
    Napi::TypeError::New(info.Env(),
                         "Client close() requires a callback function")
        .ThrowAsJavaScriptException();
    return info.Env().Undefined();
  }

  Napi::Function callback = info[0].As<Napi::Function>();

  if (pool != nullptr) {
    // Managed connection error

    callback.Call(
        {nodeRfcError("Client \"close()\" method not allowed for managed "
                      "clients, use the \"release()\" instead")});
    return info.Env().Undefined();
  }

  (new CloseAsync(callback, this))->Queue();

  return info.Env().Undefined();
}

Napi::Value Client::ResetServerContext(const Napi::CallbackInfo& info) {
  if (!info[0].IsFunction()) {
    Napi::TypeError::New(
        info.Env(), "Client resetServerContext() requires a callback function")
        .ThrowAsJavaScriptException();
    return info.Env().Undefined();
  }
  Napi::Function callback = info[0].As<Napi::Function>();

  (new ResetServerAsync(callback, this))->Queue();

  return info.Env().Undefined();
}

Napi::Value Client::Ping(const Napi::CallbackInfo& info) {
  if (!info[0].IsFunction()) {
    Napi::TypeError::New(info.Env(),
                         "Client Ping() requires a callback function")
        .ThrowAsJavaScriptException();
    return info.Env().Undefined();
  }

  Napi::Function callback = info[0].As<Napi::Function>();

  (new PingAsync(callback, this))->Queue();

  return info.Env().Undefined();
}

Napi::Value Client::Invoke(const Napi::CallbackInfo& info) {
  Napi::Array notRequested = Napi::Array::New(info.Env());
  Napi::Value bcd;

  if (!info[2].IsFunction()) {
    Napi::TypeError::New(info.Env(),
                         "Client invoke() requires a callback function")
        .ThrowAsJavaScriptException();
    return info.Env().Undefined();
  }

  Napi::Function callback = info[2].As<Napi::Function>();

  if (info[3].IsObject()) {
    Napi::Object options = info[3].ToObject();
    Napi::Array props = options.GetPropertyNames();
    for (uint_t i = 0; i < props.Length(); i++) {
      Napi::String key = props.Get(i).ToString();
      if (key.Utf8Value().compare(std::string(CALL_OPTION_KEY_NOTREQUESTED)) ==
          (int)0) {
        notRequested = options.Get(key).As<Napi::Array>();
      } else if (key.Utf8Value().compare(
                     std::string(CALL_OPTION_KEY_TIMEOUT)) == (int)0) {
        // timeout = options.Get(key).As<Napi::Array>();
      } else {
        char err[ERRMSG_LENGTH];
        std::string optionName = key.Utf8Value();
        snprintf(err, ERRMSG_LENGTH - 1, "Unknown option: %s", &optionName[0]);
        Napi::TypeError::New(node_rfc::__env, err).ThrowAsJavaScriptException();
      }
    }
  }

  Napi::String rfmName = info[0].As<Napi::String>();
  Napi::Object rfmParams = info[1].As<Napi::Object>();

  (new PrepareAsync(callback, this, rfmName, notRequested, rfmParams))->Queue();

  return info.Env().Undefined();
}

void Client::LockMutex() {
  invocationMutex.lock();
}

void Client::UnlockMutex() {
  invocationMutex.unlock();
}

}  // namespace node_rfc

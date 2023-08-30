// SPDX-FileCopyrightText: 2014 SAP SE Srdjan Boskovic <srdjan.boskovic@sap.com>
//
// SPDX-License-Identifier: Apache-2.0

#ifndef NodeRfc_Client_H
#define NodeRfc_Client_H

#include "Log.h"
#include "nwrfcsdk.h"

namespace node_rfc {

extern Napi::Env __env;
extern Log _log;

class Pool;

class Client : public Napi::ObjectWrap<Client> {
 public:
  friend class Pool;
  friend class AcquireAsync;
  friend class ReleaseAsync;
  friend class OpenAsync;
  friend class CheckPoolAsync;
  friend class CloseAsync;
  friend class ResetServerAsync;
  friend class PingAsync;
  friend class PrepareAsync;
  friend class InvokeAsync;
  static Napi::Object Init(Napi::Env env, Napi::Object exports);
  // cppcheck-suppress noExplicitConstructor
  Client(const Napi::CallbackInfo& info);
  ~Client(void);
  const std::string log_id() const { return "Client " + std::to_string(id); }

 private:
  static Napi::Object NewInstance(Napi::Env env);
  Napi::Value IdGetter(const Napi::CallbackInfo& info);
  Napi::Value AliveGetter(const Napi::CallbackInfo& info);
  Napi::Value ConfigGetter(const Napi::CallbackInfo& info);
  Napi::Value ConnectionHandleGetter(const Napi::CallbackInfo& info);
  Napi::Value PoolIdGetter(const Napi::CallbackInfo& info);
  Napi::ObjectReference clientParamsRef;
  Napi::ObjectReference clientOptionsRef;

  Napi::Value connectionClosedError(const char* suffix);
  ErrorPair connectionCheck(RFC_ERROR_INFO* errorInfo);
  Napi::Value getOperationError(bool conn_closed,
                                const char* operation,
                                ErrorPair connectionCheckError,
                                RFC_ERROR_INFO* errorInfo,
                                Napi::Env env);

  Napi::Value ConnectionInfo(const Napi::CallbackInfo& info);
  Napi::Value Release(const Napi::CallbackInfo& info);
  Napi::Value Open(const Napi::CallbackInfo& info);
  Napi::Value Close(const Napi::CallbackInfo& info);
  Napi::Value Cancel(const Napi::CallbackInfo& info);
  Napi::Value ResetServerContext(const Napi::CallbackInfo& info);
  Napi::Value Ping(const Napi::CallbackInfo& info);
  Napi::Value Invoke(const Napi::CallbackInfo& info);

  RfmErrorPath errorPath;

  void init() {
    id = Client::_id++;

    pool = nullptr;
    connectionHandle = nullptr;
  };

  static uint_t _id;
  std::mutex invocationMutex;

  uint_t id;
  Pool* pool;
  RFC_CONNECTION_HANDLE connectionHandle;

  ConnectionParamsStruct client_params = ConnectionParamsStruct(0, nullptr);
  ClientOptionsStruct client_options;

  void LockMutex();
  void UnlockMutex();
};

}  // namespace node_rfc

#endif
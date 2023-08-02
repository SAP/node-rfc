// SPDX-FileCopyrightText: 2014 SAP SE Srdjan Boskovic <srdjan.boskovic@sap.com>
//
// SPDX-License-Identifier: Apache-2.0

#ifndef NodeRfc_Server_H
#define NodeRfc_Server_H

#include <condition_variable>
#include <map>
#include <thread>
#include "Client.h"

struct ServerRequestBaton {
  RFC_CONNECTION_HANDLE request_connection_handle = nullptr;
  RFC_FUNCTION_DESC_HANDLE func_desc_handle;
  RFC_FUNCTION_HANDLE func_handle;
  RFC_ERROR_INFO* errorInfo;

  node_rfc::RfmErrorPath errorPath;
  node_rfc::ClientOptionsStruct client_options;
  uint_t paramCount;
  Napi::Reference<Napi::Array> paramNames;

  bool server_call_completed = false;
  std::mutex server_call_mutex;
  std::unique_lock<std::mutex> lock;
  std::condition_variable server_call_condition;

  void wait() {
    std::thread::id this_tid = std::this_thread::get_id();
    DEBUG("requestBaton wait ", this_tid);
    std::unique_lock<std::mutex> lock(server_call_mutex);
    server_call_condition.wait(lock, [this] { return server_call_completed; });
    DEBUG("requestBaton done ", this_tid, " status ", server_call_completed);
  }

  void done() {
    std::thread::id this_tid = std::this_thread::get_id();
    server_call_completed = true;
    server_call_condition.notify_one();
    DEBUG("requestBaton done requested ",
          this_tid,
          " status ",
          server_call_completed);
  }
};

void ServerCallJs(Napi::Env env,
                  Napi::Function callback,
                  std::nullptr_t* context,
                  ServerRequestBaton* data);

using ServerRequestTsfn = Napi::
    TypedThreadSafeFunction<std::nullptr_t, ServerRequestBaton, ServerCallJs>;

// using ServerRequestTsfn = Napi::ThreadSafeFunction;  // <std::nullptr_t,
//                                                      // ServerRequestBaton,
//                                                      // ServerCallJs>;

typedef struct _ServerFunctionStruct {
  RFC_ABAP_NAME func_name;
  RFC_FUNCTION_DESC_HANDLE func_desc_handle = nullptr;
  ServerRequestTsfn tsfnRequest;

  _ServerFunctionStruct() { func_name[0] = 0; }

  _ServerFunctionStruct(RFC_ABAP_NAME _func_name,
                        RFC_FUNCTION_DESC_HANDLE _func_desc_handle,
                        ServerRequestTsfn _tsfnRequest) {
    strcpyU(func_name, _func_name);
    func_desc_handle = _func_desc_handle;
    tsfnRequest = _tsfnRequest;
  }

  _ServerFunctionStruct& operator=(
      _ServerFunctionStruct& src)  // note: passed by copy
  {
    strcpyU(func_name, src.func_name);
    func_desc_handle = src.func_desc_handle;
    tsfnRequest = src.tsfnRequest;
    return *this;
  }

  ~_ServerFunctionStruct() {
    DEBUG("\n~_ServerFunctionStruct\n");
    tsfnRequest.Release();
  }
} ServerFunctionStruct;

typedef std::map<std::string, ServerFunctionStruct*> ServerFunctionsMap;

namespace node_rfc {
extern Napi::Env __env;

RFC_RC SAP_API metadataLookup(SAP_UC const* func_name,
                              RFC_ATTRIBUTES rfc_attributes,
                              RFC_FUNCTION_DESC_HANDLE* func_handle);
RFC_RC SAP_API genericRequestHandler(RFC_CONNECTION_HANDLE conn_handle,
                                     RFC_FUNCTION_HANDLE func_handle,
                                     RFC_ERROR_INFO* errorInfo);

class Server : public Napi::ObjectWrap<Server> {
 public:
  friend class StartAsync;
  friend class StopAsync;
  friend class GetFunctionDescAsync;
  static Napi::Object Init(Napi::Env env, Napi::Object exports);
  // cppcheck-suppress noExplicitConstructor
  Server(const Napi::CallbackInfo& info);
  ~Server(void);
  ServerFunctionsMap serverFunctions;
  // AddonData* addon_data;

  ClientOptionsStruct client_options;

 private:
  void _stop();
  void _start(RFC_ERROR_INFO* errorInfo);
  std::thread server_thread(RFC_ERROR_INFO* errorInfo) {
    return std::thread([=] { _start(errorInfo); });
  };

  std::thread st;

  Napi::Value IdGetter(const Napi::CallbackInfo& info);
  Napi::Value AliveGetter(const Napi::CallbackInfo& info);
  Napi::Value ServerConnectionHandleGetter(const Napi::CallbackInfo& info);
  Napi::Value ClientConnectionHandleGetter(const Napi::CallbackInfo& info);

  Napi::Value Start(const Napi::CallbackInfo& info);
  Napi::Value Stop(const Napi::CallbackInfo& info);
  Napi::Value AddFunction(const Napi::CallbackInfo& info);
  Napi::Value RemoveFunction(const Napi::CallbackInfo& info);
  Napi::Value GetFunctionDescription(const Napi::CallbackInfo& info);

  RFC_CONNECTION_HANDLE server_conn_handle;
  RFC_CONNECTION_HANDLE client_conn_handle;
  RFC_SERVER_HANDLE serverHandle;
  ConnectionParamsStruct server_params;
  ConnectionParamsStruct client_params;
  Napi::ObjectReference serverParamsRef;
  Napi::ObjectReference clientParamsRef;
  Napi::ObjectReference clientOptionsRef;

  void init() {
    id = Server::_id++;

    server_conn_handle = nullptr;
    client_conn_handle = nullptr;
    serverHandle = nullptr;

    // uv_sem_init(&invocationMutex, 1);

    // addon_data = new AddonData;
    // addon_data->work = nullptr;
  };

  static uint_t _id;
  uint_t id;

  void LockMutex();
  void UnlockMutex();
  // uv_sem_t invocationMutex;
};

}  // namespace node_rfc

#endif

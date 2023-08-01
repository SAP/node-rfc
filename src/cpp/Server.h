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
    DEBUG("[ServerRequestBaton wait ", this_tid, "\n");
    std::unique_lock<std::mutex> lock(server_call_mutex);
    server_call_condition.wait(lock, [this] { return server_call_completed; });
    DEBUG("[ServerRequestBaton completed", server_call_completed, "\n");
  }

  void done() {
    std::thread::id this_tid = std::this_thread::get_id();
    DEBUG("[ServerRequestBaton unblock ", this_tid, "\n");
    server_call_completed = true;
    server_call_condition.notify_one();
    DEBUG("[ServerRequestBaton unblocked ", server_call_completed, "\n");
  }
};

void ServerCallJs(Napi::Env env,
                  Napi::Function callback,
                  std::nullptr_t* context,
                  ServerRequestBaton* data);  // handles calling the JS callback
void ServerDoneCallback(const CallbackInfo& info);

using ServerRequestTsfn = Napi::
    TypedThreadSafeFunction<std::nullptr_t, ServerRequestBaton, ServerCallJs>;

typedef struct _ServerFunctionStruct {
  RFC_ABAP_NAME func_name;
  RFC_FUNCTION_DESC_HANDLE func_desc_handle = NULL;
  ServerRequestTsfn tsfnRequest;

  _ServerFunctionStruct() { func_name[0] = 0; }

  _ServerFunctionStruct(RFC_ABAP_NAME name,
                        RFC_FUNCTION_DESC_HANDLE desc_handle,
                        ServerRequestTsfn tsFunction) {
    strcpyU(func_name, name);
    func_desc_handle = desc_handle;
    tsfnRequest = tsFunction;
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
  friend class GetFunctionDescAsync;
  static Napi::Object Init(Napi::Env env, Napi::Object exports);
  // cppcheck-suppress noExplicitConstructor
  Server(const Napi::CallbackInfo& info);
  ~Server(void);
  ServerFunctionsMap serverFunctions;
  // AddonData* addon_data;

  ClientOptionsStruct client_options;

 private:
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

    server_conn_handle = NULL;
    client_conn_handle = NULL;
    serverHandle = NULL;

    // uv_sem_init(&invocationMutex, 1);

    // addon_data = new AddonData;
    // addon_data->work = NULL;
  };

  static uint_t _id;
  uint_t id;

  void LockMutex();
  void UnlockMutex();
  // uv_sem_t invocationMutex;
};

}  // namespace node_rfc

#endif

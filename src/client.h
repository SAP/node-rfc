// Copyright 2014 SAP AG.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http: //www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific
// language governing permissions and limitations under the License.

#ifndef NODE_SAPNWRFC_CLIENT_H_
#define NODE_SAPNWRFC_CLIENT_H_

#define SAPNWRFC_VERSION "0.0.1"

#include <uv.h>
#include <napi.h>
#include <sapnwrfc.h>

using namespace Napi;

namespace node_rfc
{
class Client : public Napi::ObjectWrap<Client>
{
public:
  friend class ConnectAsync;
  friend class InvokeAsync;

  static Napi::FunctionReference constructor;
  static Napi::Object Init(Napi::Env env, Napi::Object exports);

  //bool IsAlive() { return alive; };

  void init()
  {
    paramSize = 0;
    connectionParams = NULL;
    connectionHandle = NULL;
    alive = false;

    rc = (RFC_RC)0;
    errorInfo.code = rc;
  };

  Client(const Napi::CallbackInfo &info);
  ~Client(void);

private:
  static unsigned int __refCounter;
  unsigned int __refId;

  Napi::Value IdGetter(const Napi::CallbackInfo &info);

  static Napi::Value GetVersion(const Napi::CallbackInfo &info);
  Napi::Value ConnectionInfo(const Napi::CallbackInfo &info);

  Napi::Value Connect(const Napi::CallbackInfo &info);
  static void Work_Connect(uv_work_t *req);
  static void Work_AfterConnect(uv_work_t *req, int status);

  Napi::Value Invoke(const Napi::CallbackInfo &info);
  static void Work_Invoke(uv_work_t *req);
  static void Work_AfterInvoke(uv_work_t *req, int status);

  Napi::Value Ping(const Napi::CallbackInfo &info);
  Napi::Value Close(const Napi::CallbackInfo &info);
  Napi::Value Reopen(const Napi::CallbackInfo &info);
  Napi::Value IsAlive(const Napi::CallbackInfo &info);

  unsigned int paramSize;
  RFC_CONNECTION_PARAMETER *connectionParams;
  RFC_CONNECTION_HANDLE connectionHandle;
  bool alive;

  RFC_RC rc;
  RFC_ERROR_INFO errorInfo;

  void LockMutex(void);
  void UnlockMutex(void);
  uv_sem_t invocationMutex;

  struct Baton
  {
    uv_work_t request;
    Client *client;
    Napi::FunctionReference callback;
    RFC_ERROR_INFO errorInfo;

    Baton(Client *client_, Napi::Function callback_) : client(client_)
    {
      client->Ref();
      request.data = this;
      if (!callback_.IsUndefined() && callback_.IsFunction())
      {
        callback.Reset(callback_, 1);
      }
    }

    virtual ~Baton()
    {
      client->Unref();
      callback.Reset();
    }
  };

public:
  typedef void (*Work_Callback)(Baton *baton);

  struct ClientBaton : Baton
  {
    ClientBaton(Client *client, Napi::Function callback) : Baton(client, callback){};
  };

  struct InvokeBaton : Baton
  {
    RFC_FUNCTION_HANDLE functionHandle;
    RFC_FUNCTION_DESC_HANDLE functionDescHandle;

    InvokeBaton(Client *client, Napi::Function callback, RFC_FUNCTION_HANDLE _functionHandle, RFC_FUNCTION_DESC_HANDLE _functionDescHandle) : Baton(client, callback)
    {
      functionHandle = _functionHandle;
      functionDescHandle = _functionDescHandle;
    };
  };
};

} // namespace node_rfc

#endif // NODE_SAPNWRFC_CLIENT_H_

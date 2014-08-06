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

#ifndef CLIENT_H
#define CLIENT_H

#include <v8.h>
#include <node.h>
#include <string>
#include <sapnwrfc.h>


using namespace v8;


class Client: public node::ObjectWrap {
  public:
    static void Init(v8::Handle<v8::Object> exports);

  private:
    Client();
    ~Client();

    static Handle<Value> GetVersion(const Arguments& args);

    static Handle<Value> New(const Arguments& args);
    static Handle<Value> Close(const Arguments& args);
    static Handle<Value> Reopen(const Arguments& args);
    static Handle<Value> IsAlive(const Arguments& args);
    static Handle<Value> ConnectionInfo(const Arguments& args);
    static Handle<Value> Ping(const Arguments& args);

    static Handle<Value> Connect(const Arguments& args);
    static void ConnectAsync(uv_work_t *req);
    static void ConnectAsyncAfter(uv_work_t *req);;

    void LockMutex(void);
    void UnlockMutex(void);
    static Handle<Value> Invoke(const Arguments& args);
    static void InvokeAsync(uv_work_t *req);
    static void InvokeAsyncAfter(uv_work_t *req);;

    RFC_CONNECTION_HANDLE connectionHandle;
    RFC_CONNECTION_PARAMETER *connectionParams;
    unsigned int paramSize;

    bool rstrip;
    bool alive;
    uv_mutex_t invocationMutex;
};


struct ClientBaton {
  uv_work_t request;
  Persistent<Function> callback;

  RFC_ERROR_INFO errorInfo;
  Client *wrapper;
};

struct InvokeBaton {
  uv_work_t request;
  Persistent<Function> callback;

  RFC_FUNCTION_HANDLE functionHandle;
  RFC_FUNCTION_DESC_HANDLE functionDescHandle;
  RFC_ERROR_INFO errorInfo;
  Client *wrapper;
};

#endif

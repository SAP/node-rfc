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

#include <uv.h>
//#include <v8.h>
#include <node.h>
#include <node_object_wrap.h>
#include <string>
#include <sapnwrfc.h>


using namespace v8;


class Client: public node::ObjectWrap {
  public:
    static void Init(Handle<Object> exports);

  private:
    explicit Client();
    ~Client();

    static void GetVersion(const FunctionCallbackInfo<Value>& args);

    static void New(const FunctionCallbackInfo<Value>& args);
    static void Close(const FunctionCallbackInfo<Value>& args);
    static void Reopen(const FunctionCallbackInfo<Value>& args);
    static void IsAlive(const FunctionCallbackInfo<Value>& args);
    static void ConnectionInfo(const FunctionCallbackInfo<Value>& args);
    static void Ping(const FunctionCallbackInfo<Value>& args);
    static void RunCallback(const FunctionCallbackInfo<Value>& args);

    static Persistent<Function> constructor;

    static void Connect(const FunctionCallbackInfo<Value>& args);
    static void ConnectAsync(uv_work_t *req);
    static void ConnectAsyncAfter(uv_work_t *req);;

    void LockMutex(void);
    void UnlockMutex(void);
    static void Invoke(const FunctionCallbackInfo<Value>& args);
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

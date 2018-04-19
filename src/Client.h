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

#include <nan.h>
#include <sapnwrfc.h>

using namespace v8;


class Client: public Nan::ObjectWrap {
  public:
    static NAN_MODULE_INIT(Init);

  private:
    explicit Client();
    ~Client();

    static NAN_METHOD(GetVersion);

    static NAN_METHOD(New);
    static NAN_METHOD(Close);
    static NAN_METHOD(Reopen);
    static NAN_METHOD(IsAlive);
    static NAN_METHOD(ConnectionInfo);
    static NAN_METHOD(Ping);
    static NAN_METHOD(RunCallback);

    static Nan::Persistent<Function> constructor;

    static NAN_METHOD(Connect);
    static void ConnectAsync(uv_work_t *req);
    static void ConnectAsyncAfter(uv_work_t *req, int);;

    void LockMutex(void);
    void UnlockMutex(void);
    static NAN_METHOD(Invoke);
    static void InvokeAsync(uv_work_t *req);
    static void InvokeAsyncAfter(uv_work_t *req, int);;

    RFC_CONNECTION_HANDLE connectionHandle;
    RFC_CONNECTION_PARAMETER *connectionParams;
    unsigned int paramSize;
    bool rstrip;
    bool alive;

    uv_sem_t invocationMutex;
};


struct ClientBaton {
  uv_work_t request;
  Nan::Persistent<Function> callback;

  RFC_ERROR_INFO errorInfo;
  Client *wrapper;
};

struct InvokeBaton {
  uv_work_t request;
  Nan::Persistent<Function> callback;

  RFC_FUNCTION_HANDLE functionHandle;
  RFC_FUNCTION_DESC_HANDLE functionDescHandle;
  RFC_ERROR_INFO errorInfo;
  Client *wrapper;
};

#endif

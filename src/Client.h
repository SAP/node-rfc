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

#define SAPNWRFC_BINDING_VERSION "1.0.5"

#define NODERFC_BCD_STRING 0
#define NODERFC_BCD_NUMBER 1
#define NODERFC_BCD_FUNCTION 2

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
    friend class CloseAsync;
    friend class ReopenAsync;
    friend class PingAsync;
    friend class PrepareAsync;
    friend class InvokeAsync;

    static Napi::FunctionReference constructor;
    static Napi::Object Init(Napi::Env env, Napi::Object exports);

    void init(Napi::Env env)
    {
        __env = env;

        paramSize = 0;
        connectionParams = NULL;
        connectionHandle = NULL;
        alive = false;
        __rstrip = true;
        __bcd = NODERFC_BCD_STRING;

        rc = (RFC_RC)0;
        errorInfo.code = rc;
    };

    Client(const Napi::CallbackInfo &info);
    ~Client(void);

private:
    static unsigned int __refCounter;
    unsigned int __refId;

    // Client API

    Napi::Value IdGetter(const Napi::CallbackInfo &info);
    Napi::Value VersionGetter(const Napi::CallbackInfo &info);
    Napi::Value OptionsGetter(const Napi::CallbackInfo &info);

    Napi::Value ConnectionInfo(const Napi::CallbackInfo &info);

    Napi::Value Connect(const Napi::CallbackInfo &info);
    Napi::Value Invoke(const Napi::CallbackInfo &info);

    Napi::Value Ping(const Napi::CallbackInfo &info);
    Napi::Value Close(const Napi::CallbackInfo &info);
    Napi::Value Reopen(const Napi::CallbackInfo &info);
    Napi::Value IsAlive(const Napi::CallbackInfo &info);

    // SAP NW RFC SDK

    SAP_UC *fillString(const Napi::String napistr);
    SAP_UC *fillString(std::string str);
    Napi::Value fillFunctionParameter(RFC_FUNCTION_DESC_HANDLE functionDescHandle, RFC_FUNCTION_HANDLE functionHandle, Napi::String name, Napi::Value value);
    Napi::Value fillStructure(RFC_STRUCTURE_HANDLE structHandle, RFC_TYPE_DESC_HANDLE functionDescHandle, SAP_UC *cName, Napi::Value value);
    Napi::Value fillVariable(RFCTYPE typ, RFC_FUNCTION_HANDLE functionHandle, SAP_UC *cName, Napi::Value value, RFC_TYPE_DESC_HANDLE functionDescHandle);

    Napi::Value wrapString(SAP_UC *uc, int length = -1);
    Napi::Value wrapStructure(RFC_TYPE_DESC_HANDLE typeDesc, RFC_STRUCTURE_HANDLE structHandle);
    Napi::Value wrapVariable(RFCTYPE typ, RFC_FUNCTION_HANDLE functionHandle, SAP_UC *cName, unsigned int cLen, RFC_TYPE_DESC_HANDLE typeDesc);
    Napi::Value wrapResult(RFC_FUNCTION_DESC_HANDLE functionDescHandle, RFC_FUNCTION_HANDLE functionHandle);

    // RFC ERRORS

    Napi::Value RfcLibError(RFC_ERROR_INFO *errorInfo);
    Napi::Value AbapError(RFC_ERROR_INFO *errorInfo);
    Napi::Value wrapError(RFC_ERROR_INFO *errorInfo);

    Napi::Env __env = NULL;

    unsigned int paramSize;
    RFC_CONNECTION_PARAMETER *connectionParams;
    RFC_CONNECTION_HANDLE connectionHandle;
    bool alive;
    bool __rstrip;
    int __bcd = 0; // 0: string, 1: number, 2: function
    RFC_DIRECTION __filter_param_direction = (RFC_DIRECTION)0;
    Napi::FunctionReference __bcdFunction;
    // date
    Napi::FunctionReference __dateToABAP;
    Napi::FunctionReference __dateFromABAP;
    // time
    Napi::FunctionReference __timeToABAP;
    Napi::FunctionReference __timeFromABAP;

    RFC_RC rc;
    RFC_ERROR_INFO errorInfo;

    void LockMutex(void);
    void UnlockMutex(void);
    uv_sem_t invocationMutex;
};

} // namespace node_rfc

#endif // NODE_SAPNWRFC_CLIENT_H_

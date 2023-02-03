// SPDX-FileCopyrightText: 2014 SAP SE Srdjan Boskovic <srdjan.boskovic@sap.com>
//
// SPDX-License-Identifier: Apache-2.0

#ifndef NodeRfc_Client_H
#define NodeRfc_Client_H

#include <tuple>
#include <uv.h>
#include "nwrfcsdk.h"
namespace node_rfc
{
    extern Napi::Env __env;

    class Pool;

    class Client : public Napi::ObjectWrap<Client>
    {
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
        Client(const Napi::CallbackInfo &info);
        ~Client(void);

    private:
        static Napi::Object NewInstance(Napi::Env env);
        Napi::Value IdGetter(const Napi::CallbackInfo &info);
        Napi::Value AliveGetter(const Napi::CallbackInfo &info);
        Napi::Value ConfigGetter(const Napi::CallbackInfo &info);
        Napi::Value ConnectionHandleGetter(const Napi::CallbackInfo &info);
        Napi::Value PoolIdGetter(const Napi::CallbackInfo &info);
        Napi::ObjectReference clientParamsRef;
        Napi::ObjectReference clientOptionsRef;

        Napi::Value connectionClosedError(std::string suffix);
        ErrorPair connectionCheck(RFC_ERROR_INFO *errorInfo);
        Napi::Value getOperationError(bool conn_closed, std::string operation, ErrorPair connectionCheckError, RFC_ERROR_INFO *errorInfo, Napi::Env env);

        Napi::Value ConnectionInfo(const Napi::CallbackInfo &info);
        Napi::Value Release(const Napi::CallbackInfo &info);
        Napi::Value Open(const Napi::CallbackInfo &info);
        Napi::Value Close(const Napi::CallbackInfo &info);
        Napi::Value Cancel(const Napi::CallbackInfo &info);
        Napi::Value ResetServerContext(const Napi::CallbackInfo &info);
        Napi::Value Ping(const Napi::CallbackInfo &info);
        Napi::Value Invoke(const Napi::CallbackInfo &info);

        RfmErrorPath errorPath;

        void init(Napi::Env env)
        {
            if (node_rfc::__env == NULL)
            {
                node_rfc::__env = env;
            }

            id = Client::_id++;

            pool = NULL;
            connectionHandle = NULL;

            uv_sem_init(&invocationMutex, 1);
        };

        static uint_t _id;
        uint_t id;
        Pool *pool;
        RFC_CONNECTION_HANDLE connectionHandle;

        ConnectionParamsStruct client_params;
        ClientOptionsStruct client_options;

        void LockMutex();
        void UnlockMutex();
        uv_sem_t invocationMutex;
    };

} // namespace node_rfc

#define CONNECTION_INFO_SET(property) \
    infoObj.Set(#property, wrapString(connInfo.property, strlenU(connInfo.property)));
#endif

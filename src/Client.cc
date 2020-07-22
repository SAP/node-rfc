
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

#include "nwrfcsdk.h"
#include "Client.h"
#include "Pool.h"

namespace node_rfc
{
    extern Napi::Env __env;

    uint_t Client::_id = 1;

    ErrorPair connectionCheckErrorInit()
    {
        RFC_ERROR_INFO errorInfo;
        errorInfo.code = RFC_OK;
        return ErrorPair(errorInfo, "");
    }

    Napi::Value Client::getOperationError(bool conn_closed, std::string operation, ErrorPair connectionCheckError, RFC_ERROR_INFO *errorInfo, Napi::Env env)
    {
        Napi::EscapableHandleScope scope(env);
        Napi::Value error = Env().Undefined();
        if (conn_closed)
        {
            error = connectionClosedError(operation);
        }
        else if (connectionCheckError.first.code != RFC_OK)
        {
            error = rfcSdkError(&connectionCheckError.first);
        }
        else if (connectionCheckError.second.length() > 0)
        {
            error = nodeRfcError(connectionCheckError.second);
        }
        else if (errorInfo->code != RFC_OK)
        {
            error = rfcSdkError(errorInfo);
        }
        return scope.Escape(error);
    }

    void checkConnectionParams(Napi::Object clientParamsObject, ConnectionParamsStruct *clientParams)
    {
        Napi::Array paramNames = clientParamsObject.GetPropertyNames();
        clientParams->paramSize = paramNames.Length();
        //DEBUG("checkConnectionParams ", clientParams->paramSize);
        clientParams->connectionParams = static_cast<RFC_CONNECTION_PARAMETER *>(malloc(clientParams->paramSize * sizeof(RFC_CONNECTION_PARAMETER)));
        for (uint_t ii = 0; ii < clientParams->paramSize; ii++)
        {
            Napi::String name = paramNames.Get(ii).ToString();
            //DEBUG("checkConnectionParams ", name.Utf8Value().c_str());
            Napi::String value = clientParamsObject.Get(name).ToString();
            clientParams->connectionParams[ii].name = fillString(name);
            clientParams->connectionParams[ii].value = fillString(value);
        }
    }

    void checkClientOptions(Napi::Object clientOptionsObject, ClientOptionsStruct *client_options)
    {
        char errmsg[254];
        Napi::Array props = clientOptionsObject.GetPropertyNames();
        for (uint_t ii = 0; ii < props.Length(); ii++)
        {
            std::string key = props.Get(ii).ToString().Utf8Value();
            Napi::Value opt = clientOptionsObject.Get(key).As<Napi::Value>();

            // Client option: "bcd"
            if (key.compare(std::string(CLIENT_OPTION_KEY_BCD)) == 0)
            {
                if (opt.IsFunction())
                {
                    client_options->bcd = CLIENT_OPTION_BCD_FUNCTION;
                    client_options->bcdFunction = Napi::Persistent(opt.As<Napi::Function>());
                }
                else if (opt.IsString())
                {
                    std::string bcdString = opt.ToString().Utf8Value();
                    if (bcdString.compare(std::string("number")) == 0)
                    {
                        client_options->bcd = CLIENT_OPTION_BCD_NUMBER;
                    }
                    else
                    {
                        sprintf(errmsg, "Client option \"%s\" value not allowed: \"%s\"; see %s", CLIENT_OPTION_KEY_BCD, &bcdString[0], USAGE_URL);
                        Napi::TypeError::New(node_rfc::__env, errmsg).ThrowAsJavaScriptException();
                    }
                }
            }

            // Client option: "date"
            else if (key.compare(std::string(CLIENT_OPTION_KEY_DATE)) == 0)
            {
                if (!opt.IsObject())
                {
                    opt = node_rfc::__env.Null();
                }
                else
                {
                    Napi::Value toABAP = opt.As<Napi::Object>().Get("toABAP");
                    Napi::Value fromABAP = opt.As<Napi::Object>().Get("fromABAP");
                    if (!toABAP.IsFunction() || !fromABAP.IsFunction())
                    {
                        sprintf(errmsg, "Client option \"%s\" is not an object with toABAP() and fromABAP() functions; see %s", CLIENT_OPTION_KEY_DATE, USAGE_URL);
                        Napi::TypeError::New(node_rfc::__env, errmsg).ThrowAsJavaScriptException();
                    }
                    else
                    {
                        client_options->dateToABAP = Napi::Persistent(toABAP.As<Napi::Function>());
                        client_options->dateFromABAP = Napi::Persistent(fromABAP.As<Napi::Function>());
                    }
                }
            }

            // Client option: "time"
            else if (key.compare(std::string(CLIENT_OPTION_KEY_TIME)) == 0)
            {
                if (!opt.IsObject())
                {
                    opt = node_rfc::__env.Null();
                }
                else
                {
                    Napi::Value toABAP = opt.As<Napi::Object>().Get("toABAP");
                    Napi::Value fromABAP = opt.As<Napi::Object>().Get("fromABAP");
                    if (!toABAP.IsFunction() || !fromABAP.IsFunction())
                    {
                        sprintf(errmsg, "Client option \"%s\" is not an object with toABAP() and fromABAP() functions; see %s", CLIENT_OPTION_KEY_TIME, USAGE_URL);
                        Napi::TypeError::New(node_rfc::__env, errmsg).ThrowAsJavaScriptException();
                        ;
                    }
                    else
                    {
                        client_options->timeToABAP = Napi::Persistent(toABAP.As<Napi::Function>());
                        client_options->timeFromABAP = Napi::Persistent(fromABAP.As<Napi::Function>());
                    }
                }
            }

            // Client option: "filter"
            else if (key.compare(std::string(CLIENT_OPTION_KEY_FILTER)) == 0)
            {
                client_options->filter_param_type = (RFC_DIRECTION)clientOptionsObject.Get(key).As<Napi::Number>().Uint32Value();
                if (((int)client_options->filter_param_type < 1) || ((int)client_options->filter_param_type) > 4)
                {
                    sprintf(errmsg, "Client option \"%s\" value allowed: \"%u\"; see %s", CLIENT_OPTION_KEY_FILTER, (uint_t)client_options->filter_param_type, USAGE_URL);
                    Napi::TypeError::New(node_rfc::__env, errmsg).ThrowAsJavaScriptException();
                }
            }

            // Client option: "stateless"
            else if (key.compare(std::string(CLIENT_OPTION_KEY_STATELESS)) == 0)
            {
                if (!clientOptionsObject.Get(key).IsBoolean())
                {
                    sprintf(errmsg, "Client option \"%s\" requires a boolean value; see %s", CLIENT_OPTION_KEY_STATELESS, USAGE_URL);
                    Napi::TypeError::New(node_rfc::__env, errmsg).ThrowAsJavaScriptException();
                }
                client_options->stateless = clientOptionsObject.Get(key).As<Napi::Boolean>();
            }

            // Client option: unknown
            else
            {
                sprintf(errmsg, "Client option not allowed: \"%s\"; see %s", key.c_str(), USAGE_URL);
                Napi::TypeError::New(node_rfc::__env, errmsg).ThrowAsJavaScriptException();
            }
        }
    }

    Napi::Object Client::Init(Napi::Env env, Napi::Object exports)
    {
        Napi::HandleScope scope(env);

        Napi::Function func = DefineClass(
            env, "Client", {
                               InstanceAccessor("_id", &Client::IdGetter, nullptr),
                               InstanceAccessor("_alive", &Client::AliveGetter, nullptr),
                               InstanceAccessor("_connectionHandle", &Client::ConnectionHandleGetter, nullptr),
                               InstanceAccessor("_pool_id", &Client::PoolIdGetter, nullptr),
                               InstanceAccessor("_config", &Client::ConfigGetter, nullptr),
                               InstanceMethod("connectionInfo", &Client::ConnectionInfo),
                               InstanceMethod("open", &Client::Open),
                               InstanceMethod("close", &Client::Close),
                               InstanceMethod("release", &Client::Release),
                               InstanceMethod("resetServerContext", &Client::ResetServerContext),
                               InstanceMethod("ping", &Client::Ping),
                               InstanceMethod("invoke", &Client::Invoke),
                           });

        Napi::FunctionReference *constructor = new Napi::FunctionReference();
        *constructor = Napi::Persistent(func);
        constructor->SuppressDestruct();
        env.SetInstanceData(constructor);

        exports.Set("Client", func);
        return exports;
    }

    Napi::Value Client::IdGetter(const Napi::CallbackInfo &info)
    {
        return Napi::Number::New(Env(), id);
    }

    Napi::Value Client::AliveGetter(const Napi::CallbackInfo &info)
    {
        return Napi::Boolean::New(Env(), connectionHandle != NULL);
    }

    Napi::Value Client::ConfigGetter(const Napi::CallbackInfo &info)
    {
        Napi::Env env = info.Env();
        Napi::EscapableHandleScope scope(env);
        Napi::Object config = Napi::Object::New(env);
        if (pool != NULL)
        {
            if (!pool->connectionParameters.IsEmpty())
            {
                config.Set(POOL_KEY_CONNECTION_PARAMS, pool->connectionParameters.Value());
            }
            config.Set(POOL_KEY_CLIENT_OPTIONS, pool->client_options._Value(info.Env()));
        }
        else
        {
            if (!clientParamsRef.IsEmpty())
            {
                config.Set(POOL_KEY_CONNECTION_PARAMS, clientParamsRef.Value());
            }
            config.Set(POOL_KEY_CLIENT_OPTIONS, client_options._Value(info.Env()));
        }

        return scope.Escape(config);
    }

    Napi::Value Client::ConnectionHandleGetter(const Napi::CallbackInfo &info)
    {
        return Napi::Number::New(info.Env(), (double)(unsigned long long)this->connectionHandle);
    }

    Napi::Value Client::PoolIdGetter(const Napi::CallbackInfo &info)
    {
        if (pool == NULL)
        {
            return Number::New(info.Env(), 0);
        }
        return Napi::Number::New(info.Env(), pool->id);
    }

    Napi::Value Client::ConnectionInfo(const Napi::CallbackInfo &info)
    {
        if (connectionHandle == NULL)
        {
            return connectionClosedError("connectionInfo");
        }

        Napi::Object infoObj = Napi::Object::New(info.Env());
        RFC_ERROR_INFO errorInfo;
        RFC_ATTRIBUTES connInfo;
        RFC_RC rc = RfcGetConnectionAttributes(connectionHandle, &connInfo, &errorInfo);

        if (rc != RFC_OK || errorInfo.code != RFC_OK)
        {
            return rfcSdkError(&errorInfo);
        }

        infoObj.Set("dest", wrapString(connInfo.dest, 64));
        infoObj.Set("host", wrapString(connInfo.host, 100));
        infoObj.Set("partnerHost", wrapString(connInfo.partnerHost, 100));
        infoObj.Set("sysNumber", wrapString(connInfo.sysNumber, 2));
        infoObj.Set("sysId", wrapString(connInfo.sysId, 8));
        infoObj.Set("client", wrapString(connInfo.client, 3));
        infoObj.Set("user", wrapString(connInfo.user, 12));
        infoObj.Set("language", wrapString(connInfo.language, 2));
        infoObj.Set("trace", wrapString(connInfo.trace, 1));
        infoObj.Set("isoLanguage", wrapString(connInfo.isoLanguage, 2));
        infoObj.Set("codepage", wrapString(connInfo.codepage, 4));
        infoObj.Set("partnerCodepage", wrapString(connInfo.partnerCodepage, 4));
        infoObj.Set("rfcRole", wrapString(connInfo.rfcRole, 1));
        infoObj.Set("type", wrapString(connInfo.type, 1));
        infoObj.Set("partnerType", wrapString(connInfo.partnerType, 1));
        infoObj.Set("rel", wrapString(connInfo.rel, 4));
        infoObj.Set("partnerRel", wrapString(connInfo.partnerRel, 4));
        infoObj.Set("kernelRel", wrapString(connInfo.kernelRel, 4));
        infoObj.Set("cpicConvId", wrapString(connInfo.cpicConvId, 8));
        infoObj.Set("progName", wrapString(connInfo.progName, 128));
        infoObj.Set("partnerBytesPerChar", wrapString(connInfo.partnerBytesPerChar, 1));
        infoObj.Set("partnerSystemCodepage", wrapString(connInfo.partnerSystemCodepage, 4));
        infoObj.Set("partnerIP", wrapString(connInfo.partnerIP, 15));
        infoObj.Set("partnerIPv6", wrapString(connInfo.partnerIPv6, 45));
        // infoObj.Set("reserved", wrapString(connInfo.reserved, 17));

        return infoObj;
    }

    Client::Client(const Napi::CallbackInfo &info) : Napi::ObjectWrap<Client>(info)
    {
        init(info.Env());

        DEBUG("Client::Client ", id);

        if (!info[0].IsUndefined() && (info[0].IsFunction() || !info[0].IsObject()))
        {
            Napi::TypeError::New(Env(), "Client constructor requires connection parameters").ThrowAsJavaScriptException();
            return;
        }

        if (info.Length() > 0)
        {
            clientParamsRef = Napi::Persistent(info[0].As<Napi::Object>());
            checkConnectionParams(clientParamsRef.Value(), &client_params);
        }

        if (info.Length() > 1)
        {
            clientOptionsRef = Napi::Persistent(info[1].As<Napi::Object>());
            checkClientOptions(clientOptionsRef.Value(), &client_options);
        }

        if (info.Length() > 2)
        {
            char errmsg[254];
            sprintf(errmsg, "Client constructor requires max. two arguments, received %zu; see: %s", info.Length(), USAGE_URL);
            Napi::TypeError::New(node_rfc::__env, errmsg).ThrowAsJavaScriptException();
        }
    };

    Client::~Client(void)
    {
        DEBUG("~ Client ", id);

        destructor_call = true;

        if (pool == NULL)
        {
            // Close own connection
            if (connectionHandle != NULL)
            {
                RFC_ERROR_INFO errorInfo;
                DEBUG("Closing direct connection ", (pointer_t)connectionHandle);
                RFC_RC rc = RfcCloseConnection(connectionHandle, &errorInfo);
                if (rc != RFC_OK)
                {
                    EDEBUG("Error closing the direct connection handle ", (pointer_t)connectionHandle, "client ", id);
                }
            }
            else
            {
                DEBUG("Client ", id, " handle already closed");
            }

            // Unref client config
            if (!clientParamsRef.IsEmpty())
            {
                clientParamsRef.Reset();
            }
            if (!clientOptionsRef.IsEmpty())
            {
                clientOptionsRef.Reset();
            }
        }
        else
        {
            if (connectionHandle != NULL)
            {
                pool->releaseClient(connectionHandle);
            }
        }

        uv_sem_destroy(&invocationMutex);
    }

    Napi::Value Client::connectionClosedError(std::string suffix)
    {
        std::ostringstream err;
        err << "RFM client request over closed connection: " << suffix;
        return nodeRfcError(err.str());
    }

    Napi::Object Client::NewInstance(Napi::Env env)
    {
        //DEBUG("Client::NewInstance");
        Napi::EscapableHandleScope scope(env);
        Napi::Object obj = env.GetInstanceData<Napi::FunctionReference>()->New({});
        return scope.Escape(napi_value(obj)).ToObject();
    }

    class OpenAsync : public Napi::AsyncWorker
    {
    public:
        OpenAsync(Napi::Function &callback, Client *client)
            : Napi::AsyncWorker(callback), client(client) {}
        ~OpenAsync() {}

        void Execute()
        {
            client->LockMutex();
            client->connectionHandle = RfcOpenConnection(client->client_params.connectionParams, client->client_params.paramSize, &errorInfo);
            if (errorInfo.code != RFC_OK)
            {
                client->connectionHandle = NULL;
            }
            client->UnlockMutex();
        }

        void OnOK()
        {
            if (errorInfo.code != RFC_OK)
            {
                Callback().Call({rfcSdkError(&errorInfo)});
            }
            else
            {
                Callback().Call({});
            }
            Callback().Reset();
        }

    private:
        Client *client;
        RFC_ERROR_INFO errorInfo;
    };

    class CloseAsync : public Napi::AsyncWorker
    {
    public:
        CloseAsync(Napi::Function &callback, Client *client)
            : Napi::AsyncWorker(callback), client(client) {}
        ~CloseAsync() {}

        void Execute()
        {
            client->LockMutex();
            conn_closed = (client->connectionHandle == NULL);
            if (!conn_closed)
            {
                RfcCloseConnection(client->connectionHandle, &errorInfo);
                client->connectionHandle = NULL;
            }
            client->UnlockMutex();
        }

        void OnOK()
        {
            Napi::HandleScope scope(Env());

            if (conn_closed)
            {
                Callback().Call({client->connectionClosedError("close()")});
            }
            else if (errorInfo.code != RFC_OK)
            {
                Callback().Call({rfcSdkError(&errorInfo)});
            }
            else
            {
                Callback().Call({});
            }
            Callback().Reset();
        }

    private:
        Client *client;
        RFC_ERROR_INFO errorInfo;
        bool conn_closed;
    };

    class ResetServerAsync : public Napi::AsyncWorker
    {
    public:
        ResetServerAsync(Napi::Function &callback, Client *client)
            : Napi::AsyncWorker(callback), client(client) {}
        ~ResetServerAsync() {}

        void Execute()
        {
            client->LockMutex();
            conn_closed = (client->connectionHandle == NULL);
            if (!conn_closed)
            {
                RfcResetServerContext(client->connectionHandle, &errorInfo);
                if (errorInfo.code != RFC_OK)
                {
                    connectionCheckError = client->connectionCheck(&errorInfo);
                }
            }
            client->UnlockMutex();
        }

        void OnOK()
        {
            Napi::HandleScope scope(Env());
            Napi::Value error = client->getOperationError(conn_closed, "resetServerContext()", connectionCheckError, &errorInfo, Env());
            Callback().Call({error});
            Callback().Reset();
        }

    private:
        Client *client;
        RFC_ERROR_INFO errorInfo;
        bool conn_closed;
        ErrorPair connectionCheckError = connectionCheckErrorInit();
    };

    class PingAsync : public Napi::AsyncWorker
    {
    public:
        PingAsync(Napi::Function &callback, Client *client)
            : Napi::AsyncWorker(callback), client(client) {}
        ~PingAsync() {}

        void Execute()
        {
            client->LockMutex();
            conn_closed = (client->connectionHandle == NULL);
            if (!conn_closed)
            {
                RfcPing(client->connectionHandle, &errorInfo);
                if (errorInfo.code != RFC_OK)
                {
                    connectionCheckError = client->connectionCheck(&errorInfo);
                }
            }
            client->UnlockMutex();
        }

        void OnOK()
        {
            Napi::HandleScope scope(Env());
            Napi::Value error = client->getOperationError(conn_closed, "ping()", connectionCheckError, &errorInfo, Env());
            Callback().Call({error, Napi::Boolean::New(Env(), error.IsUndefined())});
            Callback().Reset();
        }

    private:
        Client *client;
        bool conn_closed;
        RFC_ERROR_INFO errorInfo;
        ErrorPair connectionCheckError = connectionCheckErrorInit();
    };

    class InvokeAsync : public Napi::AsyncWorker
    {
    public:
        InvokeAsync(Napi::Function &callback, Client *client, RFC_FUNCTION_HANDLE functionHandle, RFC_FUNCTION_DESC_HANDLE functionDescHandle)
            : Napi::AsyncWorker(callback), client(client), functionHandle(functionHandle), functionDescHandle(functionDescHandle)
        {
        }
        ~InvokeAsync() {}

        void Execute()
        {
            client->LockMutex();
            conn_closed = (client->connectionHandle == NULL);
            if (!conn_closed)
            {
                RfcInvoke(client->connectionHandle, functionHandle, &errorInfo);
                if (errorInfo.code != RFC_OK)
                {
                    connectionCheckError = client->connectionCheck(&errorInfo);
                }
            }
        }

        void OnOK()
        {
            Napi::HandleScope scope(Env());

            std::string closed_errmsg = "invoke() " + wrapString(client->errorPath.functionName).As<Napi::String>().Utf8Value();
            ValuePair result = ValuePair(client->getOperationError(conn_closed, closed_errmsg, connectionCheckError, &errorInfo, Env()), Env().Undefined());

            if (result.first.IsUndefined())
            {
                result = client->wrapResult(functionDescHandle, functionHandle);
            }

            RfcDestroyFunction(functionHandle, NULL);
            client->UnlockMutex();

            Callback().Call({result.first, result.second});
            Callback().Reset();
        }

    private:
        Client *client;
        RFC_FUNCTION_HANDLE functionHandle;
        RFC_FUNCTION_DESC_HANDLE functionDescHandle;
        RFC_ERROR_INFO errorInfo;
        bool conn_closed;
        ErrorPair connectionCheckError = connectionCheckErrorInit();
    };
    class PrepareAsync : public Napi::AsyncWorker
    {
    public:
        PrepareAsync(Napi::Function &callback, Client *client,
                     Napi::String rfmName, Napi::Array &notRequestedParameters, Napi::Object &rfmParams)
            : Napi::AsyncWorker(callback), client(client),
              notRequested(Napi::Persistent(notRequestedParameters)), rfmParams(Napi::Persistent(rfmParams))
        {
            funcName = client->fillString(rfmName);
            client->errorPath.setFunctionName(funcName);
        }
        ~PrepareAsync()
        {
            free(funcName);
        }

        void Execute()
        {
            client->LockMutex();
            conn_closed = (client->connectionHandle == NULL);
            if (!conn_closed)
            {
                functionDescHandle = RfcGetFunctionDesc(client->connectionHandle, funcName, &errorInfo);
            }
        }

        void OnOK()
        {
            client->UnlockMutex();
            RFC_FUNCTION_HANDLE functionHandle = NULL;
            Napi::Value argv[2] = {Env().Undefined(), Env().Undefined()};

            if (conn_closed)
            {
                std::string errmsg = "invoke() " + wrapString(client->errorPath.functionName).As<Napi::String>().Utf8Value();
                argv[0] = client->connectionClosedError(errmsg);
            }
            else if (functionDescHandle == NULL || errorInfo.code != RFC_OK)
            {
                argv[0] = rfcSdkError(&errorInfo);
            }
            else
            {
                // function descriptor handle created, proceed with function handle
                functionHandle = RfcCreateFunction(functionDescHandle, &errorInfo);

                if (errorInfo.code != RFC_OK)
                {
                    argv[0] = rfcSdkError(&errorInfo);
                }
                else
                {
                    for (uint_t i = 0; i < notRequested.Value().Length(); i++)
                    {
                        Napi::String name = notRequested.Value().Get(i).ToString();
                        SAP_UC *paramName = client->fillString(name);
                        RFC_RC rc = RfcSetParameterActive(functionHandle, paramName, 0, &errorInfo);
                        free(const_cast<SAP_UC *>(paramName));
                        if (rc != RFC_OK)
                        {
                            argv[0] = rfcSdkError(&errorInfo);
                            break;
                        }
                    }
                }
            }

            notRequested.Reset();

            if (argv[0].IsUndefined())
            {
                Napi::Object params = rfmParams.Value();
                Napi::Array paramNames = params.GetPropertyNames();
                uint_t paramSize = paramNames.Length();

                for (uint_t i = 0; i < paramSize; i++)
                {
                    Napi::String name = paramNames.Get(i).ToString();
                    Napi::Value value = params.Get(name);
                    argv[0] = client->fillFunctionParameter(functionDescHandle, functionHandle, name, value);

                    if (!argv[0].IsUndefined())
                    {
                        break;
                    }
                }
            }

            rfmParams.Reset();

            if (argv[0].IsUndefined())
            {
                Napi::Function callbackFunction = Callback().Value().As<Napi::Function>();
                (new InvokeAsync(callbackFunction, client, functionHandle, functionDescHandle))->Queue();
            }
            else
            {
                Callback().Call({argv[0], argv[1]});
                Callback().Reset();
            }
        }

    private:
        Client *client;
        SAP_UC *funcName;

        Napi::Reference<Napi::Array> notRequested;
        Napi::Reference<Napi::Object> rfmParams;

        RFC_FUNCTION_DESC_HANDLE functionDescHandle;
        RFC_ERROR_INFO errorInfo;
        bool conn_closed;
    };

    ErrorPair Client::connectionCheck(RFC_ERROR_INFO *errorInfo)
    {
        RFC_ERROR_INFO errorInfoOpen;

        errorInfoOpen.code = RFC_OK;

        if (
            // error code check
            errorInfo->code == RFC_COMMUNICATION_FAILURE || // Error in Network & Communication layer.
            errorInfo->code == RFC_ABAP_RUNTIME_FAILURE ||  // SAP system runtime error (SYSTEM_FAILURE): Shortdump on the backend side.
            errorInfo->code == RFC_ABAP_MESSAGE ||          // The called function module raised an E-, A- or X-Message.
            errorInfo->code == RFC_EXTERNAL_FAILURE ||      // Problems in the RFC runtime of the external program (i.e "this" library)
            // error group check, for even more robustness here
            errorInfo->group == ABAP_RUNTIME_FAILURE ||  // ABAP Message raised in ABAP function modules or in ABAP runtime of the backend (e.g Kernel)
            errorInfo->group == LOGON_FAILURE ||         // Error message raised when logon fails
            errorInfo->group == COMMUNICATION_FAILURE || // Problems with the network connection (or backend broke down and killed the connection)
            errorInfo->group == EXTERNAL_RUNTIME_FAILURE // Problems in the RFC runtime of the external program (i.e "this" library)
            )                                            // closed
        {
            DEBUG("Critical ABAP error ", (pointer_t)this->connectionHandle);
            RFC_CONNECTION_HANDLE new_handle;
            RFC_CONNECTION_HANDLE old_handle = this->connectionHandle;
            this->connectionHandle = NULL;
            if (pool == NULL)
            {
                new_handle = RfcOpenConnection(client_params.connectionParams, client_params.paramSize, &errorInfoOpen);
            }
            else
            {
                new_handle = RfcOpenConnection(pool->client_params.connectionParams, pool->client_params.paramSize, &errorInfoOpen);
            }
            if (errorInfoOpen.code != RFC_OK)
            {
                // error getting a new handle
                return ErrorPair(errorInfoOpen, "");
            }

            if (pool != NULL)
            {
                std::string updateError = pool->updateLeasedHandle(old_handle, new_handle);
                if (updateError.length() > 0)
                {
                    // pool update failed
                    return ErrorPair(errorInfoOpen, updateError);
                }

                DEBUG("// assign new handle to managed client");
                this->connectionHandle = new_handle;
            }
            else
            {
                // assign new handle to direct client
                this->connectionHandle = new_handle;
            }
        }
        else
        {
            DEBUG("Non-critical ABAP error: ", (pointer_t)this->connectionHandle);
        }

        return ErrorPair(errorInfoOpen, "");
    }

    Napi::Value Client::Release(const Napi::CallbackInfo &info)
    {
        std::ostringstream errmsg;

        if (!info[1].IsFunction())
        {
            errmsg << "Client release() requires a callback function; see" << USAGE_URL;
            Napi::TypeError::New(info.Env(), errmsg.str()).ThrowAsJavaScriptException();
            return info.Env().Undefined();
        }

        Napi::Function callback = info[1].As<Napi::Function>();

        if (pool == NULL)
        {
            errmsg << "Client release() method is for managed clients only, use \"close()\" instead; see" << USAGE_URL;
            callback.Call({nodeRfcError(errmsg.str())});
            return info.Env().Undefined();
        }

        // the rest of arguments check done in Pool::Release
        pool->Release(info);

        return info.Env().Undefined();
    }

    Napi::Value Client::Open(const Napi::CallbackInfo &info)
    {
        std::ostringstream errmsg;

        if (!info[0].IsFunction())
        {
            errmsg << "Client open() requires a callback function; see" << USAGE_URL;
            Napi::TypeError::New(info.Env(), errmsg.str()).ThrowAsJavaScriptException();
            return info.Env().Undefined();
        }

        Napi::Function callback = info[0].As<Napi::Function>();

        if (pool != NULL)
        {
            errmsg << "Client \"open()\" not allowed for managed clients, , use the \"acquire()\" instead; see" << USAGE_URL;
            callback.Call({nodeRfcError(errmsg.str())});
            return info.Env().Undefined();
        }

        (new OpenAsync(callback, this))->Queue();

        return info.Env().Undefined();
    }

    Napi::Value Client::Close(const Napi::CallbackInfo &info)
    {
        std::ostringstream errmsg;

        if (!info[0].IsFunction())
        {
            errmsg << "Client close() requires a callback function; see" << USAGE_URL;
            Napi::TypeError::New(info.Env(), errmsg.str()).ThrowAsJavaScriptException();
            return info.Env().Undefined();
        }

        Napi::Function callback = info[0].As<Napi::Function>();

        if (pool != NULL)
        {
            // Managed connection error
            errmsg << "Client \"close()\" method not allowed for managed clients, use the \"release()\" instead; see" << USAGE_URL;
            callback.Call({nodeRfcError(errmsg.str())});
            return info.Env().Undefined();
        }

        (new CloseAsync(callback, this))->Queue();

        return info.Env().Undefined();
    }

    Napi::Value Client::ResetServerContext(const Napi::CallbackInfo &info)
    {
        std::ostringstream errmsg;
        if (!info[0].IsFunction())
        {
            errmsg << "Client resetServerContext() requires a callback function; see" << USAGE_URL;
            Napi::TypeError::New(info.Env(), errmsg.str()).ThrowAsJavaScriptException();
            return info.Env().Undefined();
        }
        Napi::Function callback = info[0].As<Napi::Function>();

        (new ResetServerAsync(callback, this))->Queue();

        return info.Env().Undefined();
    }

    Napi::Value Client::Ping(const Napi::CallbackInfo &info)
    {
        if (!info[0].IsFunction())
        {
            std::ostringstream errmsg;
            errmsg << "Client Ping() requires a callback function; see" << USAGE_URL;
            Napi::TypeError::New(info.Env(), errmsg.str()).ThrowAsJavaScriptException();
            return info.Env().Undefined();
        }

        Napi::Function callback = info[0].As<Napi::Function>();

        (new PingAsync(callback, this))->Queue();

        return info.Env().Undefined();
    }

    Napi::Value Client::Invoke(const Napi::CallbackInfo &info)
    {
        Napi::Array notRequested = Napi::Array::New(info.Env());
        Napi::Value bcd;

        if (!info[2].IsFunction())
        {
            std::ostringstream errmsg;
            errmsg << "Client invoke() requires a callback function; see" << USAGE_URL;
            Napi::TypeError::New(info.Env(), errmsg.str()).ThrowAsJavaScriptException();
            return info.Env().Undefined();
        }

        Napi::Function callback = info[2].As<Napi::Function>();

        if (info[3].IsObject())
        {
            Napi::Object options = info[3].ToObject();
            Napi::Array props = options.GetPropertyNames();
            for (uint_t i = 0; i < props.Length(); i++)
            {
                Napi::String key = props.Get(i).ToString();
                if (key.Utf8Value().compare(std::string("notRequested")) == (int)0)
                {
                    notRequested = options.Get(key).As<Napi::Array>();
                }
                else
                {
                    char err[256];
                    std::string optionName = key.Utf8Value();
                    sprintf(err, "Unknown option: %s", &optionName[0]);
                    Napi::TypeError::New(node_rfc::__env, err).ThrowAsJavaScriptException();
                }
            }
        }

        Napi::String rfmName = info[0].As<Napi::String>();
        Napi::Object rfmParams = info[1].As<Napi::Object>();

        (new PrepareAsync(callback, this, rfmName, notRequested, rfmParams))->Queue();

        return info.Env().Undefined();
    }

    void Client::LockMutex()
    {
        uv_sem_wait(&invocationMutex);
    }

    void Client::UnlockMutex()
    {
        uv_sem_post(&invocationMutex);
    }

} // namespace node_rfc

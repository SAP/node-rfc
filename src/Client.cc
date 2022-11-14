// SPDX-FileCopyrightText: 2014 SAP SE Srdjan Boskovic <srdjan.boskovic@sap.com>
//
// SPDX-License-Identifier: Apache-2.0

#include <thread>
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
                               // InstanceMethod("setIniPath", &Client::SetIniPath),
                               InstanceMethod("connectionInfo", &Client::ConnectionInfo),
                               InstanceMethod("open", &Client::Open),
                               InstanceMethod("close", &Client::Close),
                               InstanceMethod("cancel", &Client::Cancel),
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

    Napi::Value Client::ConnectionHandleGetter(const Napi::CallbackInfo &info)
    {
        return Napi::Number::New(info.Env(), (double)(unsigned long long)this->connectionHandle);
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
        CONNECTION_INFO_SET(dest);
        CONNECTION_INFO_SET(host);
        CONNECTION_INFO_SET(partnerHost)
        CONNECTION_INFO_SET(sysNumber);
        CONNECTION_INFO_SET(sysId);
        CONNECTION_INFO_SET(client);
        CONNECTION_INFO_SET(user);
        CONNECTION_INFO_SET(language);
        CONNECTION_INFO_SET(trace);
        CONNECTION_INFO_SET(isoLanguage);
        CONNECTION_INFO_SET(codepage);
        CONNECTION_INFO_SET(partnerCodepage);
        CONNECTION_INFO_SET(rfcRole);
        CONNECTION_INFO_SET(type);
        CONNECTION_INFO_SET(partnerType);
        CONNECTION_INFO_SET(rel);
        CONNECTION_INFO_SET(partnerRel);
        CONNECTION_INFO_SET(kernelRel);
        CONNECTION_INFO_SET(cpicConvId);
        CONNECTION_INFO_SET(progName);
        CONNECTION_INFO_SET(partnerBytesPerChar);
        CONNECTION_INFO_SET(partnerSystemCodepage);
        CONNECTION_INFO_SET(partnerIP);
        CONNECTION_INFO_SET(partnerIPv6);

        return infoObj;
    }

    Client::Client(const Napi::CallbackInfo &info) : Napi::ObjectWrap<Client>(info)
    {
        init(info.Env());

        DEBUG("Client::Client ", id);

        if (!info[0].IsUndefined() && (info[0].IsFunction() || !info[0].IsObject()))
        {
            Napi::TypeError::New(Env(), "Client connection parameters missing").ThrowAsJavaScriptException();
            return;
        }

        if (info.Length() > 0)
        {
            clientParamsRef = Napi::Persistent(info[0].As<Napi::Object>());
            getConnectionParams(clientParamsRef.Value(), &client_params);
        }

        if (!info[1].IsUndefined())
        {
            clientOptionsRef = Napi::Persistent(info[1].As<Napi::Object>());
            checkClientOptions(clientOptionsRef.Value(), &client_options);
        }

        if (info.Length() > 2)
        {
            char errmsg[ERRMSG_LENGTH];
            snprintf(errmsg, ERRMSG_LENGTH - 1, "Client constructor requires max. two arguments, received %zu; see: %s", info.Length(), USAGE_URL);
            Napi::TypeError::New(node_rfc::__env, errmsg).ThrowAsJavaScriptException();
        }
    };

    Client::~Client(void)
    {
        DEBUG("~ Client ", id);

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
                    EDEBUG("Warning: Error closing the direct connection handle ", (pointer_t)connectionHandle, " client ", id);
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
        // DEBUG("Client::NewInstance");
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
                result = getRfmParameters(functionDescHandle, functionHandle, &client->errorPath, &client->client_options);
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
            funcName = setString(rfmName);
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
                        SAP_UC *paramName = setString(name);
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
                    argv[0] = setRfmParameter(functionDescHandle, functionHandle, name, value, &client->errorPath, &client->client_options);

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

            if (errorInfo->code == RFC_CANCELED)
            {
                EDEBUG("Connection canceled ", (pointer_t)this->connectionHandle);
            }

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
            DEBUG("Critical connection error: group ", errorInfo->group, " code ", errorInfo->code, " closed handle ", (pointer_t)old_handle, " new handle ", (pointer_t)new_handle);
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
    void cancelConnection(RFC_RC *rc, RFC_CONNECTION_HANDLE connectionHandle, RFC_ERROR_INFO *errorInfo)
    {
        EDEBUG("Cancel connection ", (pointer_t)connectionHandle);
        *rc = RfcCancel(connectionHandle, errorInfo);
    }

    Napi::Value Client::Cancel(const Napi::CallbackInfo &info)
    {
        std::ostringstream errmsg;
        if (!info[0].IsFunction())
        {
            errmsg << "Client cancel() requires a callback function; see" << USAGE_URL;
            Napi::TypeError::New(info.Env(), errmsg.str()).ThrowAsJavaScriptException();
            return info.Env().Undefined();
        }

        Napi::Function callback = info[0].As<Napi::Function>();

        RFC_RC rc = RFC_OK;
        RFC_ERROR_INFO errorInfo;

        std::thread tcancel(cancelConnection, &rc, connectionHandle, &errorInfo);
        tcancel.join();

        if (rc == RFC_OK && errorInfo.code == RFC_OK)
        {
            callback.Call({});
        }
        else
        {
            callback.Call({rfcSdkError(&errorInfo)});
        }
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
                if (key.Utf8Value().compare(std::string(CALL_OPTION_KEY_NOTREQUESTED)) == (int)0)
                {
                    notRequested = options.Get(key).As<Napi::Array>();
                }
                else if (key.Utf8Value().compare(std::string(CALL_OPTION_KEY_TIMEOUT)) == (int)0)
                {
                    // timeout = options.Get(key).As<Napi::Array>();
                }
                else
                {
                    char err[255];
                    std::string optionName = key.Utf8Value();
                    snprintf(err, 254, "Unknown option: %s", &optionName[0]);
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

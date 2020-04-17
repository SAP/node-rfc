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

#include "Client.h"
#include "noderfcsdk.h"
#include "macros.h"

namespace node_rfc
{

unsigned int Client::__refCounter = 0;
extern Napi::Env __env;

class ConnectAsync : public Napi::AsyncWorker
{
public:
    ConnectAsync(Napi::Function &callback, Client *client)
        : Napi::AsyncWorker(callback), client(client) {}
    ~ConnectAsync() {}

    void Execute()
    {
        client->LockMutex(CLIENT_CONNECT);
        client->connectionHandle = RfcOpenConnection(client->connectionParams, client->paramSize, &errorInfo);
    }

    void OnOK()
    {
        client->alive = (errorInfo.code == RFC_OK);
        client->UnlockMutex(CLIENT_CONNECT);

        if (!client->alive)
        {
            Napi::Value argv[1] = {wrapError(&errorInfo)};
            CALLBACK_CALL(Env().Global(), Callback(), 1, argv);
        }
        else
        {
            CALLBACK_CALL(Env().Global(), Callback(), 0, {});
        }
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
        ongoing_calls = client->LockMutex(CLIENT_CLOSE);
        if (ongoing_calls == 0)
        {
            client->alive = false;
            RfcCloseConnection(client->connectionHandle, &errorInfo);
        }
    }

    void OnOK()
    {
        client->UnlockMutex(CLIENT_CLOSE);
        if (ongoing_calls == 0)
        {
            if (errorInfo.code == RFC_OK)
            {
                CALLBACK_CALL(Env().Global(), Callback(), 0, {});
            }
            else
            {
                Napi::Value argv[1] = {wrapError(&errorInfo)};
                CALLBACK_CALL(Env().Global(), Callback(), 1, argv);
            }
        }
        else
        {
            char err[256];
            sprintf(err, "Close rejected because %u RFC calls still running", ongoing_calls);
            Napi::Value argv[1] = {Napi::String::New(Env(), err)};
            CALLBACK_CALL(Env().Global(), Callback(), 1, argv);
        }
    }

private:
    Client *client;
    RFC_ERROR_INFO errorInfo;
    unsigned int ongoing_calls;
};

class ReopenAsync : public Napi::AsyncWorker
{
public:
    ReopenAsync(Napi::Function &callback, Client *client)
        : Napi::AsyncWorker(callback), client(client) {}
    ~ReopenAsync() {}

    void Execute()
    {
        client->LockMutex(CLIENT_REOPEN);
        client->alive = false;

        RfcCloseConnection(client->connectionHandle, &errorInfo);
        client->connectionHandle = RfcOpenConnection(client->connectionParams, client->paramSize, &errorInfo);
    }

    void OnOK()
    {
        client->UnlockMutex(CLIENT_REOPEN);

        client->alive = (errorInfo.code == RFC_OK);
        if (client->alive)
        {
            CALLBACK_CALL(Env().Global(), Callback(), 0, {});
        }
        else
        {
            Napi::Value argv[1] = {wrapError(&errorInfo)};
            CALLBACK_CALL(Env().Global(), Callback(), 1, argv);
        }
    }

private:
    Client *client;
    RFC_ERROR_INFO errorInfo;
};

class PingAsync : public Napi::AsyncWorker
{
public:
    PingAsync(Napi::Function &callback, Client *client)
        : Napi::AsyncWorker(callback), client(client) {}
    ~PingAsync() {}

    void Execute()
    {
        client->LockMutex(CLIENT_PING);
        RfcPing(client->connectionHandle, &errorInfo);
    }

    void OnOK()
    {
        RFC_INT isValid = 0;
        if (errorInfo.code == RFC_OK)
        {
            RfcIsConnectionHandleValid(client->connectionHandle, &isValid, &errorInfo);
        }
        client->UnlockMutex(CLIENT_PING);
        Napi::Value argv[2] = {Env().Undefined(), Env().Undefined()};
        if (errorInfo.code != RFC_OK)
            argv[0] = wrapError(&errorInfo);
        argv[1] = Napi::Boolean::New(Env(), isValid && errorInfo.code == RFC_OK);
        CALLBACK_CALL(Env().Global(), Callback(), 2, argv);
    }

private:
    Client *client;
    RFC_ERROR_INFO errorInfo;
};

class InvokeAsync : public Napi::AsyncWorker
{
public:
    InvokeAsync(Napi::Function &callback, Client *client, RFC_FUNCTION_HANDLE functionHandle, RFC_FUNCTION_DESC_HANDLE functionDescHandle)
        : Napi::AsyncWorker(callback), callback(Napi::Persistent(callback)),
          client(client), functionHandle(functionHandle), functionDescHandle(functionDescHandle)
    {
    }
    ~InvokeAsync() {}

    void Execute()
    {
        client->LockMutex(CLIENT_INVOKE);
        RfcInvoke(client->connectionHandle, functionHandle, &errorInfo);
    }

    void OnOK()
    {
        Napi::Value argv[2] = {Env().Undefined(), Env().Undefined()};

        if (errorInfo.code != RFC_OK)
        {
            if (
                errorInfo.code == RFC_COMMUNICATION_FAILURE || // Error in Network & Communication layer.
                errorInfo.code == RFC_ABAP_RUNTIME_FAILURE ||  // SAP system runtime error (SYSTEM_FAILURE): Shortdump on the backend side.
                errorInfo.code == RFC_ABAP_MESSAGE ||          // The called function module raised an E-, A- or X-Message.
                errorInfo.code == RFC_EXTERNAL_FAILURE)        // Problems in the RFC runtime of the external program (i.e "this" library)
            {
                // Connection closed, reopen
                RFC_ERROR_INFO openErrorInfo;
                client->connectionHandle = RfcOpenConnection(client->connectionParams, client->paramSize, &openErrorInfo);
                client->alive = (openErrorInfo.code == RFC_OK);
                if (!client->alive)
                    errorInfo = openErrorInfo;
            }
            argv[0] = wrapError(&errorInfo, client->alive);
        }
        else
        {
            argv[1] = client->wrapResult(functionDescHandle, functionHandle);
        }
        client->runningCalls--;
        client->UnlockMutex(CLIENT_INVOKE);
        RfcDestroyFunction(functionHandle, NULL);
        CALLBACK_CALL(Env().Global(), callback, 2, argv)
        callback.Reset();
    }

private:
    Napi::FunctionReference callback;
    Client *client;
    RFC_FUNCTION_HANDLE functionHandle;
    RFC_FUNCTION_DESC_HANDLE functionDescHandle;
    RFC_ERROR_INFO errorInfo;
};

class PrepareAsync : public Napi::AsyncWorker
{
public:
    PrepareAsync(Napi::Function &callback, Client *client,
                 Napi::String rfmName, Napi::Array &notRequestedParameters, Napi::Object &rfmParams)
        : Napi::AsyncWorker(callback),
          callback(Napi::Persistent(callback)), client(client),
          notRequested(Napi::Persistent(notRequestedParameters)), rfmParams(Napi::Persistent(rfmParams))
    {
        funcName = client->fillString(rfmName);
    }
    ~PrepareAsync() {}

    void Execute()
    {
        client->LockMutex(CLIENT_PREPARE);
        client->runningCalls++;
        functionDescHandle = RfcGetFunctionDesc(client->connectionHandle, funcName, &errorInfo);
        free(funcName);
    }

    void OnOK()
    {
        client->UnlockMutex(CLIENT_PREPARE);
        RFC_FUNCTION_HANDLE functionHandle = NULL;
        Napi::Value argv[2] = {Env().Undefined(), Env().Undefined()};

        if (functionDescHandle == NULL || errorInfo.code != RFC_OK)
            argv[0] = wrapError(&errorInfo);

        if (argv[0].IsUndefined())
        {
            functionHandle = RfcCreateFunction(functionDescHandle, &errorInfo);

            if (errorInfo.code != RFC_OK)
            {
                argv[0] = wrapError(&errorInfo);
            }
            else
            {
                for (unsigned int i = 0; i < notRequested.Value().Length(); i++)
                {
                    Napi::String name = notRequested.Value().Get(i).ToString();
                    SAP_UC *paramName = client->fillString(name);
                    RFC_RC rc = RfcSetParameterActive(functionHandle, paramName, 0, &errorInfo);
                    free(const_cast<SAP_UC *>(paramName));
                    if (rc != RFC_OK)
                    {
                        argv[0] = wrapError(&errorInfo);
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
            unsigned int paramSize = paramNames.Length();

            for (unsigned int i = 0; i < paramSize; i++)
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
            Napi::Function callbackFunction = callback.Value();
            (new InvokeAsync(callbackFunction, client, functionHandle, functionDescHandle))->Queue();
        }
        else
        {
            client->runningCalls--;
            CALLBACK_CALL(Env().Global(), callback, 1, argv);
            callback.Reset();
        }
    }

private:
    Napi::FunctionReference callback;
    Client *client;
    SAP_UC *funcName;

    Napi::Reference<Napi::Array> notRequested;
    Napi::Reference<Napi::Object> rfmParams;

    RFC_FUNCTION_DESC_HANDLE functionDescHandle;
    RFC_ERROR_INFO errorInfo;
};

Napi::FunctionReference Client::constructor;

Client::Client(const Napi::CallbackInfo &info) : Napi::ObjectWrap<Client>(info)
{
    char err[256];

    init(info.Env());

    if (!info.IsConstructCall())
    {
        Napi::Error::New(info.Env(), "Use the new operator to create instances of Rfc connection.").ThrowAsJavaScriptException();
    }

    if (info.Length() < 1)
    {
        Napi::Error::New(info.Env(), "Please provide connection parameters as argument").ThrowAsJavaScriptException();
    }

    if (!info[0].IsObject())
    {
        Napi::TypeError::New(info.Env(), "Connection parameters must be an object").ThrowAsJavaScriptException();
    }

    if (info.Length() > 2)
    {
        Napi::TypeError::New(info.Env(), "Too many parameters, only connection parameters object and options object expected").ThrowAsJavaScriptException();
    }

    if (info.Length() == 2)
    {
        if (!info[1].IsUndefined() && !info[1].IsObject())
        {
            Napi::TypeError::New(info.Env(), "Options must be an object").ThrowAsJavaScriptException();
        }

        Napi::Object options = info[1].ToObject();
        Napi::Array props = options.GetPropertyNames();
        for (unsigned int i = 0; i < props.Length(); i++)
        {
            Napi::String key = props.Get(i).ToString();
            Napi::Value opt = options.Get(key).As<Napi::Value>();
            if (key.Utf8Value().compare(std::string("bcd")) == (int)0)
            {
                if (opt.IsFunction())
                {
                    __bcd = NODERFC_BCD_FUNCTION;
                    __bcdFunction = Napi::Persistent(opt.As<Napi::Function>());
                }
                else if (opt.IsString())
                {
                    std::string bcdString = opt.ToString().Utf8Value();
                    if (bcdString.compare(std::string("number")) == (int)0)
                    {
                        __bcd = NODERFC_BCD_NUMBER;
                    }
                    else
                    {
                        sprintf(err, "Unknown bcd option, only 'number' or function allowed: %s", &bcdString[0]);
                        Napi::TypeError::New(node_rfc::__env, err).ThrowAsJavaScriptException();
                    }
                }
            }
            else if (key.Utf8Value().compare(std::string("date")) == (int)0)
            {
                if (!opt.IsObject())
                {
                    opt = info.Env().Null();
                }
                else
                {
                    Napi::String fn = Napi::String::New(info.Env(), "toABAP");
                    Napi::Value toABAP = opt.As<Napi::Object>().Get(fn);
                    fn = Napi::String::New(info.Env(), "fromABAP");
                    Napi::Value fromABAP = opt.As<Napi::Object>().Get(fn);
                    if (!toABAP.IsFunction() || !fromABAP.IsFunction())
                    {
                        opt = info.Env().Null();
                    }
                    else
                    {
                        __dateToABAP = Napi::Persistent(toABAP.As<Napi::Function>());
                        __dateFromABAP = Napi::Persistent(fromABAP.As<Napi::Function>());
                    }
                }
                if (opt.IsNull())
                {
                    sprintf(err, "Date option is not an object with toABAP and fromABAP functions");
                    Napi::TypeError::New(node_rfc::__env, err).ThrowAsJavaScriptException();
                }
            }
            else if (key.Utf8Value().compare(std::string("time")) == (int)0)
            {
                if (!opt.IsObject())
                {
                    opt = info.Env().Null();
                }
                else
                {
                    Napi::String fn = Napi::String::New(info.Env(), "toABAP");
                    Napi::Value toABAP = opt.As<Napi::Object>().Get(fn);
                    fn = Napi::String::New(info.Env(), "fromABAP");
                    Napi::Value fromABAP = opt.As<Napi::Object>().Get(fn);
                    if (!toABAP.IsFunction() || !fromABAP.IsFunction())
                    {
                        opt = info.Env().Null();
                    }
                    else
                    {
                        __timeToABAP = Napi::Persistent(toABAP.As<Napi::Function>());
                        __timeFromABAP = Napi::Persistent(fromABAP.As<Napi::Function>());
                    }
                }
                if (opt.IsNull())
                {
                    sprintf(err, "Date option is not an object with toABAP and fromABAP functions");
                    Napi::TypeError::New(node_rfc::__env, err).ThrowAsJavaScriptException();
                }
            }
            else if (key.Utf8Value().compare(std::string("filter")) == (int)0)
            {
                __filter_param_direction = (RFC_DIRECTION)options.Get(key).As<Napi::Number>().Int32Value();
                if (((int)__filter_param_direction < 1) || ((int)__filter_param_direction) > 4)
                {
                    sprintf(err, "Invalid key for the filter parameter direction (see RFC_DIRECTION): %u", (int)__filter_param_direction);
                    Napi::TypeError::New(node_rfc::__env, err).ThrowAsJavaScriptException();
                }
            }
            else
            {
                std::string optionName = key.Utf8Value();
                sprintf(err, "Unknown option: %s", &optionName[0]);
                Napi::TypeError::New(node_rfc::__env, err).ThrowAsJavaScriptException();
            }
        }
    }

    this->alive = false;
    Napi::Object connectionParams = info[0].ToObject();
    Napi::Array paramNames = connectionParams.GetPropertyNames();
    this->paramSize = paramNames.Length();
    this->connectionParams = static_cast<RFC_CONNECTION_PARAMETER *>(malloc(this->paramSize * sizeof(RFC_CONNECTION_PARAMETER)));
    for (unsigned int i = 0; i < this->paramSize; i++)
    {
        Napi::String name = paramNames.Get(i).ToString();
        Napi::String value = connectionParams.Get(name).ToString();
        //printf("\n%s: %s", &name.Utf8Value()[0], &value.Utf8Value()[0]);
        this->connectionParams[i].name = fillString(name);
        this->connectionParams[i].value = fillString(value);
    }

    this->__refId = ++Client::__refCounter;

    uv_sem_init(&this->invocationMutex, 1);
}

Client::~Client(void)
{
    RFC_INT isValid;
    RFC_ERROR_INFO errorInfo;

    this->alive = false;

    RFC_RC rc = RfcIsConnectionHandleValid(this->connectionHandle, &isValid, &errorInfo);
    if (rc == RFC_OK && isValid)
    {
        rc = RfcCloseConnection(this->connectionHandle, &errorInfo);
        if (rc != RFC_OK)
        {
            printf("Error closing connection: %d", rc);
        }
    }

    for (unsigned int i = 0; i < this->paramSize; i++)
    {
        free(const_cast<SAP_UC *>(connectionParams[i].name));
        free(const_cast<SAP_UC *>(connectionParams[i].value));
    }
    free(connectionParams);
    uv_sem_destroy(&this->invocationMutex);

    __bcdFunction.Reset();
    __dateToABAP.Reset();
    __dateFromABAP.Reset();
    __timeToABAP.Reset();
    __timeFromABAP.Reset();
}

Napi::Object Client::Init(Napi::Env env, Napi::Object exports)
{
    Napi::HandleScope scope(env);

    Napi::Function t = DefineClass(env,
                                   "Client", {
                                                 InstanceAccessor("version", &Client::VersionGetter, nullptr),
                                                 InstanceAccessor("options", &Client::OptionsGetter, nullptr),
                                                 InstanceAccessor("id", &Client::IdGetter, nullptr),
                                                 InstanceAccessor("_connectionHandle", &Client::ConnectionHandleGetter, nullptr),
                                                 InstanceAccessor("runningRFCCalls", &Client::RunningCallsGetter, nullptr),
                                                 InstanceMethod("connectionInfo", &Client::ConnectionInfo),
                                                 InstanceMethod("connect", &Client::Connect),
                                                 InstanceMethod("invoke", &Client::Invoke),
                                                 InstanceMethod("ping", &Client::Ping),
                                                 InstanceMethod("close", &Client::Close),
                                                 InstanceMethod("reopen", &Client::Reopen),
                                                 InstanceMethod("isAlive", &Client::IsAlive),
                                             });

    constructor = Napi::Persistent(t);
    constructor.SuppressDestruct();

    exports.Set("Client", t);
    return exports;
}

Napi::Value Client::Connect(const Napi::CallbackInfo &info)
{
    if (info[0].IsUndefined())
    {
        Napi::TypeError::New(info.Env(), "First argument must be callback function").ThrowAsJavaScriptException();
    }
    if (!info[0].IsFunction())
    {
        Napi::TypeError::New(info.Env(), "First argument must be callback function").ThrowAsJavaScriptException();
    }

    Napi::Function callback = info[0].As<Napi::Function>();

    (new ConnectAsync(callback, this))->Queue();

    return info.Env().Undefined();
}

Napi::Value Client::Invoke(const Napi::CallbackInfo &info)
{
    Napi::Array notRequested = Napi::Array::New(info.Env());
    Napi::Value bcd;

    Napi::Function callback = info[2].As<Napi::Function>();

    if (info[3].IsObject())
    {
        Napi::Object options = info[3].ToObject();
        Napi::Array props = options.GetPropertyNames();
        for (unsigned int i = 0; i < props.Length(); i++)
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

unsigned int Client::LockMutex(RFC_CLIENT_STATE newstate)
{
    uv_sem_wait(&this->invocationMutex);

    LOG_LOCK_REQUEST(this->state, newstate);
    if (newstate == CLIENT_CLOSE && this->runningCalls > 0)
    {
        ; // Reject Close request during ongoing call
    }
    else
    {
        this->state = newstate;
    }
    LOG_LOCK_ACQUIRE(this->state);
    return this->runningCalls;
}

void Client::UnlockMutex(RFC_CLIENT_STATE newstate)
{
    LOG_UNLOCK_REQUEST(this->state, newstate);
    uv_sem_post(&this->invocationMutex);
    this->state = CLIENT_READY;
    LOG_UNLOCK_ACQUIRE(this->state);
}

Napi::Value Client::Close(const Napi::CallbackInfo &info)
{
    if (info[0].IsUndefined())
    {
        Napi::TypeError::New(info.Env(), "First argument must be callback function").ThrowAsJavaScriptException();
    }
    if (!info[0].IsFunction())
    {
        Napi::TypeError::New(info.Env(), "Callback function argument missing").ThrowAsJavaScriptException();
    }
    Napi::Function callback = info[0].As<Napi::Function>();

    (new CloseAsync(callback, this))->Queue();

    return info.Env().Undefined();
}

Napi::Value Client::Ping(const Napi::CallbackInfo &info)
{

    if (info[0].IsUndefined())
    {
        Napi::TypeError::New(info.Env(), "First argument must be callback function").ThrowAsJavaScriptException();
    }
    if (!info[0].IsFunction())
    {
        Napi::TypeError::New(info.Env(), "Callback function argument missing").ThrowAsJavaScriptException();
    }
    Napi::Function callback = info[0].As<Napi::Function>();

    (new PingAsync(callback, this))->Queue();

    return info.Env().Undefined();
}

Napi::Value Client::Reopen(const Napi::CallbackInfo &info)
{
    if (info[0].IsUndefined())
    {
        Napi::TypeError::New(info.Env(), "First argument must be callback function").ThrowAsJavaScriptException();
    }
    if (!info[0].IsFunction())
    {
        Napi::TypeError::New(info.Env(), "Callback function argument missing").ThrowAsJavaScriptException();
    }
    Napi::Function callback = info[0].As<Napi::Function>();

    (new ReopenAsync(callback, this))->Queue();

    return info.Env().Undefined();
}

Napi::Value Client::ConnectionInfo(const Napi::CallbackInfo &info)
{
    RFC_RC rc;
    RFC_ERROR_INFO errorInfo;
    RFC_ATTRIBUTES connInfo;

    Napi::Env env = info.Env();
    Napi::Object infoObj = Napi::Object::New(env);
    rc = RfcGetConnectionAttributes(this->connectionHandle, &connInfo, &errorInfo);

    if (!this->alive)
    {
        return infoObj;
    }

    if (rc != RFC_OK || errorInfo.code != RFC_OK)
    {
        return wrapError(&errorInfo);
    }

    infoObj.Set(Napi::String::New(env, "dest"), wrapString(connInfo.dest, 64));
    infoObj.Set(Napi::String::New(env, "host"), wrapString(connInfo.host, 100));
    infoObj.Set(Napi::String::New(env, "partnerHost"), wrapString(connInfo.partnerHost, 100));
    infoObj.Set(Napi::String::New(env, "sysNumber"), wrapString(connInfo.sysNumber, 2));
    infoObj.Set(Napi::String::New(env, "sysId"), wrapString(connInfo.sysId, 8));
    infoObj.Set(Napi::String::New(env, "client"), wrapString(connInfo.client, 3));
    infoObj.Set(Napi::String::New(env, "user"), wrapString(connInfo.user, 12));
    infoObj.Set(Napi::String::New(env, "language"), wrapString(connInfo.language, 2));
    infoObj.Set(Napi::String::New(env, "trace"), wrapString(connInfo.trace, 1));
    infoObj.Set(Napi::String::New(env, "isoLanguage"), wrapString(connInfo.isoLanguage, 2));
    infoObj.Set(Napi::String::New(env, "codepage"), wrapString(connInfo.codepage, 4));
    infoObj.Set(Napi::String::New(env, "partnerCodepage"), wrapString(connInfo.partnerCodepage, 4));
    infoObj.Set(Napi::String::New(env, "rfcRole"), wrapString(connInfo.rfcRole, 1));
    infoObj.Set(Napi::String::New(env, "type"), wrapString(connInfo.type, 1));
    infoObj.Set(Napi::String::New(env, "partnerType"), wrapString(connInfo.partnerType, 1));
    infoObj.Set(Napi::String::New(env, "rel"), wrapString(connInfo.rel, 4));
    infoObj.Set(Napi::String::New(env, "partnerRel"), wrapString(connInfo.partnerRel, 4));
    infoObj.Set(Napi::String::New(env, "kernelRel"), wrapString(connInfo.kernelRel, 4));
    infoObj.Set(Napi::String::New(env, "cpicConvId"), wrapString(connInfo.cpicConvId, 8));
    infoObj.Set(Napi::String::New(env, "progName"), wrapString(connInfo.progName, 128));
    infoObj.Set(Napi::String::New(env, "partnerBytesPerChar"), wrapString(connInfo.partnerBytesPerChar, 1));
    infoObj.Set(Napi::String::New(env, "partnerSystemCodepage"), wrapString(connInfo.partnerSystemCodepage, 4));
    infoObj.Set(Napi::String::New(env, "partnerIP"), wrapString(connInfo.partnerIP, 15));
    infoObj.Set(Napi::String::New(env, "partnerIPv6"), wrapString(connInfo.partnerIP, 45));
    // infoObj.Set(Napi::String::New(env, "reserved"), wrapString(connInfo.reserved, 17));

    return infoObj;
}

Napi::Value Client::IsAlive(const Napi::CallbackInfo &info)
{
    return Napi::Boolean::New(info.Env(), this->alive);
}

Napi::Value Client::IdGetter(const Napi::CallbackInfo &info)
{
    return Napi::Number::New(info.Env(), this->__refId);
}

Napi::Value Client::RunningCallsGetter(const Napi::CallbackInfo &info)
{
    return Napi::Number::New(info.Env(), this->runningCalls);
}

Napi::Value Client::ConnectionHandleGetter(const Napi::CallbackInfo &info)
{
    if (!this->alive)
        return info.Env().Undefined();
    return Napi::Number::New(info.Env(), static_cast<double>((uint64_t)this->connectionHandle));
}

Napi::Value Client::VersionGetter(const Napi::CallbackInfo &info)
{
    unsigned major, minor, patchLevel;

    RfcGetVersion(&major, &minor, &patchLevel);

    Napi::Object version = Napi::Object::New(node_rfc::__env);
    version.Set(Napi::String::New(node_rfc::__env, "major"), major);
    version.Set(Napi::String::New(node_rfc::__env, "minor"), minor);
    version.Set(Napi::String::New(node_rfc::__env, "patchLevel"), patchLevel);
    version.Set(Napi::String::New(node_rfc::__env, "binding"), Napi::String::New(node_rfc::__env, SAPNWRFC_BINDING_VERSION));
    return version;
}

Napi::Value Client::OptionsGetter(const Napi::CallbackInfo &info)
{
    Napi::Object options = Napi::Object::New(node_rfc::__env);
    if (__bcd == NODERFC_BCD_STRING)
    {
        options.Set(Napi::String::New(node_rfc::__env, "bcd"), Napi::String::New(node_rfc::__env, "string"));
    }
    else if (__bcd == NODERFC_BCD_NUMBER)
    {
        options.Set(Napi::String::New(node_rfc::__env, "bcd"), Napi::String::New(node_rfc::__env, "number"));
    }
    else if (__bcd == NODERFC_BCD_FUNCTION)
    {
        options.Set(Napi::String::New(node_rfc::__env, "bcd"), __bcdFunction.Value());
    }
    else
    {
        options.Set(Napi::String::New(node_rfc::__env, "bcd"), Napi::String::New(node_rfc::__env, "?"));
    }

    Napi::Object date = Napi::Object::New(node_rfc::__env);
    if (!__dateToABAP.IsEmpty())
    {
        date.Set(Napi::String::New(node_rfc::__env, "toABAP"), __dateToABAP.Value());
    }
    if (!__dateFromABAP.IsEmpty())
    {
        date.Set(Napi::String::New(node_rfc::__env, "fromABAP"), __dateFromABAP.Value());
    }
    options.Set(Napi::String::New(node_rfc::__env, "date"), date);

    Napi::Object time = Napi::Object::New(node_rfc::__env);
    if (!__timeToABAP.IsEmpty())
    {
        time.Set(Napi::String::New(node_rfc::__env, "toABAP"), __timeToABAP.Value());
    }
    if (!__timeFromABAP.IsEmpty())
    {
        time.Set(Napi::String::New(node_rfc::__env, "fromABAP"), __timeFromABAP.Value());
    }
    options.Set(Napi::String::New(node_rfc::__env, "time"), time);

    return options;
}

} // namespace node_rfc

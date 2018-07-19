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

#include "client.h"
#include "rfcio.h"
#include "error.h"
#include "macros.h"

namespace node_rfc
{

Napi::Env __genv = NULL;

unsigned int Client::__refCounter = 0;

class ConnectAsync : public Napi::AsyncWorker
{
  public:
    ConnectAsync(Napi::Function &callback, Client *client)
        : Napi::AsyncWorker(callback), client(client) {}
    ~ConnectAsync() {}

    void Execute()
    {
        client->connectionHandle = RfcOpenConnection(client->connectionParams, client->paramSize, &client->errorInfo);
    }

    void OnOK()
    {
        client->alive = (client->errorInfo.code == RFC_OK);

        if (!client->alive)
        {
            Napi::Value argv[1] = {wrapError(&client->errorInfo)};
            TRY_CATCH_CALL(Env().Global(), Callback(), 1, argv);
        }
        else
        {
            TRY_CATCH_CALL(Env().Global(), Callback(), 0, {});
        }
    }

  private:
    Client *client;
}; // namespace node_rfc

class ExecuteAsync : public Napi::AsyncWorker
{
  private:
    Napi::FunctionReference callback;
    Client *client;
    RFC_FUNCTION_HANDLE functionHandle;
    RFC_FUNCTION_DESC_HANDLE functionDescHandle;

  public:
    ExecuteAsync(Napi::Function &callback, Client *client, RFC_FUNCTION_HANDLE functionHandle, RFC_FUNCTION_DESC_HANDLE functionDescHandle)
        : Napi::AsyncWorker(callback), callback(Napi::Persistent(callback)),
          client(client), functionHandle(functionHandle), functionDescHandle(functionDescHandle)
    {
    }
    ~ExecuteAsync() {}

    void Execute()
    {
        RfcInvoke(client->connectionHandle, functionHandle, &client->errorInfo);

        client->UnlockMutex();
    }

    void OnOK()
    {
        Napi::Value argv[2] = {Env().Undefined(), Env().Undefined()};

        if (client->errorInfo.code != RFC_OK)
        {
            argv[0] = wrapError(&client->errorInfo);
        }
        else
        {
            client->alive = true;
            argv[1] = wrapResult(functionDescHandle, functionHandle, Env());
        }
        RfcDestroyFunction(functionHandle, NULL);
        TRY_CATCH_CALL(Env().Global(), callback, 2, argv)
        callback.Reset();
    }
};

class PrepareAsync : public Napi::AsyncWorker
{
  private:
    Napi::FunctionReference callback;
    Client *client;
    SAP_UC *funcName;

    Napi::Reference<Napi::Array> notRequested;
    Napi::Reference<Napi::Object> parameters;

    RFC_FUNCTION_DESC_HANDLE functionDescHandle;

  public:
    PrepareAsync(Napi::Function &callback, Client *client,
                 Napi::String rfmName, Napi::Array &notRequestedParameters, Napi::Object &Parameters)
        : Napi::AsyncWorker(callback),
          callback(Napi::Persistent(callback)), client(client),
          notRequested(Napi::Persistent(notRequestedParameters)), parameters(Napi::Persistent(Parameters))
    {
        funcName = fillString(rfmName);
    }
    ~PrepareAsync() {}

    void Execute()
    {
        functionDescHandle = RfcGetFunctionDesc(client->connectionHandle, funcName, &client->errorInfo);
        free(funcName);
    }

    void OnOK()
    {
        RFC_FUNCTION_HANDLE functionHandle = NULL;
        Napi::Value argv[2] = {Env().Undefined(), Env().Undefined()};

        if (functionDescHandle == NULL || client->errorInfo.code != RFC_OK)
            argv[0] = wrapError(&client->errorInfo);

        if (argv[0].IsUndefined())
        {

            client->LockMutex();

            functionHandle = RfcCreateFunction(functionDescHandle, &client->errorInfo);
            RFC_RC rc;

            for (unsigned int i = 0; i < notRequested.Value().Length(); i++)
            {
                Napi::String name = notRequested.Value().Get(i).ToString();
                SAP_UC *paramName = fillString(name);
                rc = RfcSetParameterActive(functionHandle, paramName, 0, &client->errorInfo);
                free(const_cast<SAP_UC *>(paramName));
                if (rc != RFC_OK)
                {
                    argv[0] = wrapError(&client->errorInfo);
                    break;
                }
            }
        }

        notRequested.Reset();

        if (argv[0].IsUndefined())
        {

            Napi::Object params = parameters.Value();
            Napi::Array paramNames = params.GetPropertyNames();
            unsigned int paramSize = paramNames.Length();

            for (unsigned int i = 0; i < paramSize; i++)
            {
                Napi::String name = paramNames.Get(i).ToString();
                Napi::Value value = params.Get(name);
                argv[0] = fillFunctionParameter(functionDescHandle, functionHandle, name, value);
                if (!argv[0].IsUndefined())
                {
                    break;
                }
            }
        }

        parameters.Reset();

        if (argv[0].IsUndefined())
        {
            Napi::Function callbackFunction = callback.Value();
            (new ExecuteAsync(callbackFunction, client, functionHandle, functionDescHandle))->Queue();
        }
        else
        {
            client->UnlockMutex();
            TRY_CATCH_CALL(Env().Global(), callback, 1, argv);
            callback.Reset();
        }
    }
};

Napi::FunctionReference Client::constructor;

Client::Client(const Napi::CallbackInfo &info) : Napi::ObjectWrap<Client>(info)
{
    init();

    node_rfc::__genv = info.Env();

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

    if (info.Length() > 1)
    {
        if (!info[1].IsBoolean())
        {
            Napi::TypeError::New(info.Env(), "The 'rstrip' parameter must be true or false").ThrowAsJavaScriptException();
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
        //printf("\n%s: %s\n", &name.Utf8Value()[0], &value.Utf8Value()[0]);
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
}

Napi::Object Client::Init(Napi::Env env, Napi::Object exports)
{
    Napi::HandleScope scope(env);

    Napi::Function t = DefineClass(env,
                                   "Client", {
                                                 InstanceAccessor("version", &Client::VersionGetter, nullptr),
                                                 InstanceAccessor("id", &Client::IdGetter, nullptr),
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

    Napi::Function callback = info[2].As<Napi::Function>();

    if (info[3].IsObject())
    {
        Napi::Object obj = info[3].ToObject();
        Napi::Array props = obj.GetPropertyNames();
        for (unsigned int i = 0; i < props.Length(); i++)
        {
            Napi::String key = props.Get(i).ToString();
            if (key.Utf8Value().compare(std::string("notRequested")) == (int)0)
            {
                notRequested = obj.Get(key).As<Napi::Array>();
            }
        }
    }

    Napi::String rfmName = info[0].As<Napi::String>();
    Napi::Object Parameters = info[1].As<Napi::Object>();

    (new PrepareAsync(callback, this, rfmName, notRequested, Parameters))->Queue();

    return info.Env().Undefined();
} // namespace node_rfc

void Client::LockMutex(void)
{
    uv_sem_wait(&this->invocationMutex);
}

void Client::UnlockMutex(void)
{
    uv_sem_post(&this->invocationMutex);
}

Napi::Value Client::Close(const Napi::CallbackInfo &info)
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
            return wrapError(&errorInfo);
        }
    }

    return info.Env().Undefined();
}

Napi::Value Client::ConnectionInfo(const Napi::CallbackInfo &info)
{
    RFC_RC rc;
    RFC_ERROR_INFO errorInfo;
    RFC_ATTRIBUTES connInfo;
    RFC_INT isValid;

    Client *client = this;
    Napi::Env env = info.Env();
    Napi::Object infoObj = Napi::Object::New(env);

    rc = RfcIsConnectionHandleValid(this->connectionHandle, &isValid, &errorInfo);
    if (rc == RFC_OK && isValid)
    {
        rc = RfcGetConnectionAttributes(client->connectionHandle, &connInfo, &errorInfo);

        if (rc != RFC_OK)
        {
            return wrapError(&errorInfo);
        }

        infoObj.Set(Napi::String::New(env, "host"), wrapString(connInfo.host, 100));
        infoObj.Set(Napi::String::New(env, "partnerHost"), wrapString(connInfo.partnerHost, 100));
        infoObj.Set(Napi::String::New(env, "sysNumber"), wrapString(connInfo.sysNumber, 2));
        infoObj.Set(Napi::String::New(env, "sysId"), wrapString(connInfo.sysId, 8));
        infoObj.Set(Napi::String::New(env, "client"), wrapString(connInfo.client, 3));
        infoObj.Set(Napi::String::New(env, "user"), wrapString(connInfo.user, 8));
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
        infoObj.Set(Napi::String::New(env, "reserved"), wrapString(connInfo.reserved, 84));
    }

    return infoObj;
}

Napi::Value Client::Ping(const Napi::CallbackInfo &info)
{
    RFC_RC rc;
    RFC_ERROR_INFO errorInfo;
    rc = RfcPing(this->connectionHandle, &errorInfo);
    if (rc != RFC_OK)
    {
        return Napi::Boolean::New(info.Env(), FALSE);
    }
    else
    {
        return Napi::Boolean::New(info.Env(), TRUE);
    }
}

Napi::Value Client::Reopen(const Napi::CallbackInfo &info)
{
    this->Close(info);
    return this->Connect(info);
}

Napi::Value Client::IsAlive(const Napi::CallbackInfo &info)
{
    return Napi::Boolean::New(info.Env(), this->alive);
}

Napi::Value Client::IdGetter(const Napi::CallbackInfo &info)
{
    return Napi::Number::New(info.Env(), this->__refId);
}

Napi::Value Client::VersionGetter(const Napi::CallbackInfo &info)
{
    unsigned major, minor, patchLevel;
    Napi::Env env = info.Env();

    RfcGetVersion(&major, &minor, &patchLevel);

    Napi::Object version = Napi::Object::New(env);
    version.Set(Napi::String::New(env, "major"), major);
    version.Set(Napi::String::New(env, "minor"), minor);
    version.Set(Napi::String::New(env, "patchLevel"), patchLevel);
    version.Set(Napi::String::New(env, "binding"), Napi::String::New(env, SAPNWRFC_BINDING_VERSION));
    return version;
}

} // namespace node_rfc
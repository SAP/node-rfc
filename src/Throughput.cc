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

#include "Throughput.h"
#include "Client.h"
#include "macros.h"
#include "noderfcsdk.h"

namespace node_rfc
{

unsigned int Throughput::__refCounter = 0;
extern Napi::Env __env;

Napi::FunctionReference Throughput::constructor;

Throughput::Throughput(const Napi::CallbackInfo &info) : Napi::ObjectWrap<Throughput>(info)
{
    if (!info.IsConstructCall())
    {
        Napi::Error::New(info.Env(), "Use the new operator to create instances of Rfc  Throughput.").ThrowAsJavaScriptException();
    }
    init(info.Env());
    RFC_ERROR_INFO errorInfo;
    this->__handle = RfcCreateThroughput(&errorInfo);
    if (errorInfo.code != RFC_OK)
        Napi::Error::New(info.Env(), "node-rfc internal error: Throughput create failed.\nCheck if SAP NWRFC SDK version >= 7.53").ThrowAsJavaScriptException();

    this->__refId = ++Throughput::__refCounter;
}

Throughput::~Throughput(void)
{
    RFC_ERROR_INFO errorInfo;

    if (this->__handle != NULL)
    {
        RfcDestroyThroughput(this->__handle, &errorInfo);
        //if (errorInfo.code != RFC_OK) ...
        this->__handle = NULL;
    }
}

Napi::Object Throughput::Init(Napi::Env env, Napi::Object exports)
{
    Napi::HandleScope scope(env);

    Napi::Function t = DefineClass(
        env, "Throughput",
        {
            InstanceAccessor("status", &Throughput::StatusGetter, nullptr),
            InstanceAccessor("id", &Throughput::IdGetter, nullptr),
            InstanceAccessor("_handle", &Throughput::HandleGetter, nullptr),
            InstanceMethod("setOnConnection", &Throughput::SetOnConnection),
            InstanceMethod("removeFromConnection", &Throughput::RemoveFromConnection),
            StaticMethod("getFromConnection", &Throughput::GetFromConnection),
            InstanceMethod("reset", &Throughput::Reset),
            InstanceMethod("destroy", &Throughput::Destroy),
        });

    constructor = Napi::Persistent(t);
    constructor.SuppressDestruct();

    exports.Set("Throughput", t);
    return exports;
}

Napi::Value Throughput::Reset(const Napi::CallbackInfo &info)
{
    Napi::EscapableHandleScope scope(info.Env());
    RFC_ERROR_INFO errorInfo;
    RFC_RC rc = RfcResetThroughput(this->__handle, &errorInfo);
    if (rc != RFC_OK)
        return scope.Escape(wrapError(&errorInfo));
    return info.Env().Undefined();
}

Napi::Value Throughput::Destroy(const Napi::CallbackInfo &info)
{
    Napi::EscapableHandleScope scope(info.Env());
    RFC_ERROR_INFO errorInfo;
    if (this->__handle != NULL)
    {
        RFC_RC rc = RfcDestroyThroughput(this->__handle, &errorInfo);
        this->__handle = NULL;
        if (rc != RFC_OK)
            return scope.Escape(wrapError(&errorInfo));
    }
    return info.Env().Undefined();
}

Napi::Value Throughput::IdGetter(const Napi::CallbackInfo &info)
{
    return Napi::Number::New(info.Env(), this->__refId);
}

Napi::Value Throughput::HandleGetter(const Napi::CallbackInfo &info)
{
    return Napi::Number::New(info.Env(), static_cast<double>((uint64_t)this->__handle));
}

Napi::Value Throughput::StatusGetter(const Napi::CallbackInfo &info)
{
    Napi::EscapableHandleScope scope(info.Env());

    RFC_ERROR_INFO errorInfo;
    RFC_RC rc;
    SAP_ULLONG numberOfCalls;
    SAP_ULLONG sentBytes;
    SAP_ULLONG receivedBytes;
    SAP_ULLONG applicationTime;
    SAP_ULLONG totalTime;
    SAP_ULLONG serializationTime;
    SAP_ULLONG deserializationTime;

    Napi::Object status = Napi::Object::New(info.Env());

    if (this->__handle == NULL)
        Napi::Error::New(info.Env(), "node-rfc internal error: Throughput without handle!").ThrowAsJavaScriptException();

    THROUGHPUT_CALL(NumberOfCalls, numberOfCalls);
    THROUGHPUT_CALL(SentBytes, sentBytes);
    THROUGHPUT_CALL(ReceivedBytes, receivedBytes);
    THROUGHPUT_CALL(ApplicationTime, applicationTime);
    THROUGHPUT_CALL(TotalTime, totalTime);
    THROUGHPUT_CALL(SerializationTime, serializationTime);
    THROUGHPUT_CALL(DeserializationTime, deserializationTime);

    return scope.Escape(status);
}

Napi::Value Throughput::SetOnConnection(const Napi::CallbackInfo &info)
{
    Napi::EscapableHandleScope scope(info.Env());

    if (info.Length() != 1)
    {
        Napi::Error::New(info.Env(), "Connection instance argument missing").ThrowAsJavaScriptException();
    }
    if (!info[0].IsNumber())
    {
        Napi::Error::New(info.Env(), "Connection handle required as argument").ThrowAsJavaScriptException();
    }
    const RFC_CONNECTION_HANDLE connectionHandle = (RFC_CONNECTION_HANDLE)info[0].As<Number>().Int64Value();
    RFC_ERROR_INFO errorInfo;
    RFC_RC rc = RfcSetThroughputOnConnection(connectionHandle, this->__handle, &errorInfo);
    if (rc != RFC_OK)
    {
        return scope.Escape(wrapError(&errorInfo));
    }

    return info.Env().Undefined();
}

Napi::Value Throughput::RemoveFromConnection(const Napi::CallbackInfo &info)
{
    Napi::EscapableHandleScope scope(info.Env());

    if (info.Length() != 1)
    {
        Napi::Error::New(info.Env(), "Connection instance argument missing").ThrowAsJavaScriptException();
    }
    if (!info[0].IsNumber())
    {
        Napi::Error::New(info.Env(), "Connection handle required as argument").ThrowAsJavaScriptException();
    }
    const RFC_CONNECTION_HANDLE connectionHandle = (RFC_CONNECTION_HANDLE)info[0].As<Number>().Int64Value();
    RFC_ERROR_INFO errorInfo;
    RFC_RC rc = RfcRemoveThroughputFromConnection(connectionHandle, &errorInfo);
    if (rc != RFC_OK)
    {
        return scope.Escape(wrapError(&errorInfo));
    }

    return info.Env().Undefined();
}

Napi::Value Throughput::GetFromConnection(const Napi::CallbackInfo &info)
{
    Napi::EscapableHandleScope scope(info.Env());

    if (info.Length() != 1)
    {
        Napi::Error::New(info.Env(), "Connection instance argument missing").ThrowAsJavaScriptException();
    }
    if (!info[0].IsNumber())
    {
        Napi::Error::New(info.Env(), "Connection handle required as argument").ThrowAsJavaScriptException();
    }
    const RFC_CONNECTION_HANDLE connectionHandle = (RFC_CONNECTION_HANDLE)info[0].As<Number>().Int64Value();
    RFC_ERROR_INFO errorInfo;
    RFC_THROUGHPUT_HANDLE throughputHandle = RfcGetThroughputFromConnection(connectionHandle, &errorInfo);
    if (errorInfo.code != RFC_OK)
    {
        return scope.Escape(wrapError(&errorInfo));
    }
    return Napi::Number::New(info.Env(), static_cast<double>((uint64_t)throughputHandle));
}

} // namespace node_rfc

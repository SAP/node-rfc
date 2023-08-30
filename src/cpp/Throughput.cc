// SPDX-FileCopyrightText: 2014 SAP SE Srdjan Boskovic <srdjan.boskovic@sap.com>
//
// SPDX-License-Identifier: Apache-2.0

#include "Throughput.h"
#include "Client.h"
#include "nwrfcsdk.h"

namespace node_rfc {
uint_t Throughput::_id = 0;

Napi::FunctionReference Throughput::constructor;

Throughput::Throughput(const Napi::CallbackInfo& info)
    : Napi::ObjectWrap<Throughput>(info) {
  if (!info.IsConstructCall()) {
    Napi::Error::New(
        info.Env(),
        "Use the new operator to create instances of Rfc Throughput.")
        .ThrowAsJavaScriptException();
  }
  RFC_ERROR_INFO errorInfo;
  this->throughput_handle = RfcCreateThroughput(&errorInfo);
  if (errorInfo.code != RFC_OK)
    Napi::Error::New(info.Env(),
                     "node-rfc internal error: Throughput create "
                     "failed.\nCheck if SAP NWRFC SDK version >= 7.53")
        .ThrowAsJavaScriptException();

  this->id = ++Throughput::_id;
}

Throughput::~Throughput(void) {
  RFC_ERROR_INFO errorInfo;

  if (this->throughput_handle != nullptr) {
    RfcDestroyThroughput(this->throughput_handle, &errorInfo);
    // if (errorInfo.code != RFC_OK) ...
    this->throughput_handle = nullptr;
  }
}

Napi::Object Throughput::Init(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);

  Napi::Function t = DefineClass(
      env,
      "Throughput",
      {
          InstanceAccessor("status", &Throughput::StatusGetter, nullptr),
          InstanceAccessor("id", &Throughput::IdGetter, nullptr),
          InstanceAccessor("_handle", &Throughput::HandleGetter, nullptr),
          InstanceMethod("setOnConnection", &Throughput::SetOnConnection),
          InstanceMethod("removeFromConnection",
                         &Throughput::RemoveFromConnection),
          StaticMethod("getFromConnection", &Throughput::GetFromConnection),
          InstanceMethod("reset", &Throughput::Reset),
          InstanceMethod("destroy", &Throughput::Destroy),
      });

  constructor = Napi::Persistent(t);
  constructor.SuppressDestruct();

  // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks)
  exports.Set("Throughput", t);
  return exports;
}

Napi::Value Throughput::Reset(const Napi::CallbackInfo& info) {
  Napi::EscapableHandleScope scope(info.Env());
  RFC_ERROR_INFO errorInfo;
  RFC_RC rc = RfcResetThroughput(this->throughput_handle, &errorInfo);
  if (rc != RFC_OK) return scope.Escape(rfcSdkError(&errorInfo));
  return info.Env().Undefined();
}

Napi::Value Throughput::Destroy(const Napi::CallbackInfo& info) {
  Napi::EscapableHandleScope scope(info.Env());
  RFC_ERROR_INFO errorInfo;
  if (this->throughput_handle != nullptr) {
    RFC_RC rc = RfcDestroyThroughput(this->throughput_handle, &errorInfo);
    this->throughput_handle = nullptr;
    if (rc != RFC_OK) return scope.Escape(rfcSdkError(&errorInfo));
  }
  return info.Env().Undefined();
}

Napi::Value Throughput::IdGetter(const Napi::CallbackInfo& info) {
  return Napi::Number::New(info.Env(), this->id);
}

Napi::Value Throughput::HandleGetter(const Napi::CallbackInfo& info) {
  return Napi::Number::New(
      info.Env(), static_cast<double>((uint64_t)this->throughput_handle));
}

Napi::Value Throughput::StatusGetter(const Napi::CallbackInfo& info) {
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

  if (this->throughput_handle == nullptr)
    Napi::Error::New(info.Env(),
                     "node-rfc internal error: Throughput without handle!")
        .ThrowAsJavaScriptException();

  THROUGHPUT_CALL(NumberOfCalls, numberOfCalls);
  THROUGHPUT_CALL(SentBytes, sentBytes);
  THROUGHPUT_CALL(ReceivedBytes, receivedBytes);
  THROUGHPUT_CALL(ApplicationTime, applicationTime);
  THROUGHPUT_CALL(TotalTime, totalTime);
  THROUGHPUT_CALL(SerializationTime, serializationTime);
  THROUGHPUT_CALL(DeserializationTime, deserializationTime);

  return scope.Escape(status);
}

Napi::Value Throughput::SetOnConnection(const Napi::CallbackInfo& info) {
  Napi::EscapableHandleScope scope(info.Env());

  if (info.Length() != 1) {
    Napi::Error::New(info.Env(), "Connection instance argument missing")
        .ThrowAsJavaScriptException();
  }
  if (!info[0].IsNumber()) {
    Napi::Error::New(info.Env(), "Connection handle required as argument")
        .ThrowAsJavaScriptException();
  }
  const RFC_CONNECTION_HANDLE connectionHandle =
      (RFC_CONNECTION_HANDLE)info[0].As<Number>().Int64Value();
  RFC_ERROR_INFO errorInfo;
  RFC_RC rc = RfcSetThroughputOnConnection(
      connectionHandle, this->throughput_handle, &errorInfo);
  if (rc != RFC_OK) {
    return scope.Escape(rfcSdkError(&errorInfo));
  }

  return info.Env().Undefined();
}

Napi::Value Throughput::RemoveFromConnection(const Napi::CallbackInfo& info) {
  Napi::EscapableHandleScope scope(info.Env());

  if (info.Length() != 1) {
    Napi::Error::New(info.Env(), "Connection instance argument missing")
        .ThrowAsJavaScriptException();
  }
  if (!info[0].IsNumber()) {
    Napi::Error::New(info.Env(), "Connection handle required as argument")
        .ThrowAsJavaScriptException();
  }
  const RFC_CONNECTION_HANDLE connectionHandle =
      (RFC_CONNECTION_HANDLE)info[0].As<Number>().Int64Value();
  RFC_ERROR_INFO errorInfo;
  RFC_RC rc = RfcRemoveThroughputFromConnection(connectionHandle, &errorInfo);
  if (rc != RFC_OK) {
    return scope.Escape(rfcSdkError(&errorInfo));
  }

  return info.Env().Undefined();
}

Napi::Value Throughput::GetFromConnection(const Napi::CallbackInfo& info) {
  Napi::EscapableHandleScope scope(info.Env());

  if (info.Length() != 1) {
    Napi::Error::New(info.Env(), "Connection instance argument missing")
        .ThrowAsJavaScriptException();
  }
  if (!info[0].IsNumber()) {
    Napi::Error::New(info.Env(), "Connection handle required as argument")
        .ThrowAsJavaScriptException();
  }
  const RFC_CONNECTION_HANDLE connectionHandle =
      (RFC_CONNECTION_HANDLE)info[0].As<Number>().Int64Value();
  RFC_ERROR_INFO errorInfo;
  RFC_THROUGHPUT_HANDLE throughputHandle =
      RfcGetThroughputFromConnection(connectionHandle, &errorInfo);
  if (errorInfo.code != RFC_OK) {
    return scope.Escape(rfcSdkError(&errorInfo));
  }
  return Napi::Number::New(info.Env(),
                           static_cast<double>((uint64_t)throughputHandle));
}

}  // namespace node_rfc

// SPDX-FileCopyrightText: 2014 SAP SE Srdjan Boskovic <srdjan.boskovic@sap.com>
//
// SPDX-License-Identifier: Apache-2.0

#ifndef NodeRfc_Throughput_H_
#define NodeRfc_Throughput_H_

#include "Log.h"
#include "nwrfcsdk.h"

using namespace Napi;

namespace node_rfc {
extern Napi::Env __env;
extern Log _log;

class Throughput : public Napi::ObjectWrap<Throughput> {
 public:
  static Napi::FunctionReference constructor;
  static Napi::Object Init(Napi::Env env, Napi::Object exports);
  // cppcheck-suppress noExplicitConstructor
  Throughput(const Napi::CallbackInfo& info);
  ~Throughput(void);

 private:
  static uint_t _id;
  uint_t id;
  Napi::Object _statusObj;

  // Throughput API

  Napi::Value IdGetter(const Napi::CallbackInfo& info);
  Napi::Value StatusGetter(const Napi::CallbackInfo& info);
  Napi::Value HandleGetter(const Napi::CallbackInfo& info);
  Napi::Value SetOnConnection(const Napi::CallbackInfo& info);
  Napi::Value RemoveFromConnection(const Napi::CallbackInfo& info);
  static Napi::Value GetFromConnection(const Napi::CallbackInfo& info);
  Napi::Value Reset(const Napi::CallbackInfo& info);
  Napi::Value Destroy(const Napi::CallbackInfo& info);

  // SAP NW RFC SDK
  RFC_THROUGHPUT_HANDLE throughput_handle;
};

}  // namespace node_rfc

#define THROUGHPUT_CALL(Property, property)                                    \
  rc = RfcGet##Property(this->throughput_handle, &property, &errorInfo);       \
  if (rc != RFC_OK) return scope.Escape(rfcSdkError(&errorInfo));              \
  status.Set(Napi::String::New(info.Env(), #property),                         \
             Napi::Number::New(info.Env(), static_cast<double>(property)));

#endif  // NODE_SAPNWRFC_Throughput_H_

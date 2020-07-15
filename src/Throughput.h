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

#ifndef NodeRfc_Throughput_H_
#define NodeRfc_Throughput_H_

#include "nwrfcsdk.h"

using namespace Napi;

namespace node_rfc
{
    extern Napi::Env __env;
    class Throughput : public Napi::ObjectWrap<Throughput>
    {
    public:
        static Napi::FunctionReference constructor;
        static Napi::Object Init(Napi::Env env, Napi::Object exports);

        void init(Napi::Env env)
        {
            node_rfc::__env = env;
        };

        Throughput(const Napi::CallbackInfo &info);
        ~Throughput(void);

    private:
        static uint_t _id;
        uint_t id;
        Napi::Object _statusObj;

        // Throughput API

        Napi::Value IdGetter(const Napi::CallbackInfo &info);
        Napi::Value StatusGetter(const Napi::CallbackInfo &info);
        Napi::Value HandleGetter(const Napi::CallbackInfo &info);
        Napi::Value SetOnConnection(const Napi::CallbackInfo &info);
        Napi::Value RemoveFromConnection(const Napi::CallbackInfo &info);
        static Napi::Value GetFromConnection(const Napi::CallbackInfo &info);
        Napi::Value Reset(const Napi::CallbackInfo &info);
        Napi::Value Destroy(const Napi::CallbackInfo &info);

        // SAP NW RFC SDK
        RFC_THROUGHPUT_HANDLE throughput_handle;
    };

} // namespace node_rfc

#define THROUGHPUT_CALL(Property, property)                                \
    rc = RfcGet##Property(this->throughput_handle, &property, &errorInfo); \
    if (rc != RFC_OK)                                                      \
        return scope.Escape(wrapError(&errorInfo));                        \
    status.Set(Napi::String::New(info.Env(), #property), Napi::Number::New(info.Env(), static_cast<double>(property)));

#endif // NODE_SAPNWRFC_Throughput_H_

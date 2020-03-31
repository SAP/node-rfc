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

#ifndef NODE_SAPNWRFC_THROUGHPUT_H_
#define NODE_SAPNWRFC_THROUGHPUT_H_

#include <napi.h>
#include <sapnwrfc.h>

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
    static unsigned int __refCounter;
    unsigned int __refId;
    Napi::Object __statusObj;

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
    RFC_THROUGHPUT_HANDLE __handle;
};

} // namespace node_rfc

#endif // NODE_SAPNWRFC_Throughput_H_

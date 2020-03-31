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
#ifndef NODE_SAPNWRFC_MACROS_H
#define NODE_SAPNWRFC_MACROS_H

#define DEFINE_CONSTANT_STRING(target, constant, name)                       \
    Napi::PropertyDescriptor::Value(#name, Napi::String::New(env, constant), \
                                    static_cast<napi_property_attributes>(napi_enumerable | napi_configurable))

#define CALLBACK_CALL(context, callback, argc, argv)  \
    Napi::Value *passed_argv = argv;                  \
    std::vector<napi_value> args;                     \
    if ((argc != 0) && (passed_argv != NULL))         \
    {                                                 \
        args.assign(passed_argv, passed_argv + argc); \
    }                                                 \
    (callback).Call(context, args);

#define THROUGHPUT_CALL(Property, property)                       \
    rc = RfcGet##Property(this->__handle, &property, &errorInfo); \
    if (rc != RFC_OK)                                             \
        return scope.Escape(wrapError(&errorInfo));               \
    status.Set(Napi::String::New(info.Env(), #property), Napi::Number::New(info.Env(), property));

#endif

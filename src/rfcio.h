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

#ifndef RFCIO_H
#define RFCIO_H

#include <nan.h>
#include <node.h>
#include <sapnwrfc.h>


using namespace v8;


std::string convertToString(v8::Handle<v8::Value> const &str);
SAP_UC* fillString(const Handle<Value> &str);
Handle<Value> fillFunctionParameter(RFC_FUNCTION_DESC_HANDLE functionDescHandle, RFC_FUNCTION_HANDLE functionHandle, Local<Value> name, Local<Value> value);
Local<Value> fillVariable(RFCTYPE type, RFC_FUNCTION_HANDLE functionHandle, SAP_UC* cName, Handle<Value> value, RFC_TYPE_DESC_HANDLE functionDescHandle);

Handle<Object> wrapResult(RFC_FUNCTION_DESC_HANDLE functionDescHandle, RFC_FUNCTION_HANDLE functionHandle, bool rstrip=false);
Handle<Value> wrapStructure(RFC_TYPE_DESC_HANDLE typeDesc, RFC_STRUCTURE_HANDLE structHandle, bool rstrip=false);
Handle<Value> wrapString(SAP_UC* uc, int length=-1, bool rstrip=false);
Handle<Value> wrapVariable(RFCTYPE type, RFC_FUNCTION_HANDLE container, SAP_UC* cName, unsigned int cLen, RFC_TYPE_DESC_HANDLE typeDesc, bool rstrip=false);

#endif

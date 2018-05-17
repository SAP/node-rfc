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

#ifndef NODE_SAPNWRFC_RFCIO_H_
#define NODE_SAPNWRFC_RFCIO_H_

#include <napi.h>
#include <sapnwrfc.h>

using namespace Napi;

namespace node_rfc
{

SAP_UC *fillString(const Napi::String napistr);
Napi::Value fillFunctionParameter(RFC_FUNCTION_DESC_HANDLE functionDescHandle, RFC_FUNCTION_HANDLE functionHandle, Napi::String name, Napi::Value value);
Napi::Value fillVariable(RFCTYPE typ, RFC_FUNCTION_HANDLE functionHandle, SAP_UC *cName, Napi::Value value, RFC_TYPE_DESC_HANDLE functionDescHandle);

Napi::Value wrapString(SAP_UC *uc, int length = -1, bool rstrip = true);
Napi::Value wrapStructure(RFC_TYPE_DESC_HANDLE typeDesc, RFC_STRUCTURE_HANDLE structHandle, bool rstrip = true);
Napi::Value wrapVariable(RFCTYPE typ, RFC_FUNCTION_HANDLE functionHandle, SAP_UC *cName, unsigned int cLen, RFC_TYPE_DESC_HANDLE typeDesc, bool rstrip = true);
Napi::Value wrapResult(RFC_FUNCTION_DESC_HANDLE functionDescHandle, RFC_FUNCTION_HANDLE functionHandle, bool rstrip = true);

} // namespace node_rfc

#endif
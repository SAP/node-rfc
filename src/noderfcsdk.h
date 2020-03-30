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

#ifndef NODE_SAPNWRFC_NODERFCSDK_H_
#define NODE_SAPNWRFC_NODERFCSDK_H_

#include <napi.h>
#include <sapnwrfc.h>
using namespace Napi;

namespace node_rfc
{
// SAP string wrapper, required for errors
Napi::Value wrapString(SAP_UC *uc, int length = -1);

// RFC ERRORS
Napi::Value RfcLibError(RFC_ERROR_INFO *errorInfo);
Napi::Value AbapError(RFC_ERROR_INFO *errorInfo);
Napi::Value wrapError(RFC_ERROR_INFO *errorInfo);
} // namespace node_rfc
#endif

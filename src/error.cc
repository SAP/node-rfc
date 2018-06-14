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

#include "rfcio.h"
#include "error.h"

using namespace Napi;

namespace node_rfc
{

extern Napi::Env __genv;

////////////////////////////////////////////////////////////////////////////////
// RFC ERRORS
////////////////////////////////////////////////////////////////////////////////

Napi::Value RfcLibError(RFC_ERROR_INFO *errorInfo)
{
    Napi::EscapableHandleScope scope(__genv);

    Napi::Object errorObj = Napi::Object::New(__genv);
    (errorObj).Set(Napi::String::New(__genv, "name"), "RfcLibError");
    (errorObj).Set(Napi::String::New(__genv, "code"), Napi::Number::New(__genv, errorInfo->code));
    (errorObj).Set(Napi::String::New(__genv, "key"), wrapString(errorInfo->key));
    (errorObj).Set(Napi::String::New(__genv, "message"), wrapString(errorInfo->message));

    return scope.Escape(errorObj);
}

Napi::Value AbapError(RFC_ERROR_INFO *errorInfo)
{
    Napi::EscapableHandleScope scope(__genv);

    Napi::Object errorObj = Napi::Object::New(__genv);
    (errorObj).Set(Napi::String::New(__genv, "name"), "ABAPError");
    (errorObj).Set(Napi::String::New(__genv, "code"), Napi::Number::New(__genv, errorInfo->code));
    (errorObj).Set(Napi::String::New(__genv, "key"), wrapString(errorInfo->key));
    (errorObj).Set(Napi::String::New(__genv, "message"), wrapString(errorInfo->message));
    (errorObj).Set(Napi::String::New(__genv, "abapMsgClass"), wrapString(errorInfo->abapMsgClass));
    (errorObj).Set(Napi::String::New(__genv, "abapMsgType"), wrapString(errorInfo->abapMsgType));
    (errorObj).Set(Napi::String::New(__genv, "abapMsgNumber"), wrapString(errorInfo->abapMsgNumber));
    (errorObj).Set(Napi::String::New(__genv, "abapMsgV1"), wrapString(errorInfo->abapMsgV1));
    (errorObj).Set(Napi::String::New(__genv, "abapMsgV2"), wrapString(errorInfo->abapMsgV2));
    (errorObj).Set(Napi::String::New(__genv, "abapMsgV3"), wrapString(errorInfo->abapMsgV3));
    (errorObj).Set(Napi::String::New(__genv, "abapMsgV4"), wrapString(errorInfo->abapMsgV4));

    return scope.Escape(errorObj);
}

Napi::Value wrapError(RFC_ERROR_INFO *errorInfo)
{
    Napi::EscapableHandleScope scope(__genv);

    char cBuf[256];

    switch (errorInfo->group)
    {
    case OK: // 0: should never happen
        Napi::Error::Fatal("Error handling invoked with the RFC error group OK", "node-rfc internal error");
        break;

    case LOGON_FAILURE:            // 3: Error message raised when logon fails
    case COMMUNICATION_FAILURE:    // 4: Problems with the network connection (or backend broke down and killed the connection)
    case EXTERNAL_RUNTIME_FAILURE: // 5: Problems in the RFC runtime of the external program (i.e "this" library)
        return scope.Escape(RfcLibError(errorInfo));
        break;

    case ABAP_APPLICATION_FAILURE:       // 1: ABAP Exception raised in ABAP function modules
    case ABAP_RUNTIME_FAILURE:           // 2: ABAP Message raised in ABAP function modules or in ABAP runtime of the backend (e.g Kernel)
    case EXTERNAL_APPLICATION_FAILURE:   // 6: Problems in the external program (e.g in the external server implementation)
    case EXTERNAL_AUTHORIZATION_FAILURE: // 7: Problems raised in the authorization check handler provided by the external server implementation
        return scope.Escape(AbapError(errorInfo));
        break;
    }

    // unknown error group
    sprintf(cBuf, "Unknown error group: %u", errorInfo->group);
    Napi::Error::Fatal(cBuf, "node-rfc internal error");
}

} // namespace node_rfc
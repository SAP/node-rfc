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

using namespace v8;


Local<Object> RfcLibError(RFC_ERROR_INFO* errorInfo) {
    Nan::EscapableHandleScope scope;

    Local<Value> e = Nan::Error(wrapString(errorInfo->message)->ToString());;
    Local<Object> errorObj = e->ToObject();

    Nan::Set(errorObj, Nan::New("code").ToLocalChecked(), Nan::New(errorInfo->code));
    Nan::Set(errorObj, Nan::New("key").ToLocalChecked(), wrapString(errorInfo->key));

    return scope.Escape(errorObj);
}


Local<Object> AbapError(RFC_ERROR_INFO* errorInfo) {
    Nan::EscapableHandleScope scope;

    Local<Value> e = Nan::Error(wrapString(errorInfo->message)->ToString());
    Local<Object> errorObj = e->ToObject();

    Nan::Set(errorObj, Nan::New("code").ToLocalChecked(), Nan::New(errorInfo->code));
    Nan::Set(errorObj, Nan::New("key").ToLocalChecked(), wrapString(errorInfo->key));
    Nan::Set(errorObj, Nan::New("message").ToLocalChecked(), wrapString(errorInfo->message));
    Nan::Set(errorObj, Nan::New("abapMsgClass").ToLocalChecked(), wrapString(errorInfo->abapMsgClass));
    Nan::Set(errorObj, Nan::New("abapMsgType").ToLocalChecked(), wrapString(errorInfo->abapMsgType));
    Nan::Set(errorObj, Nan::New("abapMsgNumber").ToLocalChecked(), wrapString(errorInfo->abapMsgNumber));
    Nan::Set(errorObj, Nan::New("abapMsgV1").ToLocalChecked(), wrapString(errorInfo->abapMsgV1));
    Nan::Set(errorObj, Nan::New("abapMsgV2").ToLocalChecked(), wrapString(errorInfo->abapMsgV2));
    Nan::Set(errorObj, Nan::New("abapMsgV3").ToLocalChecked(), wrapString(errorInfo->abapMsgV3));
    Nan::Set(errorObj, Nan::New("abapMsgV4").ToLocalChecked(), wrapString(errorInfo->abapMsgV4));

    return scope.Escape(errorObj);
}

Local<Value> wrapError(RFC_ERROR_INFO* errorInfo) {
    Nan::EscapableHandleScope scope;
    Local <Value> e;
    char cBuf[256];

    switch (errorInfo->group) {
    case OK:                              // 0: should never happen
      e = Nan::Error("node-rfc internal error: Error handling invoked with the RFC error group OK");
      return scope.Escape(e->ToObject());
      break;

    case LOGON_FAILURE:                   // 3: Error message raised when logon fails
    case COMMUNICATION_FAILURE:           // 4: Problems with the network connection (or backend broke down and killed the connection)
    case EXTERNAL_RUNTIME_FAILURE:        // 5: Problems in the RFC runtime of the external program (i.e "this" library)
      return scope.Escape(RfcLibError(errorInfo));
      break;

    case ABAP_APPLICATION_FAILURE:        // 1: ABAP Exception raised in ABAP function modules
    case ABAP_RUNTIME_FAILURE:            // 2: ABAP Message raised in ABAP function modules or in ABAP runtime of the backend (e.g Kernel)
    case EXTERNAL_APPLICATION_FAILURE:    // 6: Problems in the external program (e.g in the external server implementation)
    case EXTERNAL_AUTHORIZATION_FAILURE:  // 7: Problems raised in the authorization check handler provided by the external server implementation
      return scope.Escape(AbapError(errorInfo));
      break;
    }

    // unknown error group
    sprintf(cBuf, "node-rfc internal error: Unknown error group: %u", errorInfo->group);
    e = Nan::Error(cBuf);
    return scope.Escape(e->ToObject());
}


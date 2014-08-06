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

#include "error.h"
#include "wrappers.h"

using namespace v8;


Handle<Value> RfclibError(RFC_ERROR_INFO* errorInfo) {
  HandleScope scope;

  Local<Value> e = Exception::Error(wrapString(errorInfo->message)->ToString());
  Local<Object> errorObj = e->ToObject();
  errorObj->Set(String::New("code"), Integer::New(errorInfo->code));
  errorObj->Set(String::New("key"), wrapString(errorInfo->key));

  return scope.Close(errorObj);
}


Handle<Value> AbapError(RFC_ERROR_INFO* errorInfo) {
  HandleScope scope;

  Local<Value> e = Exception::Error(wrapString(errorInfo->message)->ToString());
  Local<Object> errorObj = e->ToObject();
  errorObj->Set(String::New("code"), Integer::New(errorInfo->code));
  errorObj->Set(String::New("key"), wrapString(errorInfo->key));
  errorObj->Set(String::New("message"), wrapString(errorInfo->message));
  errorObj->Set(String::New("abapMsgClass"), wrapString(errorInfo->abapMsgClass));
  errorObj->Set(String::New("abapMsgType"), wrapString(errorInfo->abapMsgType));
  errorObj->Set(String::New("abapMsgNumber"), wrapString(errorInfo->abapMsgNumber));
  errorObj->Set(String::New("abapMsgV1"), wrapString(errorInfo->abapMsgV1));
  errorObj->Set(String::New("abapMsgV2"), wrapString(errorInfo->abapMsgV2));
  errorObj->Set(String::New("abapMsgV3"), wrapString(errorInfo->abapMsgV3));
  errorObj->Set(String::New("abapMsgV4"), wrapString(errorInfo->abapMsgV4));
  return scope.Close(errorObj);
}

Handle<Value> wrapError(RFC_ERROR_INFO* errorInfo) {
  HandleScope scope;
  Handle<Object> errorObj;
  Handle<Value> e;

  switch (errorInfo->group) {
    case ABAP_APPLICATION_FAILURE:  // fall throu
    case ABAP_RUNTIME_FAILURE:
      return scope.Close(AbapError(errorInfo));

    case COMMUNICATION_FAILURE:  // fall throu
    case EXTERNAL_RUNTIME_FAILURE:
      return scope.Close(RfclibError(errorInfo));

    case LOGON_FAILURE:
      e = Exception::Error(wrapString(errorInfo->message)->ToString());
      break;

    default:
      return scope.Close(AbapError(errorInfo));
      //e = Exception::Error(String::New("NODE-RFC-ERROR: Unknown error group"));
  }
  return scope.Close(e->ToObject());
}

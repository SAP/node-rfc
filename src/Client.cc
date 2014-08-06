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

#include <string>
#include <iostream>
#include "wrappers.h"
#include "Client.h"


using namespace v8;
using namespace node;


Client::Client() :
  connectionHandle(NULL),
  connectionParams(NULL),
  paramSize(0),
  alive(false)
  { uv_mutex_init(&this->invocationMutex); }

Client::~Client() {
  RFC_RC rc;
  RFC_ERROR_INFO errorInfo;
  rc = RfcCloseConnection(this->connectionHandle, &errorInfo);
  this->alive = false;
  uv_mutex_destroy(&this->invocationMutex);
  for (unsigned int i = 0; i < this->paramSize; i++) {
     free(const_cast<SAP_UC*>(connectionParams[i].name));
     free(const_cast<SAP_UC*>(connectionParams[i].value));
  }
  free(connectionParams);
}

void Client::Init(Handle<Object> exports) {
  HandleScope scope;

  Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  tpl->SetClassName(String::NewSymbol("Client"));

  NODE_SET_PROTOTYPE_METHOD(tpl, "connect", Connect);
  NODE_SET_PROTOTYPE_METHOD(tpl, "close", Close);
  NODE_SET_PROTOTYPE_METHOD(tpl, "reopen", Reopen);
  NODE_SET_PROTOTYPE_METHOD(tpl, "isAlive", IsAlive);
  NODE_SET_PROTOTYPE_METHOD(tpl, "invoke", Invoke);
  NODE_SET_PROTOTYPE_METHOD(tpl, "getVersion", GetVersion);
  NODE_SET_PROTOTYPE_METHOD(tpl, "connectionInfo", ConnectionInfo);
  NODE_SET_PROTOTYPE_METHOD(tpl, "ping", Ping);

  exports->Set(String::NewSymbol("Client"), tpl->GetFunction());
}

Handle<Value> Client::New(const Arguments &args) {
  if (!args.IsConstructCall()) {
    return ThrowException(Exception::TypeError(
            String::New("Use the new operator to create instances of Rfc connection.")));
  }

  if (args.Length() < 1) {
    return ThrowException(String::New("Please provide connection parameters as argument"));
  }
  if (!args[0]->IsObject()) {
    return ThrowException(Exception::TypeError(
            String::New("Connection parameters must be an object")));
  }

  Client *wrapper = new Client();
  wrapper->Wrap(args.This());

  if (args.Length() > 1) {
    if (!args[1]->IsBoolean()) {
      return ThrowException(Exception::TypeError(
              String::New("Third parameter for 'rstrip' must be true or false")));
    }
    wrapper->rstrip = args[1]->BooleanValue();
  } else {
    wrapper->rstrip = true;
  }

  Local<Object> connectionParams = args[0]->ToObject();
  Local<Array> paramNames = connectionParams->GetPropertyNames();
  wrapper->paramSize = paramNames->Length();
  wrapper->connectionParams = static_cast<RFC_CONNECTION_PARAMETER*>(malloc(wrapper->paramSize * sizeof(RFC_CONNECTION_PARAMETER)));

  for (unsigned int i = 0; i < wrapper->paramSize; i++) {
    Local<Value> name = paramNames->Get(i);
    Local<Value> value = connectionParams->Get(name->ToString());

    wrapper->connectionParams[i].name = fillString(name);
    wrapper->connectionParams[i].value = fillString(value);
  }

  return args.This();
}


void Client::LockMutex(void) {
  uv_mutex_lock(&this->invocationMutex);
}

void Client::UnlockMutex(void) {
  uv_mutex_unlock(&this->invocationMutex);
}


void Client::ConnectAsync(uv_work_t* req) {
  ClientBaton* baton = static_cast<ClientBaton*>(req->data);

  baton->wrapper->connectionHandle = RfcOpenConnection(baton->wrapper->connectionParams, baton->wrapper->paramSize, &baton->errorInfo);
}

void Client::ConnectAsyncAfter(uv_work_t* req) {
  HandleScope scope;
  ClientBaton* baton = static_cast<ClientBaton*>(req->data);
  if (baton->errorInfo.code != RFC_OK) {
    Handle<Value> argv[] = { wrapError(&baton->errorInfo) };

    TryCatch try_catch;
    baton->callback->Call(Context::GetCurrent()->Global(), 1, argv);

    if (try_catch.HasCaught()) {
      node::FatalException(try_catch);
    }
  } else {
    // connect
    baton->callback->Call(Context::GetCurrent()->Global(), 0, NULL);
    baton->wrapper->alive = true;
  }

  baton->callback.Dispose();
  delete baton;
}

Handle<Value> Client::Connect(const Arguments& args) {
  HandleScope scope;

  if (!args[0]->IsFunction()) {
    return ThrowException(Exception::TypeError(
            String::New("First Argument must be callback function")));
  }

  Client *wrapper = Unwrap<Client>(args.This());

  Local<Function> callback = Local<Function>::Cast(args[0]);

  ClientBaton* baton = new ClientBaton();
  baton->request.data = baton;
  baton->callback = Persistent<Function>::New(callback);
  baton->wrapper = wrapper;

  uv_queue_work(uv_default_loop(), &baton->request, ConnectAsync, (uv_after_work_cb)ConnectAsyncAfter);

  return scope.Close(Undefined());
}

Handle<Value> Client::Close(const Arguments& args) {
  HandleScope scope;

  Client *wrapper = Unwrap<Client>(args.This());
  RFC_RC rc;
  RFC_ERROR_INFO errorInfo;
  rc = RfcCloseConnection(wrapper->connectionHandle, &errorInfo);
  wrapper->alive = false;
  if (rc != RFC_OK) {
    return scope.Close(wrapError(&errorInfo));
  }
  return scope.Close(Undefined());
}

Handle<Value> Client::Reopen(const Arguments& args) {
  HandleScope scope;
  Client *wrapper = Unwrap<Client>(args.This());

  wrapper->Close(args);
  return scope.Close(wrapper->Connect(args));
}

Handle<Value> Client::IsAlive(const Arguments& args) {
  HandleScope scope;
  Client *wrapper = Unwrap<Client>(args.This());
  return scope.Close(Boolean::New(wrapper->alive));
}


void Client::InvokeAsync(uv_work_t* req) {
  InvokeBaton* baton = static_cast<InvokeBaton*>(req->data);

  RfcInvoke(baton->wrapper->connectionHandle, baton->functionHandle, &baton->errorInfo);
  baton->wrapper->UnlockMutex();
}

void Client::InvokeAsyncAfter(uv_work_t* req) {
  HandleScope scope;
  InvokeBaton* baton = static_cast<InvokeBaton*>(req->data);

  Handle<Value> argv[2] = { Null(), Null() };

  if (baton->errorInfo.code != RFC_OK) {
    argv[0] = wrapError(&baton->errorInfo);

    TryCatch try_catch;
    baton->callback->Call(Context::GetCurrent()->Global(), 1, argv);
    if (try_catch.HasCaught()) {
      node::FatalException(try_catch);
    }
  } else {
    Handle<Object> result = wrapResult(baton->functionDescHandle, baton->functionHandle, baton->wrapper->rstrip);
    RfcDestroyFunction(baton->functionHandle, NULL);

    argv[1] = result;
    baton->callback->Call(Context::GetCurrent()->Global(), 2, argv);
  }

  baton->callback.Dispose();
  delete baton;
}

Handle<Value> Client::Invoke(const Arguments& args) {
  HandleScope scope;

  if (args.Length() < 3) {
    return ThrowException(String::New("Please provide function module, parameters and callback as parameters"));
  }
  if (!args[0]->IsString()) {
    return ThrowException(Exception::TypeError(
            String::New("First parameter (rfc function name) must be an string")));
  }
  if (!args[1]->IsObject()) {
    return ThrowException(Exception::TypeError(
            String::New("Second parameter (rfc function arguments) must be an object")));
  }
  if (!args[2]->IsFunction()) {
    return ThrowException(Exception::TypeError(
            String::New("Third Argument must be callback function")));
  }

  Client *wrapper = Unwrap<Client>(args.This());

  Local<Function> callback = Local<Function>::Cast(args[2]);

  InvokeBaton* baton = new InvokeBaton();
  baton->request.data = baton;
  baton->callback = Persistent<Function>::New(callback);
  baton->wrapper = wrapper;

  Handle<Value> argv[2] = { Null(), Null() };
  SAP_UC *funcName = fillString(args[0]);

  baton->wrapper->LockMutex();
  baton->functionDescHandle = RfcGetFunctionDesc(wrapper->connectionHandle, funcName, &baton->errorInfo);
  free(funcName);
  if (baton->functionDescHandle == NULL) {
    argv[0] = wrapError(&baton->errorInfo);
    baton->callback->Call(Context::GetCurrent()->Global(), 2, argv);
    delete baton;
    return scope.Close(Undefined());
  }
  baton->functionHandle = RfcCreateFunction(baton->functionDescHandle, &baton->errorInfo);

  Local<Object> params = args[1]->ToObject();
  Local<Array> paramNames = params->GetPropertyNames();
  unsigned int paramSize = paramNames->Length();

  for (unsigned int i = 0; i < paramSize; i++) {
    Local<Value> name = paramNames->Get(i);
    Local<Value> value = params->Get(name->ToString());

    argv[0] = fillFunctionParameter(baton->functionDescHandle, baton->functionHandle, name, value);
    if (!argv[0]->IsNull()) {
      baton->callback->Call(Context::GetCurrent()->Global(), 2, argv);
      delete baton;
      return scope.Close(Undefined());
    }
  }

  uv_queue_work(uv_default_loop(), &baton->request, InvokeAsync, (uv_after_work_cb)InvokeAsyncAfter);

  return scope.Close(Undefined());
}


Handle<Value> Client::Ping(const Arguments& args) {
  HandleScope scope;
  Client *wrapper = Unwrap<Client>(args.This());

  RFC_RC rc;
  RFC_ERROR_INFO errorInfo;
  rc = RfcPing(wrapper->connectionHandle, &errorInfo);
  if (rc != RFC_OK) {
    return scope.Close(False());
  }
  return scope.Close(True());
}

Handle<Value> Client::ConnectionInfo(const Arguments& args) {
  HandleScope scope;
  Client *wrapper = Unwrap<Client>(args.This());
  Local<Object> infoObj = Object::New();

  RFC_RC rc;
  RFC_ERROR_INFO errorInfo;
  RFC_ATTRIBUTES connInfo;

  rc = RfcGetConnectionAttributes(wrapper->connectionHandle, &connInfo, &errorInfo);

  if (rc != RFC_OK) {
    // FIXME
    // return wrapError(&errorInfo)
    return scope.Close(infoObj);
  }

  infoObj->Set(String::New("dest"), wrapString(connInfo.dest, 64));
  infoObj->Set(String::New("host"), wrapString(connInfo.host, 100));
  infoObj->Set(String::New("partnerHost"), wrapString(connInfo.partnerHost, 100));
  infoObj->Set(String::New("sysNumber"), wrapString(connInfo.sysNumber, 2));
  infoObj->Set(String::New("sysId"), wrapString(connInfo.sysId, 8));
  infoObj->Set(String::New("client"), wrapString(connInfo.client, 3));
  infoObj->Set(String::New("user"), wrapString(connInfo.user, 8));
  infoObj->Set(String::New("language"), wrapString(connInfo.language, 2));
  infoObj->Set(String::New("trace"), wrapString(connInfo.trace, 1));
  infoObj->Set(String::New("isoLanguage"), wrapString(connInfo.isoLanguage, 2));
  infoObj->Set(String::New("codepage"), wrapString(connInfo.codepage, 4));
  infoObj->Set(String::New("partnerCodepage"), wrapString(connInfo.partnerCodepage, 4));
  infoObj->Set(String::New("rfcRole"), wrapString(connInfo.rfcRole, 1));
  infoObj->Set(String::New("type"), wrapString(connInfo.type, 1));
  infoObj->Set(String::New("partnerType"), wrapString(connInfo.partnerType, 1));
  infoObj->Set(String::New("rel"), wrapString(connInfo.rel, 4));
  infoObj->Set(String::New("partnerRel"), wrapString(connInfo.partnerRel, 4));
  infoObj->Set(String::New("kernelRel"), wrapString(connInfo.kernelRel, 4));
  infoObj->Set(String::New("cpicConvId"), wrapString(connInfo.cpicConvId, 8));
  infoObj->Set(String::New("progName"), wrapString(connInfo.progName, 128));
  infoObj->Set(String::New("partnerBytesPerChar"), wrapString(connInfo.partnerBytesPerChar, 1));
  infoObj->Set(String::New("reserved"), wrapString(connInfo.reserved, 84));

  return scope.Close(infoObj);
}

Handle<Value> Client::GetVersion(const Arguments& args) {
  HandleScope scope;
  unsigned major, minor, patchLevel;

  RfcGetVersion(&major, &minor, &patchLevel);
  Local<Array> version = Array::New(3);

  version->Set(Integer::New(0), Integer::New(major));
  version->Set(Integer::New(1), Integer::New(minor));
  version->Set(Integer::New(2), Integer::New(patchLevel));

  return scope.Close(version);
}


/*
Handle<Value> Client::DateTest(const Arguments& args) {
  HandleScope scope;

  if (!args[0]->IsDate()) {
    return ThrowException(String::New("Please provide Date object"));
  }
  Handle<Object> d = args[0]->ToObject();//'1400765915');
  double tval = args[0]->NumberValue()/1000;  // /1000 -> seconds instead of millisec timestamp

  printf("Date with %fl.", tval);
  //return scope.Close(Handle<Value>::Cast(d->Get(String::New("getYear"))));
  //return scope.Close(String::New(args[0]->NumberValue()));
  unsigned major, minor, patchLevel;

  Local<Value> newDate = Date::New((double)0);
  Local<Object> dateObj = newDate->ToObject();

  Local<Function> dateFunc = Function::Cast(*dateObj->Get(String::New("setFullYear")));
  dateFunc->Call(newDate, 1, ));
  RfcGetVersion(&major, &minor, &patchLevel);
  Local<Array> version = Array::New(5);

  version->Set(Integer::New(0), Integer::New(major));
  version->Set(Integer::New(1), Integer::New(minor));
  version->Set(Integer::New(2), Integer::New(patchLevel));

  Local<Function> getYearFunc = Function::Cast(*d->Get(String::New("getFullYear")));
  version->Set(Integer::New(3), getYearFunc->Call(d, 0, NULL));
  version->Set(Integer::New(4), newDate);

  return scope.Close(version);
}
*/

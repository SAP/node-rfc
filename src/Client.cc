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
#include "rfcio.h"
#include "Client.h"


using namespace v8;
using namespace node;

Persistent<Function> Client::constructor;

Client::Client() :
    connectionHandle(NULL),
    connectionParams(NULL),
    paramSize(0),
    alive(false)
{ uv_mutex_init(&this->invocationMutex); }

Client::~Client() {
    RFC_ERROR_INFO errorInfo;
    RFC_RC rc = RfcCloseConnection(this->connectionHandle, &errorInfo);
    rc = rc; // FIXME check rc ...
    this->alive = false;
    uv_mutex_destroy(&this->invocationMutex);
    for (unsigned int i = 0; i < this->paramSize; i++) {
        free(const_cast<SAP_UC*>(connectionParams[i].name));
        free(const_cast<SAP_UC*>(connectionParams[i].value));
    }
    free(connectionParams);
}

void Client::Init(Handle<Object> exports) {
    Isolate* isolate = Isolate::GetCurrent();

    // Prepare constructor template
    Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
    tpl->SetClassName(String::NewFromUtf8(isolate, "Client"));
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    // Prototype
    NODE_SET_PROTOTYPE_METHOD(tpl, "connect", Connect);
    NODE_SET_PROTOTYPE_METHOD(tpl, "close", Close);
    NODE_SET_PROTOTYPE_METHOD(tpl, "reopen", Reopen);
    NODE_SET_PROTOTYPE_METHOD(tpl, "isAlive", IsAlive);
    NODE_SET_PROTOTYPE_METHOD(tpl, "invoke", Invoke);
    NODE_SET_PROTOTYPE_METHOD(tpl, "getVersion", GetVersion);
    NODE_SET_PROTOTYPE_METHOD(tpl, "connectionInfo", ConnectionInfo);
    NODE_SET_PROTOTYPE_METHOD(tpl, "ping", Ping);

    constructor.Reset(isolate, tpl->GetFunction());
    exports->Set(String::NewFromUtf8(isolate, "Client"), tpl->GetFunction());
}

void Client::New(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = Isolate::GetCurrent();

    if (!args.IsConstructCall()) {
        args.GetReturnValue().Set(isolate->ThrowException(Exception::TypeError(
            String::NewFromUtf8(isolate, "Use the new operator to create instances of Rfc connection."))));
    }   

    if (args.Length() < 1) {
        args.GetReturnValue().Set(isolate->ThrowException(String::NewFromUtf8(isolate, "Please provide connection parameters as argument")));
    }

    if (!args[0]->IsObject()) {
        args.GetReturnValue().Set(isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Connection parameters must be an object"))));
    }

    Client *wrapper = new Client();
    wrapper->Wrap(args.This());

    if (args.Length() > 1) {
        if (!args[1]->IsBoolean()) {
            args.GetReturnValue().Set(isolate->ThrowException(Exception::TypeError(
                String::NewFromUtf8(isolate, "Third parameter for 'rstrip' must be true or false"))));
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

    args.GetReturnValue().Set(args.This());
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
    Isolate* isolate = Isolate::GetCurrent();

    ClientBaton* baton = static_cast<ClientBaton*>(req->data);

    if (baton->errorInfo.code != RFC_OK) {
        Local<Value> argv[] =  { wrapError(&baton->errorInfo) };

        TryCatch try_catch;
        Local<Function> localCallback = Local<Function>::New(isolate, baton->callback);
        localCallback->Call(isolate->GetCurrentContext()->Global(), 1, argv);

        if (try_catch.HasCaught()) {
            FatalException(try_catch);
        }
    } 
    else {
        baton->wrapper->alive = true;
        Local<Function> localCallback = Local<Function>::New(isolate, baton->callback);
        localCallback->Call(isolate->GetCurrentContext()->Global(), 0, NULL);
    }

    baton->callback.Reset();
    delete baton;
}

void Client::Connect(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = Isolate::GetCurrent();

    if (!args[0]->IsFunction()) {
       args.GetReturnValue().Set(isolate->ThrowException(Exception::TypeError(
            String::NewFromUtf8(isolate, "First Argument must be callback function"))));
    }

    Client *wrapper = Unwrap<Client>(args.This());

    ClientBaton* baton = new ClientBaton();
    baton->request.data = baton;
    baton->wrapper = wrapper;

    Local<Function> callback = Local <Function>::Cast(args[0]);
    baton->callback.Reset(isolate, callback);

    uv_queue_work(uv_default_loop(), &baton->request, ConnectAsync, (uv_after_work_cb)ConnectAsyncAfter);

    args.GetReturnValue().SetUndefined();
}

void Client::Close(const FunctionCallbackInfo<Value>& args) {
    Client *wrapper = Unwrap<Client>(args.This());
    RFC_RC rc;
    RFC_ERROR_INFO errorInfo;
    rc = RfcCloseConnection(wrapper->connectionHandle, &errorInfo);
    wrapper->alive = false;
    if (rc != RFC_OK) {
        args.GetReturnValue().Set(wrapError(&errorInfo));
    }

    args.GetReturnValue().SetUndefined();
}

void Client::Reopen(const FunctionCallbackInfo<Value>& args) {
    Client *wrapper = Unwrap<Client>(args.This());

    wrapper->Close(args);
    return wrapper->Connect(args);
}

void Client::IsAlive(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);
    Client *wrapper = Unwrap<Client>(args.This());
    args.GetReturnValue().Set(Boolean::New(isolate, wrapper->alive));
}


void Client::InvokeAsync(uv_work_t* req) {
    InvokeBaton* baton = static_cast<InvokeBaton*>(req->data);

    RfcInvoke(baton->wrapper->connectionHandle, baton->functionHandle, &baton->errorInfo);
    baton->wrapper->UnlockMutex();
}

void Client::InvokeAsyncAfter(uv_work_t* req) {
    Isolate* isolate = Isolate::GetCurrent();

    InvokeBaton* baton = static_cast<InvokeBaton*>(req->data);

    Local<Value> argv[] = { Null(isolate), Null(isolate) };

    if (baton->errorInfo.code != RFC_OK) {
        argv[0] = wrapError(&baton->errorInfo) ;
        RfcDestroyFunction(baton->functionHandle, NULL);

        TryCatch try_catch;

        Local<Function> localCallback = Local<Function>::New(isolate, baton->callback);
        localCallback->Call(isolate->GetCurrentContext()->Global(), 1, argv);

        if (try_catch.HasCaught()) {
            FatalException(try_catch);
        }
    } else {
        argv[1] = wrapResult(baton->functionDescHandle, baton->functionHandle, baton->wrapper->rstrip);
        RfcDestroyFunction(baton->functionHandle, NULL);

        TryCatch try_catch;

        Local<Function> localCallback = Local<Function>::New(isolate, baton->callback);
        localCallback->Call(isolate->GetCurrentContext()->Global(), 2, argv);

        if (try_catch.HasCaught()) {
            FatalException(try_catch);
        }
    }

    baton->callback.Reset();
    delete baton;
}

void Client::Invoke(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = Isolate::GetCurrent();

    if (args.Length() < 3) {
        args.GetReturnValue().Set(isolate->ThrowException(String::NewFromUtf8(isolate, "Please provide function module, parameters and callback as parameters")));
        return;
    }
    if (!args[0]->IsString()) {
        args.GetReturnValue().Set(isolate->ThrowException(Exception::TypeError(
            String::NewFromUtf8(isolate, "First parameter (rfc function name) must be an string"))));
        return;
    }
    if (!args[1]->IsObject()) {
        args.GetReturnValue().Set(isolate->ThrowException(Exception::TypeError(
            String::NewFromUtf8(isolate, "Second parameter (rfc function arguments) must be an object"))));
        return;
    }
    if (!args[2]->IsFunction()) {
        args.GetReturnValue().Set(isolate->ThrowException(Exception::TypeError(
            String::NewFromUtf8(isolate, "Third Argument must be callback function"))));
        return;
    }

    Client *wrapper = Unwrap<Client>(args.This());

    InvokeBaton* baton = new InvokeBaton();
    baton->request.data = baton;
    baton->wrapper = wrapper;
    Local<Function> callback = Local <Function>::Cast(args[2]);
    baton->callback.Reset(isolate, callback);   // baton->callback = Persistent<Function>::New(callback);

    Handle<Value> argv[2] = { Null(isolate), Null(isolate) };
    SAP_UC *funcName = fillString(args[0]);

    baton->wrapper->LockMutex();
    baton->functionDescHandle = RfcGetFunctionDesc(wrapper->connectionHandle, funcName, &baton->errorInfo);
    free(funcName);
    if (baton->functionDescHandle == NULL) {
        // ABAP function module not found
        argv[0] = wrapError(&baton->errorInfo);
        callback->Call(isolate->GetCurrentContext()->Global(), 1, argv);
        delete baton;
        args.GetReturnValue().SetUndefined();
    } else {

        baton->functionHandle = RfcCreateFunction(baton->functionDescHandle, &baton->errorInfo);

        Local<Object> params = args[1]->ToObject();
        Local<Array> paramNames = params->GetPropertyNames();
        unsigned int paramSize = paramNames->Length();

        for (unsigned int i = 0; i < paramSize; i++) {
            Local<Value> name = paramNames->Get(i);
            Local<Value> value = params->Get(name->ToString());
            argv[0] = fillFunctionParameter(baton->functionDescHandle, baton->functionHandle, name, value);
            if (!argv[0]->IsNull()) {
                // Invalid parameter name
                Local<Function> localCallback = Local<Function>::New(isolate, baton->callback);
                localCallback->Call(isolate->GetCurrentContext()->Global(), 1, argv);
                delete baton;
                args.GetReturnValue().SetUndefined();
                return; // skip RFC invoke
            }
        }

        uv_queue_work(uv_default_loop(), &baton->request, InvokeAsync, (uv_after_work_cb)InvokeAsyncAfter);

        args.GetReturnValue().SetUndefined();
    }
}


void Client::Ping(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = Isolate::GetCurrent();
    Client *wrapper = Unwrap<Client>(args.This());

    RFC_RC rc;
    RFC_ERROR_INFO errorInfo;
    rc = RfcPing(wrapper->connectionHandle, &errorInfo);
    if (rc != RFC_OK) {
        args.GetReturnValue().Set(False(isolate));
    } else {
        args.GetReturnValue().Set(True(isolate));
    }
}

void Client::ConnectionInfo(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = Isolate::GetCurrent();
    Client *wrapper = Unwrap<Client>(args.This());
    Local<Object> infoObj = Object::New(isolate);

    RFC_RC rc;
    RFC_ERROR_INFO errorInfo;
    RFC_ATTRIBUTES connInfo;

    rc = RfcGetConnectionAttributes(wrapper->connectionHandle, &connInfo, &errorInfo);

    if (rc != RFC_OK) {
        args.GetReturnValue().Set(wrapError(&errorInfo));
    }

    infoObj->Set(String::NewFromUtf8(isolate, "dest"), wrapString(connInfo.dest, 64));
    infoObj->Set(String::NewFromUtf8(isolate, "host"), wrapString(connInfo.host, 100));
    infoObj->Set(String::NewFromUtf8(isolate, "partnerHost"), wrapString(connInfo.partnerHost, 100));
    infoObj->Set(String::NewFromUtf8(isolate, "sysNumber"), wrapString(connInfo.sysNumber, 2));
    infoObj->Set(String::NewFromUtf8(isolate, "sysId"), wrapString(connInfo.sysId, 8));
    infoObj->Set(String::NewFromUtf8(isolate, "client"), wrapString(connInfo.client, 3));
    infoObj->Set(String::NewFromUtf8(isolate, "user"), wrapString(connInfo.user, 8));
    infoObj->Set(String::NewFromUtf8(isolate, "language"), wrapString(connInfo.language, 2));
    infoObj->Set(String::NewFromUtf8(isolate, "trace"), wrapString(connInfo.trace, 1));
    infoObj->Set(String::NewFromUtf8(isolate, "isoLanguage"), wrapString(connInfo.isoLanguage, 2));
    infoObj->Set(String::NewFromUtf8(isolate, "codepage"), wrapString(connInfo.codepage, 4));
    infoObj->Set(String::NewFromUtf8(isolate, "partnerCodepage"), wrapString(connInfo.partnerCodepage, 4));
    infoObj->Set(String::NewFromUtf8(isolate, "rfcRole"), wrapString(connInfo.rfcRole, 1));
    infoObj->Set(String::NewFromUtf8(isolate, "type"), wrapString(connInfo.type, 1));
    infoObj->Set(String::NewFromUtf8(isolate, "partnerType"), wrapString(connInfo.partnerType, 1));
    infoObj->Set(String::NewFromUtf8(isolate, "rel"), wrapString(connInfo.rel, 4));
    infoObj->Set(String::NewFromUtf8(isolate, "partnerRel"), wrapString(connInfo.partnerRel, 4));
    infoObj->Set(String::NewFromUtf8(isolate, "kernelRel"), wrapString(connInfo.kernelRel, 4));
    infoObj->Set(String::NewFromUtf8(isolate, "cpicConvId"), wrapString(connInfo.cpicConvId, 8));
    infoObj->Set(String::NewFromUtf8(isolate, "progName"), wrapString(connInfo.progName, 128));
    infoObj->Set(String::NewFromUtf8(isolate, "partnerBytesPerChar"), wrapString(connInfo.partnerBytesPerChar, 1));
    infoObj->Set(String::NewFromUtf8(isolate, "reserved"), wrapString(connInfo.reserved, 84));

    args.GetReturnValue().Set(infoObj);
}

void Client::GetVersion(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = Isolate::GetCurrent();
    unsigned major, minor, patchLevel;

    RfcGetVersion(&major, &minor, &patchLevel);
    Local<Array> version = Array::New(isolate, 3);

    version->Set(Integer::New(isolate, 0), Integer::New(isolate, major));
    version->Set(Integer::New(isolate, 1), Integer::New(isolate, minor));
    version->Set(Integer::New(isolate, 2), Integer::New(isolate, patchLevel));

    args.GetReturnValue().Set(version);
}


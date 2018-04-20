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
#include "Client.h"
#include "error.h"

using namespace v8;
using namespace node;

Nan::Persistent<Function> Client::constructor;

Client::Client() :
    connectionHandle(NULL),
    connectionParams(NULL),
    paramSize(0),
    alive(false) { 
    uv_sem_init(&this->invocationMutex, 1);
}

Client::~Client() {
    RFC_ERROR_INFO errorInfo;
    RFC_RC rc = RfcCloseConnection(this->connectionHandle, &errorInfo);
    if (rc != RFC_OK) {
        printf ("Error closing connection: %u", rc); // FIXME error handling
    }
    this->alive = false;
    uv_sem_destroy(&this->invocationMutex);
    for (unsigned int i = 0; i < this->paramSize; i++) {
        free(const_cast<SAP_UC*>(connectionParams[i].name));
        free(const_cast<SAP_UC*>(connectionParams[i].value));
    }
    free(connectionParams);
}

NAN_MODULE_INIT(Client::Init) {
    // Prepare constructor template
    Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);
    tpl->SetClassName(Nan::New("Client").ToLocalChecked());
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    // Prototype
    Nan::SetPrototypeMethod(tpl, "connect", Connect);
    Nan::SetPrototypeMethod(tpl, "close", Close);
    Nan::SetPrototypeMethod(tpl, "reopen", Reopen);
    Nan::SetPrototypeMethod(tpl, "isAlive", IsAlive);
    Nan::SetPrototypeMethod(tpl, "invoke", Invoke);
    Nan::SetPrototypeMethod(tpl, "getVersion", GetVersion);
    Nan::SetPrototypeMethod(tpl, "connectionInfo", ConnectionInfo);
    Nan::SetPrototypeMethod(tpl, "ping", Ping);

    constructor.Reset(Nan::GetFunction(tpl).ToLocalChecked());
    Nan::Set(target, Nan::New("Client").ToLocalChecked(), Nan::GetFunction(tpl).ToLocalChecked());
}

NAN_METHOD(Client::New) {
    if (!info.IsConstructCall()) {
        Local<Value> e = Nan::TypeError("Use the new operator to create instances of Rfc connection.");
        Nan::ThrowError(e);
        info.GetReturnValue().Set(e);
        return;
    }   

    if (info.Length() < 1) {
        Local<Value> e = Nan::Error("Please provide connection parameters as argument");
        Nan::ThrowError(e);
        return info.GetReturnValue().Set(e);
    }

    if (!info[0]->IsObject()) {
        Local<Value> e = Nan::TypeError("Connection parameters must be an object");
        Nan::ThrowError(e);
        info.GetReturnValue().Set(e);
        return;
    }

    Client *wrapper = new Client();
    wrapper->Wrap(info.This());

    if (info.Length() > 1) {
        if (!info[1]->IsBoolean()) {
            Local<Value> e = Nan::TypeError("Third parameter for 'rstrip' must be true or false");
            Nan::ThrowError(e);
            info.GetReturnValue().Set(e);
            return;
        }

        wrapper->rstrip = info[1]->BooleanValue();
    } else {
        wrapper->rstrip = true;
    }

    Local<Object> connectionParams = info[0]->ToObject();
    Local<Array> paramNames = connectionParams->GetPropertyNames();
    wrapper->paramSize = paramNames->Length();
    wrapper->connectionParams = static_cast<RFC_CONNECTION_PARAMETER*>(malloc(wrapper->paramSize * sizeof(RFC_CONNECTION_PARAMETER)));

    for (unsigned int i = 0; i < wrapper->paramSize; i++) {
        Local<Value> name = paramNames->Get(i);
        Local<Value> value = connectionParams->Get(name->ToString());

        wrapper->connectionParams[i].name = fillString(name);
        wrapper->connectionParams[i].value = fillString(value);
    }

    info.GetReturnValue().Set(info.This());
}

void Client::LockMutex(void) {
    uv_sem_wait(&this->invocationMutex);
}

void Client::UnlockMutex(void) {
    uv_sem_post(&this->invocationMutex);
}


void Client::ConnectAsync(uv_work_t* req) {
    ClientBaton* baton = static_cast<ClientBaton*>(req->data);

    baton->wrapper->connectionHandle = RfcOpenConnection(baton->wrapper->connectionParams, baton->wrapper->paramSize, &baton->errorInfo);
}

void Client::ConnectAsyncAfter(uv_work_t* req, int status) {
    Nan::HandleScope scope;

    ClientBaton* baton = static_cast<ClientBaton*>(req->data);

    if (baton->errorInfo.code != RFC_OK) {
        Local<Value> argv[] =  { wrapError(&baton->errorInfo) };

        Nan::TryCatch try_catch;
        Local<Function> callback = Nan::New<Function>(baton->callback);
        Nan::Call(callback, Nan::New<Object>(), 1, argv);

        if (try_catch.HasCaught()) {
            Nan::FatalException(try_catch);
        }
    } 
    else {
        baton->wrapper->alive = true;
        Local<Function> callback = Nan::New<Function>(baton->callback);
        Nan::Call(callback, Nan::New<Object>(), 0, NULL);
    }

    baton->callback.Reset();
    delete baton;
}

NAN_METHOD(Client::Connect) {
    if (!info[0]->IsFunction()) {
       Local<Value> e = Nan::TypeError("First Argument must be callback function");
       Nan::ThrowError(e);
       info.GetReturnValue().Set(e);
       return;
    }

    Client *wrapper = Unwrap<Client>(info.This());

    ClientBaton* baton = new ClientBaton();
    baton->request.data = baton;
    baton->wrapper = wrapper;

    Local<Function> callback = info[0].As<Function>();
    baton->callback.Reset(callback);

    uv_queue_work(uv_default_loop(), &baton->request, ConnectAsync, (uv_after_work_cb)ConnectAsyncAfter);

    info.GetReturnValue().SetUndefined();
}

NAN_METHOD(Client::Close) {
    Client *wrapper = Unwrap<Client>(info.This());
    RFC_RC rc;
    RFC_ERROR_INFO errorInfo;
    rc = RfcCloseConnection(wrapper->connectionHandle, &errorInfo);
    wrapper->alive = false;
    if (rc != RFC_OK) {
        info.GetReturnValue().Set(wrapError(&errorInfo));
        return;
    }
    info.GetReturnValue().SetUndefined();
}

NAN_METHOD(Client::Reopen) {
    Client *wrapper = Unwrap<Client>(info.This());

    wrapper->Close(info);
    wrapper->Connect(info);
}

NAN_METHOD(Client::IsAlive) {
    Client *wrapper = Unwrap<Client>(info.This());
    info.GetReturnValue().Set(Nan::New(wrapper->alive));
}


void Client::InvokeAsync(uv_work_t* req) {
    InvokeBaton* baton = static_cast<InvokeBaton*>(req->data);

    RfcInvoke(baton->wrapper->connectionHandle, baton->functionHandle, &baton->errorInfo);
    baton->wrapper->UnlockMutex();
}

void Client::InvokeAsyncAfter(uv_work_t* req, int status) {
    Nan::HandleScope scope;

    InvokeBaton* baton = static_cast<InvokeBaton*>(req->data);

    Local<Value> argv[] = { Nan::Null(), Nan::Null() };

    if (baton->errorInfo.code != RFC_OK) {
        argv[0] = wrapError(&baton->errorInfo) ;
        RfcDestroyFunction(baton->functionHandle, NULL);

        Nan::TryCatch try_catch;

        Local<Function> callback = Nan::New<Function>(baton->callback);
        Nan::Call(callback, Nan::New<Object>(), 1, argv);

        if (try_catch.HasCaught()) {
            Nan::FatalException(try_catch);
        }
    } else {
        argv[1] = wrapResult(baton->functionDescHandle, baton->functionHandle, baton->wrapper->rstrip);
        RfcDestroyFunction(baton->functionHandle, NULL);

        Nan::TryCatch try_catch;

        Local<Function> callback = Nan::New<Function>(baton->callback);
        Nan::Call(callback, Nan::New<Object>(), 2, argv);

        if (try_catch.HasCaught()) {
            Nan::FatalException(try_catch);
        }
    }

    baton->callback.Reset();
    delete baton;
}

NAN_METHOD(Client::Invoke) {
    RFC_RC rc;
    Local <Array> notRequested = Nan::New<Array>();

    if (info.Length() < 3) {
        Local<Value> e = Nan::Error("Please provide function module, parameters and callback as arguments");
        Nan::ThrowError(e);
        return info.GetReturnValue().Set(e);
    }
    if (!info[0]->IsString()) {
        Local<Value> e = Nan::TypeError("First argument (rfc function name) must be an string");
        Nan::ThrowError(e);
        return info.GetReturnValue().Set(e);
    }
    if (!info[1]->IsObject()) {
        Local<Value> e = Nan::TypeError("Second argument (rfc function arguments) must be an object");
        Nan::ThrowError(e);
        return info.GetReturnValue().Set(e);
    }
    if (!info[2]->IsFunction()) {
        Local<Value> e = Nan::TypeError("Third argument must be callback function");
        Nan::ThrowError(e);
        return info.GetReturnValue().Set(e);
    }
    if (info.Length() == 4) {
        if (!info[3]->IsObject() ) {
            Local<Value> e = Nan::TypeError("Fourth argument is optional object");
            Nan::ThrowError(e);
            return info.GetReturnValue().Set(e);
        }
        v8::Local<v8::Object> obj = info[3]->ToObject();
        v8::Local<v8::Array> props = obj->GetPropertyNames();
        for (unsigned int i = 0; i < props->Length(); i++) {
            Local<Value> key = props->Get(i)->ToString();
            notRequested = obj->Get(key->ToString()).As<Array>();
        }
    }

    Client *wrapper = Unwrap<Client>(info.This());

    InvokeBaton* baton = new InvokeBaton();
    baton->request.data = baton;
    baton->wrapper = wrapper;
    Local<Function> callback = info[2].As<Function>();
    baton->callback.Reset(callback);

    Handle<Value> argv[2] = { Nan::Null(), Nan::Null() };
    SAP_UC *funcName = fillString(info[0]);

    baton->wrapper->LockMutex();
    baton->functionDescHandle = RfcGetFunctionDesc(wrapper->connectionHandle, funcName, &baton->errorInfo);
    free(funcName);
    if (baton->functionDescHandle == NULL) {
        // ABAP function module not found
        argv[0] = wrapError(&baton->errorInfo);
        Nan::Call(callback, Nan::New<Object>(), 1, argv);
        delete baton;
        info.GetReturnValue().SetUndefined();
    } else {

        baton->functionHandle = RfcCreateFunction(baton->functionDescHandle, &baton->errorInfo);

        if (notRequested->Length() != 0) {
              for (unsigned int i = 0; i < notRequested->Length(); i++) {
                Local<String> name = notRequested->Get(i)->ToString();
                SAP_UC *paramName = fillString(name);
                rc = RfcSetParameterActive(baton->functionHandle, paramName, 0, &baton->errorInfo);
                free(const_cast<SAP_UC*>(paramName));
                if (rc != RFC_OK) {
                    return info.GetReturnValue().Set(wrapError(&baton->errorInfo));
                }   
            } 
        }

        Local<Object> params = info[1]->ToObject();
        Local<Array> paramNames = params->GetPropertyNames();
        unsigned int paramSize = paramNames->Length();

        for (unsigned int i = 0; i < paramSize; i++) {
            Local<Value> name = paramNames->Get(i);
            Local<Value> value = params->Get(name->ToString());
            argv[0] = fillFunctionParameter(baton->functionDescHandle, baton->functionHandle, name, value);
            if (!argv[0]->IsNull()) {
                // Invalid parameter name, skip RFC invoke
                Local<Function> callback = Nan::New<Function>(baton->callback);
                Nan::Call(callback, Nan::New<Object>(), 1, argv);
                delete baton;
                return info.GetReturnValue().SetUndefined();
            }
        }

        uv_queue_work(uv_default_loop(), &baton->request, InvokeAsync, (uv_after_work_cb)InvokeAsyncAfter);

        info.GetReturnValue().SetUndefined();
    }
}

NAN_METHOD(Client::Ping) {
    Client *wrapper = Unwrap<Client>(info.This());

    RFC_RC rc;
    RFC_ERROR_INFO errorInfo;
    rc = RfcPing(wrapper->connectionHandle, &errorInfo);
    if (rc != RFC_OK) {
        info.GetReturnValue().Set(Nan::False());
    } else {
        info.GetReturnValue().Set(Nan::True());
    }
}

NAN_METHOD(Client::ConnectionInfo) {
    Client *wrapper = Unwrap<Client>(info.This());
    Local<Object> infoObj = Nan::New<Object>();

    RFC_RC rc;
    RFC_ERROR_INFO errorInfo;
    RFC_ATTRIBUTES connInfo;

    rc = RfcGetConnectionAttributes(wrapper->connectionHandle, &connInfo, &errorInfo);

    if (rc != RFC_OK) {
        return info.GetReturnValue().Set(wrapError(&errorInfo));
    }   

    Nan::Set(infoObj, Nan::New("dest").ToLocalChecked(),                    wrapString(connInfo.dest, 64));
    Nan::Set(infoObj, Nan::New("host").ToLocalChecked(),                    wrapString(connInfo.host, 100));
    Nan::Set(infoObj, Nan::New("partnerHost").ToLocalChecked(),             wrapString(connInfo.partnerHost, 100));
    Nan::Set(infoObj, Nan::New("sysNumber").ToLocalChecked(),               wrapString(connInfo.sysNumber, 2));
    Nan::Set(infoObj, Nan::New("sysId").ToLocalChecked(),                   wrapString(connInfo.sysId, 8));
    Nan::Set(infoObj, Nan::New("client").ToLocalChecked(),                  wrapString(connInfo.client, 3));
    Nan::Set(infoObj, Nan::New("user").ToLocalChecked(),                    wrapString(connInfo.user, 8, true));
    Nan::Set(infoObj, Nan::New("language").ToLocalChecked(),                wrapString(connInfo.language, 2));
    Nan::Set(infoObj, Nan::New("trace").ToLocalChecked(),                   wrapString(connInfo.trace, 1));
    Nan::Set(infoObj, Nan::New("isoLanguage").ToLocalChecked(),             wrapString(connInfo.isoLanguage, 2));
    Nan::Set(infoObj, Nan::New("codepage").ToLocalChecked(),                wrapString(connInfo.codepage, 4));
    Nan::Set(infoObj, Nan::New("partnerCodepage").ToLocalChecked(),         wrapString(connInfo.partnerCodepage, 4));
    Nan::Set(infoObj, Nan::New("rfcRole").ToLocalChecked(),                 wrapString(connInfo.rfcRole, 1));
    Nan::Set(infoObj, Nan::New("type").ToLocalChecked(),                    wrapString(connInfo.type, 1));
    Nan::Set(infoObj, Nan::New("partnerType").ToLocalChecked(),             wrapString(connInfo.partnerType, 1));
    Nan::Set(infoObj, Nan::New("rel").ToLocalChecked(),                     wrapString(connInfo.rel, 4, True));
    Nan::Set(infoObj, Nan::New("partnerRel").ToLocalChecked(),              wrapString(connInfo.partnerRel, 4, true));
    Nan::Set(infoObj, Nan::New("kernelRel").ToLocalChecked(),               wrapString(connInfo.kernelRel, 4, true));
    Nan::Set(infoObj, Nan::New("cpicConvId").ToLocalChecked(),              wrapString(connInfo.cpicConvId, 8));
    Nan::Set(infoObj, Nan::New("progName").ToLocalChecked(),                wrapString(connInfo.progName, 128, true));
    Nan::Set(infoObj, Nan::New("partnerBytesPerChar").ToLocalChecked(),     wrapString(connInfo.partnerBytesPerChar, 1));
    Nan::Set(infoObj, Nan::New("reserved").ToLocalChecked(),                wrapString(connInfo.reserved, 84));

    info.GetReturnValue().Set(infoObj);
}

NAN_METHOD(Client::GetVersion) {
    Isolate* isolate = Isolate::GetCurrent();
    unsigned major, minor, patchLevel;

    RfcGetVersion(&major, &minor, &patchLevel);
    Local<Object> version = Object::New(isolate);
    version->Set(String::NewFromUtf8(isolate, "major"), Integer::New(isolate, major));
    version->Set(String::NewFromUtf8(isolate, "minor"), Integer::New(isolate, minor));
    version->Set(String::NewFromUtf8(isolate, "patchLevel"), Uint32::New(isolate, patchLevel));
    info.GetReturnValue().Set(version);
}


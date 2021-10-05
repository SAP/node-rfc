// SPDX-FileCopyrightText: 2014 SAP SE Srdjan Boskovic <srdjan.boskovic@sap.com>
//
// SPDX-License-Identifier: Apache-2.0

#include "Server.h"
#include "nwrfcsdk.h"

#include <node_api.h>
#include <assert.h>
#include <uv.h>
#include <stdio.h>

namespace node_rfc
{
    extern Napi::Env __env;
    ClientOptionsStruct client_options; // Messy horror
    uint_t Server::_id = 1;
    Server *__server = NULL;

    typedef struct
    {
        napi_async_work work;
        napi_threadsafe_function tsfn;
    } AddonData;

    Napi::Object Server::Init(Napi::Env env, Napi::Object exports)
    {
        Napi::HandleScope scope(env);

        Napi::Function func = DefineClass(
            env, "Server", {
                               InstanceAccessor("_id", &Server::IdGetter, nullptr),
                               InstanceAccessor("_alive", &Server::AliveGetter, nullptr),
                               InstanceAccessor("_server_conn_handle", &Server::ServerConnectionHandleGetter, nullptr),
                               InstanceAccessor("_client_conn_handle", &Server::ClientConnectionHandleGetter, nullptr),
                               InstanceMethod("start", &Server::Start),
                               InstanceMethod("stop", &Server::Stop),
                               InstanceMethod("addFunction", &Server::AddFunction),
                               InstanceMethod("removeFunction", &Server::RemoveFunction),
                               InstanceMethod("getFunctionDescription", &Server::GetFunctionDescription),
                           });

        Napi::FunctionReference *constructor = new Napi::FunctionReference();
        *constructor = Napi::Persistent(func);
        constructor->SuppressDestruct();

        exports.Set("Server", func);
        return exports;
    }

    Napi::Value Server::IdGetter(const Napi::CallbackInfo &info)
    {
        return Napi::Number::New(Env(), id);
    }

    Napi::Value Server::AliveGetter(const Napi::CallbackInfo &info)
    {
        return Napi::Boolean::New(Env(), server_conn_handle != NULL);
    }

    Napi::Value Server::ServerConnectionHandleGetter(const Napi::CallbackInfo &info)
    {
        return Napi::Number::New(info.Env(), (double)(unsigned long long)this->server_conn_handle);
    }
    Napi::Value Server::ClientConnectionHandleGetter(const Napi::CallbackInfo &info)
    {
        return Napi::Number::New(info.Env(), (double)(unsigned long long)this->client_conn_handle);
    }

    Server::Server(const Napi::CallbackInfo &info) : Napi::ObjectWrap<Server>(info)
    {
        node_rfc::__server = this;

        init(info.Env());

        DEBUG("Server::Server ", id);

        if (!info[0].IsObject())
        {
            Napi::TypeError::New(Env(), "Server constructor requires server parameters").ThrowAsJavaScriptException();
            return;
        }

        serverParamsRef = Napi::Persistent(info[0].As<Napi::Object>());
        getConnectionParams(serverParamsRef.Value(), &server_params);

        if (!info[1].IsObject())
        {
            Napi::TypeError::New(Env(), "Server constructor requires client parameters").ThrowAsJavaScriptException();
            return;
        }

        clientParamsRef = Napi::Persistent(info[1].As<Napi::Object>());
        getConnectionParams(clientParamsRef.Value(), &client_params);

        if (!info[2].IsUndefined())
        {
            if (!info[2].IsObject())
            {
                Napi::TypeError::New(Env(), "Server constructor client options must be an object").ThrowAsJavaScriptException();
                return;
            }
            clientOptionsRef = Napi::Persistent(info[2].As<Napi::Object>());
            checkClientOptions(clientOptionsRef.Value(), &client_options);
        }
    };

    RFC_RC SAP_API metadataLookup(SAP_UC const *func_name, RFC_ATTRIBUTES rfc_attributes, RFC_FUNCTION_DESC_HANDLE *func_desc_handle)
    {
        RFC_RC rc = RFC_NOT_FOUND;

        Server *server = node_rfc::__server; // todo check if null

        ServerFunctionsMap::iterator it = server->serverFunctions.begin();
        while (it != server->serverFunctions.end())
        {
            if (strcmpU(func_name, it->second.func_name) == 0)
            {
                *func_desc_handle = it->second.func_desc_handle;
                rc = RFC_OK;
                DEBUG("\nmetadataLookup found: ", (pointer_t)*func_desc_handle);
                break;
            }
            it++;
        }

        return rc;
    }

    RFC_RC SAP_API genericHandler(RFC_CONNECTION_HANDLE conn_handle, RFC_FUNCTION_HANDLE func_handle, RFC_ERROR_INFO *errorInfo)
    {
        Server *server = node_rfc::__server;

        RFC_RC rc = RFC_NOT_FOUND;

        RFC_FUNCTION_DESC_HANDLE func_desc = RfcDescribeFunction(func_handle, errorInfo);
        if (errorInfo->code != RFC_OK)
        {
            return errorInfo->code;
        }
        RFC_ABAP_NAME func_name;
        RfcGetFunctionName(func_desc, func_name, errorInfo);
        if (errorInfo->code != RFC_OK)
        {
            return errorInfo->code;
        }

        ServerFunctionsMap::iterator it = server->serverFunctions.begin();
        while (it != server->serverFunctions.end())
        {
            if (strcmpU(func_name, it->second.func_name) == 0)
            {
                break;
            }
            it++;
        }

        if (it == server->serverFunctions.end())
        {
            printf("not found!\n");
            return rc;
        }

        RFC_FUNCTION_DESC_HANDLE func_desc_handle = it->second.func_desc_handle;


        //
        // JS Call
        //
  
        ServerCallbackContainer *payload = new ServerCallbackContainer();
        auto errorPath = node_rfc::RfmErrorPath();
        
        // Initialize the condition variable and mutexes and wait for the JS callback to complete
        
        uv_cond_init(&payload->wait_js);
        uv_mutex_init(&payload->wait_js_mutex);
        uv_mutex_init(&payload->js_running_mutex);
        payload->func_desc_handle = func_desc_handle;
        payload->func_handle = func_handle;
        payload->errorInfo = errorInfo;
        payload->client_options = client_options;
        payload->errorPath = errorPath;
        
        
        DEBUG("Before cond [genericHandler] with cond_mutex @", (unsigned long)&payload->cond_mutex);
        uv_mutex_lock(&payload->js_running_mutex);
        DEBUG("Obtained js_running_mutex lock\n");
	      payload->js_running = true;
	      uv_mutex_unlock(&payload->js_running_mutex);
	      
	      it->second.threadSafeCallback.BlockingCall(payload);
	      
	      while(true) {
	          uv_mutex_lock(&payload->wait_js_mutex);
	          uv_cond_wait(&payload->wait_js, &payload->wait_js_mutex);
	          uv_mutex_unlock(&payload->wait_js_mutex);
	          	          
	          // Exit condition
	          uv_mutex_lock(&payload->js_running_mutex);
	          bool exit = !payload->js_running;
	          uv_mutex_unlock(&payload->js_running_mutex);
	          
	          if(exit) break;
	      }
	      
	      uv_mutex_destroy(&payload->wait_js_mutex);
        uv_mutex_destroy(&payload->js_running_mutex);
        uv_cond_destroy(&payload->wait_js);
        DEBUG("After cond [genericHandler]\n");
        delete payload;
        
        if (errorInfo->code != RFC_OK) {
          return errorInfo->code;
        } else { 
        	return RFC_OK;
        }
   }

    class StartAsync : public Napi::AsyncWorker
    {
    public:
        StartAsync(Napi::Function &callback, Server *server)
            : Napi::AsyncWorker(callback), server(server) {}
        ~StartAsync()
        {
        }

        void Execute()
        {
            server->LockMutex();
            DEBUG("StartAsync locked");
            server->client_conn_handle = RfcOpenConnection(server->client_params.connectionParams, server->client_params.paramSize, &errorInfo);
            if (errorInfo.code != RFC_OK)
            {
                return;
            }
            DEBUG("Server:: client connection ok");

            server->server_conn_handle = RfcRegisterServer(server->server_params.connectionParams, server->server_params.paramSize, &errorInfo);
            if (errorInfo.code != RFC_OK)
            {
                return;
            }
            DEBUG("Server:: registered");

            RfcInstallGenericServerFunction(genericHandler, metadataLookup, &errorInfo);
            if (errorInfo.code != RFC_OK)
            {
                return;
            }
            DEBUG("Server:: installed");

            server->serverHandle = RfcCreateServer(server->server_params.connectionParams, 1, &errorInfo);
            if (errorInfo.code != RFC_OK)
            {
                return;
            }
            DEBUG("Server:: created");

            RfcLaunchServer(server->serverHandle, &errorInfo);
            if (errorInfo.code != RFC_OK)
            {
                return;
            }
            DEBUG("Server:: launched ", (pointer_t)server->serverHandle)

            server->UnlockMutex();
            DEBUG("StartAsync unlocked");
        }

        void OnOK()
        {
            Napi::EscapableHandleScope scope(Env());
            if (errorInfo.code != RFC_OK)
            {
                //Callback().Call({Env().Undefined()});
                Callback().Call({rfcSdkError(&errorInfo)});
            }
            else
            {
                Callback().Call({});
            }
            Callback().Reset();
        }

    private:
        Server *server;
        RFC_ERROR_INFO errorInfo;
    };

    class GetFunctionDescAsync : public Napi::AsyncWorker
    {
    public:
        GetFunctionDescAsync(Napi::Function &callback, Server *server)
            : Napi::AsyncWorker(callback), server(server) {}
        ~GetFunctionDescAsync() {}

        void Execute()
        {
            server->LockMutex();
            server->server_conn_handle = RfcRegisterServer(server->server_params.connectionParams, server->server_params.paramSize, &errorInfo);
            server->UnlockMutex();
        }

        void OnOK()
        {
            if (server->server_conn_handle == NULL)
            {
                Callback().Call({rfcSdkError(&errorInfo)});
            }
            else
            {
                Callback().Call({});
            }
            Callback().Reset();
        }

    private:
        Server *server;
        RFC_ERROR_INFO errorInfo;
    };

    Napi::Value Server::Start(const Napi::CallbackInfo &info)
    {
        DEBUG("Server::Serve");

        std::ostringstream errmsg;

        if (!info[0].IsFunction())
        {
            errmsg << "Server start() requires a callback function; see" << USAGE_URL;
            Napi::TypeError::New(info.Env(), errmsg.str()).ThrowAsJavaScriptException();
            return info.Env().Undefined();
        }

        Napi::Function callback = info[0].As<Napi::Function>();

        (new StartAsync(callback, this))->Queue();

        return info.Env().Undefined();
    };

    Napi::Value Server::Stop(const Napi::CallbackInfo &info)
    {
        DEBUG("Server::Stop");

        std::ostringstream errmsg;

        if (!info[0].IsFunction())
        {
            errmsg << "Server stop() requires a callback function; see" << USAGE_URL;
            Napi::TypeError::New(info.Env(), errmsg.str()).ThrowAsJavaScriptException();
            return info.Env().Undefined();
        }

        //Napi::Function callback = info[0].As<Napi::Function>();
        //(new StopAsync(callback, this))->Queue();

        return info.Env().Undefined();
    };

    Napi::Value Server::AddFunction(const Napi::CallbackInfo &info)
    {
        Napi::EscapableHandleScope scope(info.Env());

        std::ostringstream errmsg;

        if (!info[0].IsString())
        {
            errmsg << "Server addFunction() requires ABAP RFM name; see" << USAGE_URL;
            Napi::TypeError::New(info.Env(), errmsg.str()).ThrowAsJavaScriptException();
            return info.Env().Undefined();
        }

        Napi::String functionName = info[0].As<Napi::String>();

        if (functionName.Utf8Value().length() == 0 || functionName.Utf8Value().length() > 30)
        {
            errmsg << "Server addFunction() accepts max. 30 characters long ABAP RFM name; see" << USAGE_URL;
            Napi::TypeError::New(info.Env(), errmsg.str()).ThrowAsJavaScriptException();
            return info.Env().Undefined();
        }

        DEBUG("Server::AddFunction ", functionName.Utf8Value());

        if (!info[1].IsFunction())
        {
            errmsg << "Server addFunction() requires a NodeJS handler function; see" << USAGE_URL;
            Napi::TypeError::New(info.Env(), errmsg.str()).ThrowAsJavaScriptException();
            return info.Env().Undefined();
        }

        Napi::Function jsFunction = info[1].As<Napi::Function>();

        if (!info[2].IsFunction())
        {
            errmsg << "Server addFunction() requires a callback function; see" << USAGE_URL;
            Napi::TypeError::New(info.Env(), errmsg.str()).ThrowAsJavaScriptException();
            return info.Env().Undefined();
        }

        Napi::Function callback = info[2].As<Napi::Function>();

        // Install function
        RFC_ERROR_INFO errorInfo;

        SAP_UC *func_name = setString(functionName);

        RFC_FUNCTION_DESC_HANDLE func_desc_handle = RfcGetFunctionDesc(client_conn_handle, func_name, &errorInfo);

        if (errorInfo.code != RFC_OK)
        {
            free(func_name);
            callback.Call({rfcSdkError(&errorInfo)});
            return scope.Escape(info.Env().Undefined());
        }
        
        // Create thread-safe function from `jsFunction`
        ServerCallbackTsfn threadSafeFunction = ServerCallbackTsfn::New(
            node_rfc::__env,
            jsFunction,             // JavaScript function called asynchronously
            "ServerCallbackTsfn",   // Resource name
            0,                      // Unlimited queue
            1,                      // Only one thread will use this initially
            nullptr                 // No context needed
        );
				  
        ServerFunctionStruct sfs = ServerFunctionStruct(func_name, func_desc_handle, threadSafeFunction);

        free(func_name);

        serverFunctions[functionName.Utf8Value()] = sfs;
        DEBUG("Server::AddFunction added ", functionName.Utf8Value(), ": ", (pointer_t)func_desc_handle);

        callback.Call({});
        return scope.Escape(info.Env().Undefined());
    };

    Napi::Value Server::RemoveFunction(const Napi::CallbackInfo &info)
    {
        Napi::EscapableHandleScope scope(info.Env());

        std::ostringstream errmsg;

        if (!info[0].IsString())
        {
            errmsg << "Server removeFunction() requires ABAP RFM name; see" << USAGE_URL;
            Napi::TypeError::New(info.Env(), errmsg.str()).ThrowAsJavaScriptException();
            return info.Env().Undefined();
        }

        Napi::String functionName = info[0].As<Napi::String>();

        if (functionName.Utf8Value().length() == 0 || functionName.Utf8Value().length() > 30)
        {
            errmsg << "Server removeFunction() accepts max. 30 characters long ABAP RFM name; see" << USAGE_URL;
            Napi::TypeError::New(info.Env(), errmsg.str()).ThrowAsJavaScriptException();
            return info.Env().Undefined();
        }

        DEBUG("Server::RemoveFunction ", functionName.Utf8Value());

        if (!info[1].IsFunction())
        {
            errmsg << "Server removeFunction() requires a callback function; see" << USAGE_URL;
            Napi::TypeError::New(info.Env(), errmsg.str()).ThrowAsJavaScriptException();
            return info.Env().Undefined();
        }

        Napi::Function callback = info[1].As<Napi::Function>();

        // Remove function

        SAP_UC *func_name = setString(functionName);

        ServerFunctionsMap::iterator it = serverFunctions.begin();
        while (it != serverFunctions.end())
        {
            if (strcmpU(func_name, it->second.func_name) == 0)
            {
                break;
            }
            it++;
        }
        
        free(func_name);
        it->second.threadSafeCallback.Release();


        if (it == serverFunctions.end())
        {
            errmsg << "Server removeFunction() did not find function: " << functionName.Utf8Value();
            callback.Call({nodeRfcError(errmsg.str())});
            return scope.Escape(info.Env().Undefined());
        }

        DEBUG("Server::RemoveFunction removed ", functionName.Utf8Value(), ": ", (pointer_t)it->second.func_desc_handle);
        serverFunctions.erase(it);

        callback.Call({});
        return scope.Escape(info.Env().Undefined());
    };

    Napi::Value Server::GetFunctionDescription(const Napi::CallbackInfo &info)
    {
        DEBUG("Server::GetFunctionDescription");

        std::ostringstream errmsg;

        if (!info[0].IsString())
        {
            errmsg << "Server getFunctionDescription() requires ABAP RFM name; see" << USAGE_URL;
            Napi::TypeError::New(info.Env(), errmsg.str()).ThrowAsJavaScriptException();
            return info.Env().Undefined();
        }

        if (!info[0].IsFunction())
        {
            errmsg << "Server getFunctionDescription() requires a callback function; see" << USAGE_URL;
            Napi::TypeError::New(info.Env(), errmsg.str()).ThrowAsJavaScriptException();
            return info.Env().Undefined();
        }

        Napi::Function callback = info[1].As<Napi::Function>();

        (new GetFunctionDescAsync(callback, this))->Queue();

        return info.Env().Undefined();
    };

    Server::~Server(void)
    {
        DEBUG("~ Server ", id);

        uv_sem_destroy(&invocationMutex);
        if (serverHandle != NULL)
        {
            RfcShutdownServer(serverHandle, 60, NULL);
            RfcDestroyServer(serverHandle, NULL);
        }

        if (client_conn_handle != NULL)
        {
            RfcCloseConnection(client_conn_handle, NULL);
        }
    }

    void Server::LockMutex()
    {
        uv_sem_wait(&invocationMutex);
    }

    void Server::UnlockMutex()
    {
        uv_sem_post(&invocationMutex);
    }

} // namespace node_rfc

void ServerDoneCallback(const CallbackInfo& info)
{
    Env env = info.Env();
    ServerCallbackContainer* data = (ServerCallbackContainer*)info.Data();

    //
    // JS -> ABAP parameters
    //

    DEBUG("[NODE RFC] done() callback initiated @\n", (unsigned long)&data->wait_js_mutex);
    Napi::Value err = env.Undefined();
    Napi::Function callback;

    if (info.Length() > 0) {
        Napi::Object params = info[0].As<Napi::Object>();
        if(info.Length() > 1)
            callback = info[1].As<Napi::Function>();
        
        if (data->errorInfo->code == RFC_OK) {
            for (uint_t i = 0; i < data->paramCount; i++) {
                Napi::String name = data->paramNames.Value().Get(i).ToString();
                
                Napi::Value value = params.Get(name);
                err = node_rfc::setRfmParameter(data->func_desc_handle, data->func_handle, name, value, &data->errorPath, &data->client_options);

                if (!err.IsUndefined()) {
                    break;
                }
            }
        }
    }

    uv_mutex_lock(&data->js_running_mutex);
    bool js_running = data->js_running;
    uv_mutex_unlock(&data->js_running_mutex);

    if (!js_running) {
        DEBUG("[NODE RFC] Detected fake done() call\n"); // This happens if the user calls done more than once
        return;
    }

    uv_mutex_lock(&data->js_running_mutex);
    data->js_running = false;
    uv_mutex_unlock(&data->js_running_mutex);

    uv_mutex_lock(&data->wait_js_mutex);
    uv_cond_signal(&data->wait_js);
    uv_mutex_unlock(&data->wait_js_mutex);
    
    if(info.Length() > 1)
		    callback.Call({err});            
}

void ServerCallJs(Napi::Env env, Napi::Function callback, std::nullptr_t* context, ServerCallbackContainer* data)
{
    DEBUG("[NODE RFC] Callback BEGIN\n");

    //
    // ABAP -> JS parameters
    //

    auto func_desc_handle = data->func_desc_handle;
    auto func_handle = data->func_handle;

    node_rfc::ValuePair jsContainer = getRfmParameters(func_desc_handle, func_handle, &data->errorPath, &data->client_options);
    
    Napi::Object errorObj = jsContainer.first.As<Napi::Object>();
    Napi::Object abapArgs = jsContainer.second.As<Napi::Object>();
    Napi::Array paramNames = abapArgs.GetPropertyNames();


    data->paramNames = Napi::Persistent(paramNames);
    RfcGetParameterCount(data->func_desc_handle, &data->paramCount, data->errorInfo);

    // Is the JavaScript environment still available to call into, eg. the TSFN is
    // not aborted
    if (env != nullptr) {
        if (callback != nullptr) {
            DEBUG("[NODE RFC] Callback initiated\n");
            callback.Call({ errorObj, abapArgs, Napi::Function::New<ServerDoneCallback>(env, nullptr, data) });
            DEBUG("[NODE RFC] Callback returned\n");
        }
    }
}

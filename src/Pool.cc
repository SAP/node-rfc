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

#include "Pool.h"
#include "Throughput.h"

namespace node_rfc
{
    Napi::Env __env = NULL;

    uint_t Pool::_id = 1;

    class SetPoolAsync : public Napi::AsyncWorker
    {
    public:
        SetPoolAsync(Napi::Function &callback, Pool *pool, int32_t ready_low)
            : Napi::AsyncWorker(callback), pool(pool), ready_low(ready_low) {}
        ~SetPoolAsync() {}

        void Execute()
        {
            pool->lockMutex();
            DEBUG("SetPoolAsync %u", ready_low);
            uint_t ready = pool->connReady.size();
            DEBUG("SetPoolAsync ready_low: %u, ready: %u", ready_low, ready);
            errorInfo.code = RFC_OK;

            if (ready < ready_low)
            {
                DEBUG("Pool up: %u up to %u", ready, ready_low);
                for (uint_t ii = ready; ii < ready_low; ii++)
                {
                    connectionHandle = RfcOpenConnection(pool->client_params.connectionParams, pool->client_params.paramSize, &errorInfo);
                    if (errorInfo.code == RFC_OK)
                    {
                        DEBUG("SetAsync: %lu", (pointer_t)(connectionHandle));
                        pool->connReady.insert(connectionHandle);
                    }
                    else
                    {
                        break;
                    }
                }
            }

            /*
            if (errorInfo.code == RFC_OK)
            {
                ready = pool->connReady.size();
                if (ready > pool->ready_high)
                {
                    DEBUG("Pool down: %u to %u", ready, pool->ready_high);
                    ConnectionSetType::iterator it = pool->connReady.begin();
                    while (pool->connReady.size() > pool->ready_high)
                    {
                        pool->connReady.erase(it);
                        connectionHandle = *it++;
                        RfcCloseConnection(connectionHandle, &errorInfo);
                    }
                }
            }
            */
        }

        void OnOK()
        {
            Napi::HandleScope scope(Env());
            if (errorInfo.code != RFC_OK)
            {
                Callback().Call({wrapError(&errorInfo)});
            }
            else
            {
                Callback().Call({});
            }
            pool->unlockMutex();
        }

    private:
        Pool *pool;
        RFC_CONNECTION_HANDLE connectionHandle;
        RFC_ERROR_INFO errorInfo;
        uint_t ready_low;
    };
    class CheckPoolAsync : public Napi::AsyncWorker
    {
    public:
        CheckPoolAsync(Napi::Function &callback, Pool *pool)
            : Napi::AsyncWorker(callback), pool(pool) {}
        ~CheckPoolAsync() {}

        void Execute()
        {
            pool->lockMutex();
            DEBUG("CheckPoolAsync %u", pool->ready_low);
            uint_t ready = pool->connReady.size();
            errorInfo.code = RFC_OK;

            if (ready < pool->ready_low)
            {
                DEBUG("Pool up: %u up to %u", ready, pool->ready_low);
                for (uint_t ii = ready; ii < pool->ready_low; ii++)
                {
                    connectionHandle = RfcOpenConnection(pool->client_params.connectionParams, pool->client_params.paramSize, &errorInfo);
                    if (errorInfo.code == RFC_OK)
                    {
                        pool->connReady.insert(connectionHandle);
                    }
                    else
                    {
                        break;
                    }
                }
            }

            if (errorInfo.code == RFC_OK)
            {
                ready = pool->connReady.size();
                if (ready > pool->ready_high)
                {
                    DEBUG("Pool down: %u to %u", ready, pool->ready_high);
                    ConnectionSetType::iterator it = pool->connReady.begin();
                    while (pool->connReady.size() > pool->ready_high)
                    {
                        pool->connReady.erase(it);
                        connectionHandle = *it++;
                        RfcCloseConnection(connectionHandle, &errorInfo);
                    }
                }
            }
            pool->unlockMutex();
        }
        void OnOK()
        {
        }

    private:
        Pool *pool;
        RFC_CONNECTION_HANDLE connectionHandle;
        RFC_ERROR_INFO errorInfo;
    };

    class AcquireAsync : public Napi::AsyncWorker
    {
    public:
        friend class CheckPoolAsync;
        AcquireAsync(Napi::Function &callback, const uint_t clients_requested, Pool *pool)
            : Napi::AsyncWorker(callback), clients_requested(clients_requested), pool(pool) {}
        ~AcquireAsync() {}

        void Execute()
        {
            pool->lockMutex();

            it = pool->connReady.begin();

            uint_t ii = clients_requested;
            while (ii-- > 0)
            {
                if (it != pool->connReady.end())
                {
                    pool->connReady.erase(it);
                    connections.insert(*it++);
                }
                else
                {
                    connectionHandle = RfcOpenConnection(pool->client_params.connectionParams, pool->client_params.paramSize, &errorInfo);
                    error = errorInfo.code != RFC_OK;
                    if (error)
                    {
                        break;
                    }
                    else
                    {
                        connections.insert(connectionHandle);
                    }
                }
            }
        }

        void OnOK()
        {
            Napi::HandleScope scope(Env());
            if (error)
            {
                Callback().Call({wrapError(&errorInfo)});
            }
            else
            {
                Napi::Object jsclient;
                Napi::Array js_clients = Napi::Array::New(Env());

                it = connections.begin();
                uint_t ii = 0;
                while (it != connections.end())
                {
                    connectionHandle = *it++;
                    jsclient = Client::NewInstance(Env());
                    Client *client = Napi::ObjectWrap<Client>::Unwrap(jsclient);
                    // pool client_params not copied to client, connecton open() and close() managed by the pool
                    // pool client_options copied to client
                    client->client_options = pool->client_options;
                    // pool reference set, that client can notify on broken connections
                    client->pool = pool;
                    // connection handle set by pool
                    client->connectionHandle = connectionHandle;
                    // and added to leased connections
                    pool->connLeased.insert(connectionHandle);
                    js_clients.Set(ii++, jsclient);
                }

                if (js_clients.Length() == 1)
                {
                    Callback().Call({Env().Undefined(), jsclient});
                }
                else
                {
                    Callback().Call({Env().Undefined(), js_clients});
                }
            }
            pool->unlockMutex();
            Napi::Function fn = Callback().Value();
            (new CheckPoolAsync(fn, pool))->Queue();
        }

    private:
        uint_t clients_requested;
        Pool *pool;
        ConnectionSetType connections = {};
        ConnectionSetType::iterator it;
        bool error = false;
        RFC_CONNECTION_HANDLE connectionHandle;
        RFC_ERROR_INFO errorInfo;
    };

    class ReleaseAsync : public Napi::AsyncWorker
    {
    public:
        ReleaseAsync(Napi::Function &callback, Pool *pool, std::set<Client *> clients)
            : Napi::AsyncWorker(callback), pool(pool), clients(clients) {}
        ~ReleaseAsync() {}

        void Execute()
        {
            pool->lockMutex();
            std::set<Client *>::iterator client = clients.begin();
            while (client != clients.end())
            {
                RFC_CONNECTION_HANDLE connectionHandle = (*client)->connectionHandle;

                (*client)->connectionHandle = NULL;

                pool->connLeased.erase(connectionHandle);

                errorInfo.code = RFC_OK;

                if (pool->connReady.size() < pool->ready_high)
                {
                    (*client)->LockMutex();
                    DEBUG("ReleaseAsync ResetServerContext client: %u handle: %lu", (*client)->id, (pointer_t)(*client)->connectionHandle);
                    RfcResetServerContext(connectionHandle, &errorInfo);
                    (*client)->UnlockMutex();
                    if (errorInfo.code == RFC_OK)
                    {
                        pool->connReady.insert(connectionHandle);
                    }
                }
                else
                {
                    (*client)->LockMutex();
                    DEBUG("ReleaseAsync Close client: %u handle: %lu", (*client)->id, (pointer_t)connectionHandle);
                    RfcCloseConnection(connectionHandle, &errorInfo);
                    (*client)->UnlockMutex();
                    if (errorInfo.code != RFC_OK)
                    {
                        break;
                    }
                }
                client++;
            }
            pool->unlockMutex();
        }

        void OnOK()
        {
            Napi::HandleScope scope(Env());
            if (errorInfo.code != RFC_OK)
            {
                argv = wrapError(&errorInfo);
            }
            Callback().Call({argv});
        }

    private:
        Pool *pool;
        std::set<Client *> clients;
        Napi::Value argv = Env().Undefined();
        RFC_ERROR_INFO errorInfo;
    };

    /*
    class ReleaseClientAsync : public Napi::AsyncWorker
    {
    public:
        ReleaseClientAsync(Napi::Function &callback, Pool *pool, RFC_CONNECTION_HANDLE connectionHandle)
            : Napi::AsyncWorker(callback), pool(pool), connectionHandle(connectionHandle) {}
        ~ReleaseClientAsync() {}

        void Execute()
        {
            pool->lockMutex();
            removed = pool->connLeased.erase(connectionHandle);
            pool->unlockMutex();
        }

        void OnOK()
        {
            if (removed != 0)
            {
                DEBUG("Connection handle %lu removed from pool %u leased connections", (pointer_t)connectionHandle, pool->id);
            }
            else
            {
                DEBUG("Connection handle %lu not found in pool %u leased connections", (pointer_t)connectionHandle, pool->id);
            }
        }

    private:
        Pool *pool;
        RFC_CONNECTION_HANDLE connectionHandle;
        uint_t removed = 0;
    };
*/
    uint_t checkArgsAcquire(const Napi::CallbackInfo &info)
    {
        char errmsg[254];
        if (info.Length() != 2)
        {
            sprintf(errmsg, "Pool acquire() expects two arguments; see: %s", USAGE_URL);
            Napi::Error::New(info.Env(), errmsg).ThrowAsJavaScriptException();
        }

        if (!info[0].IsNumber())
        {
            sprintf(errmsg, "Pool acquire() first argument must be a number, got %s; see: %s", info[0].ToString().Utf8Value().c_str(), USAGE_URL);
            Napi::Error::New(info.Env(), errmsg).ThrowAsJavaScriptException();
        }

        if (!info[1].IsFunction())
        {
            sprintf(errmsg, "Pool acquire() second argument must be a callback function, got %s; see: %s", info[1].ToString().Utf8Value().c_str(), USAGE_URL);
            Napi::Error::New(info.Env(), errmsg).ThrowAsJavaScriptException();
        }

        return info[0].As<Napi::Number>().Uint32Value();
    }

    Napi::Value Pool::Acquire(const Napi::CallbackInfo &info)
    {

        uint_t clients_requested = checkArgsAcquire(info);

        DEBUG("Pool::Acquire: %u", clients_requested);

        Napi::Function callback = info[1].As<Napi::Function>();

        (new AcquireAsync(callback, clients_requested, this))->Queue();

        return info.Env().Undefined();
    }

    std::set<Client *> argsCheckRelease(const Napi::CallbackInfo &info)
    {
        std::set<Client *> clients = {};

        char errmsg[254];

        if (info.Length() != 2)
        {
            sprintf(errmsg, "Pool release() expects two arguments; see: %s", USAGE_URL);
            Napi::Error::New(info.Env(), errmsg).ThrowAsJavaScriptException();
        }

        if (!info[0].IsArray())
        {
            if (!info[0].IsObject())
            {
                sprintf(errmsg, "Pool release() first argument must be a single Client instance or array of Client instances; see: %s", USAGE_URL);
                Napi::Error::New(info.Env(), errmsg).ThrowAsJavaScriptException();
            }
            else
            {
                Napi::Object js_client = info[0].As<Napi::Object>();
                Client *client = Napi::ObjectWrap<Client>::Unwrap(js_client);
                clients.insert(client);
            }
        }
        else
        {
            Napi::Array js_clients = info[0].As<Napi::Array>();
            for (uint_t ii = 0; ii < js_clients.Length(); ii++)
            {
                Napi::Object js_client = js_clients.Get(ii).As<Napi::Object>();
                Client *client = Napi::ObjectWrap<Client>::Unwrap(js_client);
                clients.insert(client);
            }
        }

        if (!info[1].IsFunction())
        {
            sprintf(errmsg, "Pool release() second argument must be a callback function; see: %s", USAGE_URL);
            Napi::Error::New(info.Env(), errmsg).ThrowAsJavaScriptException();
        }

        return clients;
    }

    Napi::Value Pool::Release(const Napi::CallbackInfo &info)
    {
        DEBUG("Pool::Release");

        std::set<Client *> clients = argsCheckRelease(info);

        Napi::Function callback = info[1].As<Napi::Function>();

        (new ReleaseAsync(callback, this, clients))->Queue();

        return info.Env().Undefined();
    }

    void Pool::releaseClient(RFC_CONNECTION_HANDLE connectionHandle)
    {
        // synchronous because called with locked client mutex or from client destructor
        if (connLeased.erase(connectionHandle) == 0)
        {
            DEBUG("Pool %u did not find the connection %lu in leased set", id, (pointer_t)connectionHandle);
        }
        else
        {
            DEBUG("Pool %u released connection %lu", id, (pointer_t)connectionHandle);
        }
    }

    bool argsCheckReady(const Napi::CallbackInfo &info, uint_t *new_ready, Napi::Function *callback)
    {
        char errmsg[254];

        uint_t ii = info.Length();

        while (ii-- > 0)
        {
            if (info[ii].IsNumber())
            {
                int32_t n = info[ii].As<Napi::Number>().Int32Value();
                if (n < 0)
                {
                    sprintf(errmsg, "Pool ready() number of connections argument must be positive, received %d; see: %s", n, USAGE_URL);
                    Napi::Error::New(info.Env(), errmsg).ThrowAsJavaScriptException();
                    return false;
                }
                *new_ready = n;
            }
            else if (info[ii].IsFunction())
            {
                *callback = info[ii].As<Napi::Function>();
            }
            else if (!info[ii].IsUndefined())
            {
                sprintf(errmsg, "Pool ready() argument is neiter a number, nor a function; see: %s", USAGE_URL);
                Napi::Error::New(info.Env(), errmsg).ThrowAsJavaScriptException();
                return false;
            }
        }
        return true;
    }

    Napi::Value Pool::Ready(const Napi::CallbackInfo &info)
    {
        uint_t new_ready = ready_low;
        Napi::Function callback;
        DEBUG("Pool::Ready: %u", new_ready);

        if (argsCheckReady(info, &new_ready, &callback))
        {
            (new SetPoolAsync(callback, this, new_ready))->Queue();
        }
        return info.Env().Undefined();
    }

    Napi::Value Pool::CloseAll(const Napi::CallbackInfo &info)
    {
        DEBUG("Pool::CloseAll %u", id);

        closeConnections();

        if (!info[0].IsUndefined())
        {
            if (info[0].IsFunction())
            {
                info[0].As<Napi::Function>().Call({});
            }
            else
            {
                char errmsg[254];
                sprintf(errmsg, "Pool closeAll argument, if provided, must be a function; see: %s", USAGE_URL);
                Napi::Error::New(info.Env(), errmsg).ThrowAsJavaScriptException();
            }
        }

        return info.Env().Undefined();
    }

    Napi::Object Pool::Init(Napi::Env env, Napi::Object exports)
    {
        DEBUG("Pool::Init");
        Napi::HandleScope scope(env);

        Napi::Function func = DefineClass(env,
                                          "Pool", {
                                                      InstanceAccessor("_id", &Pool::IdGetter, nullptr),
                                                      InstanceAccessor("_config", &Pool::ConfigGetter, nullptr),
                                                      InstanceAccessor("_status", &Pool::StatusGetter, nullptr),
                                                      InstanceMethod("acquire", &Pool::Acquire),
                                                      InstanceMethod("release", &Pool::Release),
                                                      InstanceMethod("ready", &Pool::Ready),
                                                      InstanceMethod("closeAll", &Pool::CloseAll),
                                                  });

        Napi::FunctionReference *constructor = new Napi::FunctionReference();
        *constructor = Napi::Persistent(func);
        constructor->SuppressDestruct();

        exports.Set("Pool", func);
        return exports;
    }

    Napi::Value Pool::IdGetter(const Napi::CallbackInfo &info)
    {
        return Napi::Number::New(Env(), id);
    }

    Pool::Pool(const Napi::CallbackInfo &info) : Napi::ObjectWrap<Pool>(info)
    {
        Napi::Env env = info.Env();
        Napi::HandleScope scope(env);

        init(env);

        char errmsg[254];

        DEBUG("Pool::Pool %u %u", ready_low, ready_high);
        if (info.Length() < 1)
        {
            sprintf(errmsg, "Pool initialization argument missing; see: %s", USAGE_URL);
            Napi::Error::New(info.Env(), errmsg).ThrowAsJavaScriptException();
        }

        if (info.Length() > 1)
        {
            sprintf(errmsg, "Too many Pool initialization arguments; see: %s", USAGE_URL);
            Napi::Error::New(info.Env(), errmsg).ThrowAsJavaScriptException();
        }

        if (!info[0].IsObject())
        {
            sprintf(errmsg, "Pool initialization argument not an object; see: %s", USAGE_URL);
            Napi::Error::New(info.Env(), errmsg).ThrowAsJavaScriptException();
        }

        poolConfiguration = Napi::Persistent(info[0].As<Napi::Object>());
        if (!poolConfiguration.Value().Has(POOL_KEY_CONNECTION_PARAMS))
        {
            sprintf(errmsg, "Pool initialization object w/o \"connectionParameters\" key; see %s", USAGE_URL);
            Napi::Error::New(info.Env(), errmsg).ThrowAsJavaScriptException();
        }

        Napi::Array poolKeys = poolConfiguration.Value().GetPropertyNames();
        for (uint_t i = 0; i < poolKeys.Length(); i++)
        {
            std::string key = poolKeys.Get(i).As<Napi::String>().Utf8Value();

            //
            // Client connection parameters
            //
            if (key.compare(std::string(POOL_KEY_CONNECTION_PARAMS)) == 0)
            {
                if (!poolConfiguration.Get(key).IsObject())
                {
                    sprintf(errmsg, "Pool \"%s\" not an object; see: %s", POOL_KEY_CONNECTION_PARAMS, USAGE_URL);
                    Napi::Error::New(info.Env(), errmsg).ThrowAsJavaScriptException();
                }

                connectionParameters = Napi::Persistent(poolConfiguration.Get(POOL_KEY_CONNECTION_PARAMS).As<Napi::Object>());

                checkConnectionParams(connectionParameters.Value(), &client_params);
            }

            //
            // Client options
            //
            else if (key.compare(std::string(POOL_KEY_CLIENT_OPTIONS)) == 0)
            {
                if (!poolConfiguration.Get(key).IsObject())
                {
                    sprintf(errmsg, "Pool \"%s\" not an object; see: %s", POOL_KEY_CLIENT_OPTIONS, USAGE_URL);
                    Napi::Error::New(info.Env(), errmsg).ThrowAsJavaScriptException();
                }

                clientOptions = Napi::Persistent(poolConfiguration.Get(key).As<Napi::Object>());

                checkClientOptions(clientOptions.Value(), &client_options);
            }

            //
            // Pool options
            //
            else if (key.compare(std::string(POOL_KEY_POOL_OPTIONS)) == 0)
            {
                if (!poolConfiguration.Get(POOL_KEY_POOL_OPTIONS).IsObject())
                {
                    sprintf(errmsg, "Pool \"%s\" not an object; see: %s", POOL_KEY_POOL_OPTIONS, USAGE_URL);
                    Napi::Error::New(info.Env(), errmsg).ThrowAsJavaScriptException();
                }

                poolOptions = Napi::Persistent(poolConfiguration.Get(POOL_KEY_POOL_OPTIONS).As<Napi::Object>());
                Napi::Array props = poolOptions.Value().GetPropertyNames();
                for (uint_t n = 0; n < props.Length(); n++)
                {
                    std::string key = props.Get(n).ToString().Utf8Value();
                    if (!poolOptions.Get(key).IsNumber())
                    {
                        sprintf(errmsg, "Pool option \"%s\" must be a number; see %s", key.c_str(), USAGE_URL);
                        Napi::TypeError::New(env, errmsg).ThrowAsJavaScriptException();
                    }
                    if (key.compare(POOL_KEY_OPTION_LOW) == 0)
                    {
                        ready_low = poolOptions.Get(key).As<Napi::Number>();
                        if (ready_low < 0)
                        {
                            sprintf(errmsg, "Pool option \"%s\" must not be negative; see %s", key.c_str(), USAGE_URL);
                            Napi::TypeError::New(env, errmsg).ThrowAsJavaScriptException();
                        }
                    }
                    else if (key.compare(POOL_KEY_OPTION_HIGH) == 0)
                    {
                        ready_high = poolOptions.Get(key).As<Napi::Number>();
                        if (ready_high < 1)
                        {
                            sprintf(errmsg, "Pool option \"%s\" must not greater than zero; see %s", key.c_str(), USAGE_URL);
                            Napi::TypeError::New(env, errmsg).ThrowAsJavaScriptException();
                        }
                    }
                    else
                    {
                        sprintf(errmsg, "Pool option not allowed: \"%s\"; see %s", key.c_str(), USAGE_URL);
                        Napi::TypeError::New(env, errmsg).ThrowAsJavaScriptException();
                    }
                }
                if (ready_low > ready_high)
                {
                    sprintf(errmsg, "Pool option \"%s\": %u > \"%s\": %u; see %s", POOL_KEY_OPTION_LOW, ready_low, POOL_KEY_OPTION_HIGH, ready_high, USAGE_URL);
                    Napi::TypeError::New(env, errmsg).ThrowAsJavaScriptException();
                }
            }

            //
            // Unknown option
            //
            else
            {
                sprintf(errmsg, "Pool initialization object key not allowed: \"%s\"; see: https://github.com/SAP/node-rfc#usage", &key[0]);
                Napi::TypeError::New(info.Env(), errmsg).ThrowAsJavaScriptException();
            }
        }
        DEBUG("Pool::Pool %u %u", ready_low, ready_high);
    };

    void Pool::closeConnections()
    {
        // Close connections
        if (connReady.size() > 0)
        {
            DEBUG("~ Pool %u closing ready connections: %lu", id, connReady.size());
            ConnectionSetType::iterator it = connReady.begin();
            while (it != connReady.end())
            {
                RFC_ERROR_INFO errorInfo;
                RFC_CONNECTION_HANDLE connectionHandle = *it;
                RfcCloseConnection(connectionHandle, &errorInfo);
                if (errorInfo.code == RFC_OK)
                {
                    DEBUG("    closed %lu", (pointer_t)connectionHandle);
                }
                else
                {
                    DEBUG("    error closing %lu: group: %u code: %u", (pointer_t)connectionHandle, errorInfo.group, errorInfo.code);
                }
                connReady.erase(it++);
            }
        }

        if (connLeased.size() > 0)
        {
            DEBUG("~ Pool %u closing leased connections: %lu", id, connLeased.size());
            ConnectionSetType::iterator it = connLeased.begin();
            while (it != connLeased.end())
            {
                RFC_ERROR_INFO errorInfo;
                RFC_CONNECTION_HANDLE connectionHandle = *it;
                RfcCloseConnection(connectionHandle, &errorInfo);
                if (errorInfo.code == RFC_OK)
                {
                    DEBUG("    closed %lu", (pointer_t)connectionHandle);
                }
                else
                {
                    DEBUG("    error closing %lu: group: %u code: %u", (pointer_t)connectionHandle, errorInfo.group, errorInfo.code);
                }
                connReady.erase(it++);
            }
        }
    }

    Pool::~Pool(void)
    {
        DEBUG("~Pool %u", id);

        closeConnections();

        // Unreference configuration
        if (!connectionParameters.IsEmpty())
        {
            DEBUG("~ Pool::Pool unref connectionParameters");
            connectionParameters.Unref();
        }
        if (!clientOptions.IsEmpty())
        {
            DEBUG("~ Pool::Pool unref clientOptions");
            clientOptions.Unref();
        }
        if (!poolOptions.IsEmpty())
        {
            DEBUG("~ Pool::Pool unref poolOptions");
            poolOptions.Unref();
        }
        if (!poolConfiguration.IsEmpty())
        {
            poolConfiguration.Unref();
            DEBUG("~ Pool::Pool unref poolConfiguration");
        }

        // Close mutex
        uv_sem_destroy(&leaseMutex);
    }

    void Pool::lockMutex()
    {
        uv_sem_wait(&leaseMutex);
    }

    void Pool::unlockMutex()
    {
        uv_sem_post(&leaseMutex);
    }

    Napi::Value BindingVersions(Napi::Env env)
    {
        uint_t major, minor, patchLevel;
        Napi::EscapableHandleScope scope(env);

        RfcGetVersion(&major, &minor, &patchLevel);

        Napi::Object nwrfcsdk = Napi::Object::New(env);
        nwrfcsdk.Set("major", major);
        nwrfcsdk.Set("minor", minor);
        nwrfcsdk.Set("patchLevel", patchLevel);

        Napi::Object version = Napi::Object::New(env);
        version.Set("version", NODERFC_VERSION);
        version.Set("nwrfcsdk", nwrfcsdk);

        return scope.Escape(version);
    }

    Napi::Value Pool::ConfigGetter(const Napi::CallbackInfo &info)
    {
        return poolConfiguration.Value();
    }

    Napi::Value Pool::StatusGetter(const Napi::CallbackInfo &info)
    {
        Napi::EscapableHandleScope scope(info.Env());
        Napi::Object status = Napi::Object::New(info.Env());
        status.Set("ready", Napi::Number::New(info.Env(), (double)connReady.size()));
        status.Set("leased", Napi::Number::New(info.Env(), (double)connLeased.size()));
        return scope.Escape(status);
    }

    Napi::Object RegisterModule(Napi::Env env, Napi::Object exports)
    {
        exports.Set("bindingVersions", BindingVersions(env));

        Pool::Init(env, exports);
        Client::Init(env, exports);
        Throughput::Init(env, exports);

        return exports;
    }

    NODE_API_MODULE(NODE_GYP_MODULE_NAME, RegisterModule);

} // namespace node_rfc

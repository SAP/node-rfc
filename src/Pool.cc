// SPDX-FileCopyrightText: 2014 SAP SE Srdjan Boskovic <srdjan.boskovic@sap.com>
//
// SPDX-License-Identifier: Apache-2.0

#include "Pool.h"
#include "Throughput.h"

namespace node_rfc
{
    uint_t Pool::_id = 1;

    class CheckPoolAsync : public Napi::AsyncWorker
    {
    public:
        CheckPoolAsync(Napi::Function &callback, Pool *pool)
            : Napi::AsyncWorker(callback), pool(pool) {}
        ~CheckPoolAsync() {}

        void Execute()
        {
            pool->lockMutex();

            uint_t ready = pool->connReady.size();

            // DEBUG("CheckPoolAsync ready_low: ", pool->ready_low, "ready: ", ready);

            if (ready < pool->ready_low)
            {
                // DEBUG("  Pool up: ", ready, " to ", pool->ready_low);
                for (uint_t ii = ready; ii < pool->ready_low; ii++)
                {
                    RFC_ERROR_INFO errorInfo;
                    RFC_CONNECTION_HANDLE connectionHandle = RfcOpenConnection(pool->client_params.connectionParams, pool->client_params.paramSize, &errorInfo);
                    if (errorInfo.code == RFC_OK)
                    {
                        // DEBUG("    new handle: ", (pointer_t)(connectionHandle));
                        pool->connReady.insert(connectionHandle);
                    }
                    else
                    {
                        break;
                    }
                }
            }
            pool->unlockMutex();
        }

        void OnOK() {}

    private:
        Pool *pool;
    };

    class SetPoolAsync : public Napi::AsyncWorker
    {
    public:
        SetPoolAsync(Napi::Function &callback, Pool *pool, int32_t ready_low)
            : Napi::AsyncWorker(callback), pool(pool), ready_low(ready_low) {}
        ~SetPoolAsync() {}

        void Execute()
        {
            pool->lockMutex();
            uint_t ready = pool->connReady.size();
            // DEBUG("SetPoolAsync new ready_low: ", ready_low, " ready: ", ready);
            errorInfo.code = RFC_OK;

            if (ready < ready_low)
            {
                // DEBUG("Pool up: ", ready, " to ", ready_low);
                for (uint_t ii = ready; ii < ready_low; ii++)
                {
                    connectionHandle = RfcOpenConnection(pool->client_params.connectionParams, pool->client_params.paramSize, &errorInfo);
                    if (errorInfo.code == RFC_OK)
                    {
                        // DEBUG("    new handle: ", (pointer_t)(connectionHandle));
                        pool->connReady.insert(connectionHandle);
                    }
                    else
                    {
                        break;
                    }
                }
            }
        }

        void OnOK()
        {
            Napi::HandleScope scope(Env());
            if (errorInfo.code != RFC_OK)
            {
                Callback().Call({rfcSdkError(&errorInfo)});
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
    class AcquireAsync : public Napi::AsyncWorker
    {
    public:
        AcquireAsync(Napi::Function &callback, const uint_t clients_requested, Pool *pool)
            : Napi::AsyncWorker(callback), clients_requested(clients_requested), pool(pool) {}
        ~AcquireAsync()
        {
        }

        void Execute()
        {
            pool->lockMutex();
            errorInfo.code = RFC_OK;

            uint_t ii = clients_requested;

            it = pool->connReady.begin();
            while (ii-- > 0)
            {
                if (it != pool->connReady.end())
                {
                    ready_connections.insert(*it);
                    pool->connReady.erase(it++);
                }
                else
                {
                    connectionHandle = RfcOpenConnection(pool->client_params.connectionParams, pool->client_params.paramSize, &errorInfo);
                    if (errorInfo.code != RFC_OK)
                    {
                        break;
                    }
                    else
                    {
                        new_connections.insert(connectionHandle);
                    }
                }
            }
            if (errorInfo.code != RFC_OK)
            {
                // rollback ready
                it = ready_connections.begin();
                while (it != ready_connections.end())
                {
                    pool->connReady.insert(*it++);
                }

                // rollback new
                it = new_connections.begin();
                while (it != new_connections.end())
                {
                    RFC_ERROR_INFO ei;
                    RfcCloseConnection(*it, &ei);
                }
            }
            else
            {
                connections.insert(new_connections.begin(), new_connections.end());
                connections.insert(ready_connections.begin(), ready_connections.end());
            }
        }

        void OnOK()
        {
            Napi::HandleScope scope(Env());
            if (errorInfo.code != RFC_OK)
            {
                pool->unlockMutex();
                Callback().Call({rfcSdkError(&errorInfo)});
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
                pool->unlockMutex();

                if (js_clients.Length() == 1)
                {
                    Callback().Call({Env().Undefined(), jsclient});
                }
                else
                {
                    Callback().Call({Env().Undefined(), js_clients});
                }
            }

            Napi::Function fn = Callback().Value();
            (new CheckPoolAsync(fn, pool))->Queue();
        }

    private:
        uint_t clients_requested;
        Pool *pool;
        ConnectionSetType connections = {};
        ConnectionSetType ready_connections = {};
        ConnectionSetType new_connections = {};
        ConnectionSetType::iterator it;
        RFC_CONNECTION_HANDLE connectionHandle;
        RFC_ERROR_INFO errorInfo;
    };

    class ReleaseAsync : public Napi::AsyncWorker
    {
    public:
        ReleaseAsync(Napi::Function &callback, Pool *pool, std::set<Client *> clients)
            : Napi::AsyncWorker(callback), pool(pool), clients(clients) {}
        ~ReleaseAsync()
        {
        }

        void Execute()
        {
            pool->lockMutex();
            std::set<Client *>::iterator client = clients.begin();

            // check if all clients open
            while (client != clients.end())
            {
                if ((*client)->connectionHandle == NULL)
                {
                    closed_client_id = (*client)->id;
                    break;
                }
                client++;
            }

            if (closed_client_id == 0)
            {
                client = clients.begin();
                while (client != clients.end())
                {
                    RFC_CONNECTION_HANDLE connectionHandle = (*client)->connectionHandle;

                    pool->connLeased.erase(connectionHandle);

                    errorInfo.code = RFC_OK;

                    if (pool->connReady.size() < pool->ready_high)
                    {
                        (*client)->LockMutex();
                        (*client)->connectionHandle = NULL;
                        // DEBUG("ReleaseAsync ResetServerContext client: %u handle: %lu", (*client)->id, (pointer_t)connectionHandle);
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
                        (*client)->connectionHandle = NULL;
                        // DEBUG("ReleaseAsync Close client: %u handle: %lu", (*client)->id, (pointer_t)connectionHandle);
                        RfcCloseConnection(connectionHandle, &errorInfo);
                        (*client)->UnlockMutex();
                        if (errorInfo.code != RFC_OK)
                        {
                            break;
                        }
                    }
                    client++;
                }
            }
            pool->unlockMutex();
        }

        void OnOK()
        {
            Napi::HandleScope scope(Env());
            Napi::Value argv = Env().Undefined();
            if (closed_client_id != 0)
            {
                argv = nodeRfcError("Client release() invoked for already closed client");
            }
            else if (errorInfo.code != RFC_OK)
            {
                argv = rfcSdkError(&errorInfo);
            }
            Callback().Call({argv});
        }

    private:
        Pool *pool;
        std::set<Client *> clients;
        RFC_ERROR_INFO errorInfo;
        uint_t closed_client_id = 0;
    };

    uint_t checkArgsAcquire(const Napi::CallbackInfo &info)
    {
        std::ostringstream errmsg;
        uint_t clients_requested = 0;

        if (info.Length() != 2)
        {
            errmsg << "Pool acquire() expects two arguments; see: " << USAGE_URL;
            Napi::Error::New(info.Env(), errmsg.str()).ThrowAsJavaScriptException();
            return clients_requested;
        }

        if (!info[0].IsNumber())
        {
            errmsg << "Pool acquire() first argument must be a number, got " << info[0].ToString().Utf8Value() << "; see: " << USAGE_URL;
            Napi::Error::New(info.Env(), errmsg.str()).ThrowAsJavaScriptException();
            return clients_requested;
        }

        double arg = info[0].As<Napi::Number>().DoubleValue();
        if (arg < 1)
        {

            errmsg << "Pool acquire() first argument must be greater or equal 1, got " << arg << "; see: " << USAGE_URL;
            Napi::Error::New(info.Env(), errmsg.str()).ThrowAsJavaScriptException();
            return clients_requested;
        }

        if (!info[1].IsFunction())
        {
            errmsg << "Pool acquire() second argument must be a callback function, got " << info[1].ToString().Utf8Value() << "; see: " << USAGE_URL;
            Napi::Error::New(info.Env(), errmsg.str()).ThrowAsJavaScriptException();
            return clients_requested;
        }

        clients_requested = info[0].As<Napi::Number>().Uint32Value();

        return clients_requested;
    }

    Napi::Value Pool::Acquire(const Napi::CallbackInfo &info)
    {
        uint_t clients_requested = checkArgsAcquire(info);

        DEBUG("Pool::Acquire: ", clients_requested);

        Napi::Function callback = info[1].As<Napi::Function>();

        if (clients_requested > 0)
        {
            (new AcquireAsync(callback, clients_requested, this))->Queue();
        }

        return info.Env().Undefined();
    }

    std::set<Client *> argsCheckRelease(const Napi::CallbackInfo &info)
    {
        std::set<Client *> clients = {};

        std::ostringstream errmsg;

        if (info.Length() != 2)
        {
            errmsg << "Pool release() expects two arguments; see: " << USAGE_URL;
            Napi::Error::New(info.Env(), errmsg.str()).ThrowAsJavaScriptException();
            return clients;
        }

        if (!info[0].IsArray())
        {
            errmsg << "Pool release() first argument must be an array of Client instances; see: " << USAGE_URL;
            Napi::Error::New(info.Env(), errmsg.str()).ThrowAsJavaScriptException();
            return clients;
        }

        Napi::Array js_clients = info[0].As<Napi::Array>();
        for (uint_t ii = 0; ii < js_clients.Length(); ii++)
        {
            Napi::Object js_client = js_clients.Get(ii).As<Napi::Object>();
            Client *client = Napi::ObjectWrap<Client>::Unwrap(js_client);
            clients.insert(client);
        }

        if (!info[1].IsFunction())
        {
            errmsg << "Pool release() 2nd argument, if provided, must be a callback function; see: " << USAGE_URL;
            Napi::Error::New(info.Env(), errmsg.str()).ThrowAsJavaScriptException();
            return clients;
        }

        return clients;
    }

    Napi::Value Pool::Release(const Napi::CallbackInfo &info)
    {
        DEBUG("Pool::Release");

        std::set<Client *> clients = argsCheckRelease(info);

        if (clients.size() > 0)
        {
            Napi::Function callback = info[1].As<Napi::Function>();

            (new ReleaseAsync(callback, this, clients))->Queue();
        }

        return info.Env().Undefined();
    }

    void Pool::releaseClient(RFC_CONNECTION_HANDLE connectionHandle)
    {
        // synchronous because called with locked client mutex or from client destructor
        if (connLeased.erase(connectionHandle) == 0)
        {
            EDEBUG("Warning: Connection ", (pointer_t)connectionHandle, " not found in Pool ", id, " leased set");
        }
        else
        {
            DEBUG("Connection ", (pointer_t)connectionHandle, " released from Pool ", id);
        }
    }

    std::string Pool::updateLeasedHandle(RFC_CONNECTION_HANDLE old_handle, RFC_CONNECTION_HANDLE new_handle)
    {
        lockMutex();
        if (connLeased.erase(old_handle) != 0)
        {
            connLeased.insert(new_handle);
            unlockMutex();
            return "";
        }
        unlockMutex();
        std::ostringstream errmsg;
        errmsg << "The connection handle " << (pointer_t)old_handle << " not found in Pool leased connections, to be replaced by " << (uintptr_t)new_handle;
        return errmsg.str();
    }

    bool argsCheckReady(const Napi::CallbackInfo &info, uint_t *new_ready, Napi::Function *callback)
    {
        char errmsg[ERRMSG_LENGTH];

        uint_t ii = info.Length();

        while (ii-- > 0)
        {
            if (info[ii].IsNumber())
            {
                int32_t n = info[ii].As<Napi::Number>().Int32Value();
                if (n < 0)
                {
                    snprintf(errmsg, ERRMSG_LENGTH - 1, "Pool ready() number of connections argument must be positive, received %d; see: %s", n, USAGE_URL);
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
                snprintf(errmsg, ERRMSG_LENGTH - 1, "Pool ready() argument is neiter a number, nor a function; see: %s", USAGE_URL);
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
        DEBUG("Pool::Ready: ", new_ready);

        if (argsCheckReady(info, &new_ready, &callback))
        {
            (new SetPoolAsync(callback, this, new_ready))->Queue();
        }
        return info.Env().Undefined();
    }

    Napi::Value Pool::CloseAll(const Napi::CallbackInfo &info)
    {
        DEBUG("Pool::CloseAll ", id);

        closeConnections();

        if (!info[0].IsUndefined())
        {
            if (info[0].IsFunction())
            {
                info[0].As<Napi::Function>().Call({});
            }
            else
            {
                char errmsg[ERRMSG_LENGTH];
                snprintf(errmsg, ERRMSG_LENGTH - 1, "Pool closeAll argument, if provided, must be a function; see: %s", USAGE_URL);
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

        std::ostringstream errmsg;

        DEBUG("Pool::Pool ", ready_low, ready_high);
        if (info.Length() < 1)
        {
            errmsg << "Pool initialization argument missing; see: " << USAGE_URL;
            Napi::Error::New(info.Env(), errmsg.str()).ThrowAsJavaScriptException();
            return;
        }

        if (info.Length() > 1)
        {
            errmsg << "Too many Pool initialization arguments; see: " << USAGE_URL;
            Napi::Error::New(info.Env(), errmsg.str()).ThrowAsJavaScriptException();
            return;
        }

        if (!info[0].IsObject())
        {
            errmsg << "Pool initialization argument not an object; see: " << USAGE_URL;
            Napi::Error::New(info.Env(), errmsg.str()).ThrowAsJavaScriptException();
        }

        poolConfiguration = Napi::Persistent(info[0].As<Napi::Object>());

        if (!poolConfiguration.Value().Has(POOL_KEY_CONNECTION_PARAMS))
        {
            errmsg << "Pool configuration object must provide \"connectionParameters\"; see" << USAGE_URL;
            Napi::Error::New(info.Env(), errmsg.str()).ThrowAsJavaScriptException();
            return;
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
                    errmsg << "Pool() \"" << POOL_KEY_CONNECTION_PARAMS << "\" not an object; see: " << USAGE_URL;
                    Napi::Error::New(info.Env(), errmsg.str()).ThrowAsJavaScriptException();
                    return;
                }

                connectionParameters = Napi::Persistent(poolConfiguration.Get(POOL_KEY_CONNECTION_PARAMS).As<Napi::Object>());

                getConnectionParams(connectionParameters.Value(), &client_params);
            }

            //
            // Client options
            //
            else if (key.compare(std::string(POOL_KEY_CLIENT_OPTIONS)) == 0)
            {
                if (!poolConfiguration.Get(key).IsObject())
                {
                    errmsg << "Pool() \"" << POOL_KEY_CLIENT_OPTIONS << "\" not an object; see: " << USAGE_URL;
                    Napi::Error::New(info.Env(), errmsg.str()).ThrowAsJavaScriptException();
                    return;
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
                    errmsg << "Pool \"" << POOL_KEY_POOL_OPTIONS << "\"not an object; see: " << USAGE_URL;
                    Napi::Error::New(info.Env(), errmsg.str()).ThrowAsJavaScriptException();
                    return;
                }

                poolOptions = Napi::Persistent(poolConfiguration.Get(POOL_KEY_POOL_OPTIONS).As<Napi::Object>());
                Napi::Array props = poolOptions.Value().GetPropertyNames();
                for (uint_t n = 0; n < props.Length(); n++)
                {
                    std::string key = props.Get(n).ToString().Utf8Value();
                    if (!poolOptions.Get(key).IsNumber())
                    {
                        errmsg << "Pool() option \"" << key << "\" must be a number; see" << USAGE_URL;
                        Napi::TypeError::New(env, errmsg.str()).ThrowAsJavaScriptException();
                        return;
                    }
                    if (key.compare(POOL_KEY_OPTION_LOW) == 0)
                    {
                        ready_low = poolOptions.Get(key).As<Napi::Number>();
                        if (ready_low < 0)
                        {
                            errmsg << "Pool option \"" << key << "\" must not be negative; see" << USAGE_URL;
                            Napi::TypeError::New(env, errmsg.str()).ThrowAsJavaScriptException();
                            return;
                        }
                    }
                    else if (key.compare(POOL_KEY_OPTION_HIGH) == 0)
                    {
                        ready_high = poolOptions.Get(key).As<Napi::Number>();
                        if (ready_high < 1)
                        {
                            errmsg << "Pool option \"" << key << "\" must be greater than zero; see" << USAGE_URL;
                            Napi::TypeError::New(env, errmsg.str()).ThrowAsJavaScriptException();
                            return;
                        }
                    }
                    else
                    {
                        errmsg << "Pool option not allowed: \"" << key << "\"; see" << USAGE_URL;
                        Napi::TypeError::New(env, errmsg.str()).ThrowAsJavaScriptException();
                        return;
                    }
                }
                if (ready_low > ready_high)
                {
                    errmsg << "Pool option \"" << POOL_KEY_OPTION_LOW << "\": " << ready_low << ", must not be greater than \"" << POOL_KEY_OPTION_HIGH << "\": " << ready_high << ";see" << ready_high << USAGE_URL;
                    Napi::TypeError::New(env, errmsg.str()).ThrowAsJavaScriptException();
                    return;
                }
            }

            //
            // Unknown option
            //
            else
            {
                errmsg << "Pool initialization object key not allowed: \"" << key << "\"; see: " << USAGE_URL;
                Napi::TypeError::New(info.Env(), errmsg.str()).ThrowAsJavaScriptException();
                return;
            }
        }

        DEBUG("Pool::Pool ", ready_low, ready_high);
    };

    void Pool::closeConnections()
    {
        // Close connections
        if (connReady.size() > 0)
        {
            DEBUG("~ Pool ", id, " closing ready connections: ", connReady.size());
            ConnectionSetType::iterator it = connReady.begin();
            while (it != connReady.end())
            {
                RFC_ERROR_INFO errorInfo;
                RFC_CONNECTION_HANDLE connectionHandle = *it;
                RfcCloseConnection(connectionHandle, &errorInfo);
                if (errorInfo.code == RFC_OK)
                {
                    DEBUG("    closed ", (pointer_t)connectionHandle);
                }
                else
                {
                    EDEBUG("    Warning: error closing ", (pointer_t)connectionHandle, " group: ", errorInfo.group, "code: ", errorInfo.code);
                }
                connReady.erase(it++);
            }
        }

        if (connLeased.size() > 0)
        {
            DEBUG("~ Pool ", id, " closing leased connections: ", connLeased.size());
            ConnectionSetType::iterator it = connLeased.begin();
            while (it != connLeased.end())
            {
                RFC_ERROR_INFO errorInfo;
                RFC_CONNECTION_HANDLE connectionHandle = *it;
                RfcCloseConnection(connectionHandle, &errorInfo);
                if (errorInfo.code == RFC_OK)
                {
                    DEBUG("    closed ", (pointer_t)connectionHandle);
                }
                else
                {
                    EDEBUG("    Warning: error closing ", (pointer_t)connectionHandle, " group: ", errorInfo.group, "code: ", errorInfo.code);
                }
                connReady.erase(it++);
            }
        }
    }

    Pool::~Pool(void)
    {
        DEBUG("~Pool ", id);

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

} // namespace node_rfc

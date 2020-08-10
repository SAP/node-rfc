// SPDX-FileCopyrightText: 2014 SAP SE Srdjan Boskovic <srdjan.boskovic@sap.com>
//
// SPDX-License-Identifier: Apache-2.0

#include "Client.h"
#include "Pool.h"
#include "Throughput.h"
#include "Server.h"

namespace node_rfc
{
    Napi::Env __env = NULL;

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

    Napi::Object RegisterModule(Napi::Env env, Napi::Object exports)
    {
        exports.Set("bindingVersions", BindingVersions(env));

        Pool::Init(env, exports);
        Client::Init(env, exports);
        Throughput::Init(env, exports);
        Server::Init(env, exports);

        return exports;
    }

    NODE_API_MODULE(NODE_GYP_MODULE_NAME, RegisterModule);
} // namespace node_rfc

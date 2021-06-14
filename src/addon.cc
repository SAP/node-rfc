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

    Napi::Value LoadCryptoLibrary(const Napi::CallbackInfo &info)
    {
        if (!info[0].IsString())
        {
            std::ostringstream errmsg;
            errmsg << "Client setIniPath() requires the directory in which to search for the sapnwrfc.ini file, received: ";
            errmsg << info[0].As<Napi::String>().Utf8Value() << "; see" << USAGE_URL;
            Napi::TypeError::New(info.Env(), errmsg.str()).ThrowAsJavaScriptException();
            return info.Env().Undefined();
        }

        Napi::String cryptoLibAbsolutePath = info[0].As<Napi::String>();

        RFC_ERROR_INFO errorInfo;
        SAP_UC *libPath = setString(cryptoLibAbsolutePath);
        RFC_RC rc = RfcLoadCryptoLibrary(libPath, &errorInfo);
        free(libPath);

        if (rc != RFC_OK || errorInfo.code != RFC_OK)
        {
            return rfcSdkError(&errorInfo);
        }

        return info.Env().Undefined();
    }

    Napi::Value SetIniFileDirectory(const Napi::CallbackInfo &info)
    {
        if (!info[0].IsString())
        {
            std::ostringstream errmsg;
            errmsg << "Client setIniPath() requires the directory in which to search for the sapnwrfc.ini file, received: ";
            errmsg << info[0].As<Napi::String>().Utf8Value() << "; see" << USAGE_URL;
            Napi::TypeError::New(info.Env(), errmsg.str()).ThrowAsJavaScriptException();
            return info.Env().Undefined();
        }

        Napi::String iniFileDir = info[0].As<Napi::String>();

        RFC_ERROR_INFO errorInfo;
        SAP_UC *pathName = setString(iniFileDir);
        RFC_RC rc = RfcSetIniPath(pathName, &errorInfo);
        free(pathName);

        if (rc != RFC_OK || errorInfo.code != RFC_OK)
        {
            return rfcSdkError(&errorInfo);
        }

        return info.Env().Undefined();
    }

    Napi::Value Cancel(const Napi::CallbackInfo &info)
    {
        DEBUG("Cancel");

        Napi::Object client = info[0].As<Napi::Object>();
        RFC_FUNCTION_HANDLE functionHandle = NULL;
        RFC_CONNECTION_HANDLE connectionHandle = (RFC_CONNECTION_HANDLE)client.Get("connectionHandle").As<Napi::Number>().Int64Value();
        Napi::Value num = client.Get("functionHandle");
        if (num.IsNumber())
        {
            functionHandle = (RFC_FUNCTION_HANDLE)num.As<Napi::Number>().Int64Value();
        }

        RFC_RC rc = RFC_OK;
        RFC_ERROR_INFO errorInfo;

        DEBUG("Cancel connectionHandle: ", (pointer_t)connectionHandle, " functionHandle: ", (pointer_t)functionHandle);
        rc = RfcCancel(connectionHandle, &errorInfo);
        if (rc == RFC_OK && errorInfo.code == RFC_OK)
        {
            if (functionHandle != NULL)
            {
                rc = RfcDestroyFunction(functionHandle, &errorInfo);
            }
        }

        return (rc == RFC_OK && errorInfo.code == RFC_OK) ? info.Env().Undefined() : rfcSdkError(&errorInfo);
    }

    Napi::Object RegisterModule(Napi::Env env, Napi::Object exports)
    {
        exports.Set("bindingVersions", BindingVersions(env));
        exports.Set("setIniFileDirectory", Napi::Function::New(env, SetIniFileDirectory));
        exports.Set("loadCryptoLibrary", Napi::Function::New(env, LoadCryptoLibrary));
        exports.Set("cancel", Napi::Function::New(env, Cancel));

        Pool::Init(env, exports);
        Client::Init(env, exports);
        Throughput::Init(env, exports);
        Server::Init(env, exports);

        return exports;
    }

    NODE_API_MODULE(NODE_GYP_MODULE_NAME, RegisterModule);
} // namespace node_rfc

// SPDX-FileCopyrightText: 2014 SAP SE Srdjan Boskovic <srdjan.boskovic@sap.com>
//
// SPDX-License-Identifier: Apache-2.0

#include "Client.h"
#include "Log.h"
#include "Pool.h"
#include "Server.h"
#include "Throughput.h"

namespace node_rfc {

extern Log _log;

// Instantiate global namespace env
Napi::Env __env = nullptr;

Napi::Value BindingVersions(Napi::Env env) {
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

Napi::Value LoadCryptoLibrary(const Napi::CallbackInfo& info) {
  Napi::String cryptoLibAbsolutePath = info[0].As<Napi::String>();

  RFC_ERROR_INFO errorInfo;
  SAP_UC* libPath = setString(cryptoLibAbsolutePath);
  RFC_RC rc = RfcLoadCryptoLibrary(libPath, &errorInfo);
  delete[] libPath;

  if (rc != RFC_OK || errorInfo.code != RFC_OK) {
    return rfcSdkError(&errorInfo);
  }

  return info.Env().Undefined();
}

Napi::Value SetIniFileDirectory(const Napi::CallbackInfo& info) {
  Napi::String iniFileDir = info[0].As<Napi::String>();

  RFC_ERROR_INFO errorInfo;
  SAP_UC* pathName = setString(iniFileDir);
  RFC_RC rc = RfcSetIniPath(pathName, &errorInfo);
  delete[] pathName;

  if (rc != RFC_OK || errorInfo.code != RFC_OK) {
    return rfcSdkError(&errorInfo);
  }

  return info.Env().Undefined();
}

Napi::Value ReloadIniFile(const Napi::CallbackInfo& info) {
  RFC_ERROR_INFO errorInfo;
  RFC_RC rc = RfcReloadIniFile(&errorInfo);
  if (rc != RFC_OK || errorInfo.code != RFC_OK) {
    return rfcSdkError(&errorInfo);
  }

  return info.Env().Undefined();
}

Napi::Value LanguageIsoToSap(const Napi::CallbackInfo& info) {
  Napi::EscapableHandleScope scope(info.Env());

  RFC_ERROR_INFO errorInfo;

  Napi::String langISO = info[0].As<Napi::String>();
  SAP_UC* ucLangISO = setString(langISO);
  SAP_UC ucLangSAP[8];
  RFC_RC rc = RfcLanguageIsoToSap(ucLangISO, ucLangSAP, &errorInfo);
  delete[] ucLangISO;

  if (rc != RFC_OK || errorInfo.code != RFC_OK) {
    return rfcSdkError(&errorInfo);
  }

  return scope.Escape(wrapString(ucLangSAP, 1));
}

Napi::Value LanguageSapToIso(const Napi::CallbackInfo& info) {
  Napi::EscapableHandleScope scope(info.Env());
  RFC_ERROR_INFO errorInfo;

  std::string lang_sap = info[0].As<Napi::String>().Utf8Value();
  SAP_UC* ucLangSAP = setString(lang_sap);
  SAP_UC ucLangISO[8];
  RFC_RC rc = RfcLanguageSapToIso(ucLangSAP, ucLangISO, &errorInfo);
  delete[] ucLangSAP;

  if (rc != RFC_OK || errorInfo.code != RFC_OK) {
    _log.error(
        logClass::addon,
        "Error converting SAP language code: '" + lang_sap + "' to ISO code");
    return rfcSdkError(&errorInfo);
  }

  return scope.Escape(wrapString(ucLangISO, 2));
}

Napi::Value SetLogFilePath(const Napi::CallbackInfo& info) {
  _log.set_log_file_path(info[0].As<Napi::String>().Utf8Value());
  return info.Env().Undefined();
}
Napi::Object RegisterModule(Napi::Env env, Napi::Object exports) {
  if (node_rfc::__env == nullptr) {
    node_rfc::__env = env;
  }

  exports.Set("bindingVersions", BindingVersions(env));
  exports.Set("setLogFilePath", Napi::Function::New(env, SetLogFilePath));
  exports.Set("setIniFileDirectory",
              Napi::Function::New(env, SetIniFileDirectory));
  exports.Set("loadCryptoLibrary", Napi::Function::New(env, LoadCryptoLibrary));
  exports.Set("languageIsoToSap", Napi::Function::New(env, LanguageIsoToSap));
  exports.Set("languageSapToIso", Napi::Function::New(env, LanguageSapToIso));
  exports.Set("reloadIniFile", Napi::Function::New(env, ReloadIniFile));

  Pool::Init(env, exports);
  Client::Init(env, exports);
  Throughput::Init(env, exports);
  Server::Init(env, exports);

  return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, RegisterModule)
}  // namespace node_rfc

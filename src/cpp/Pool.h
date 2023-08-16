// SPDX-FileCopyrightText: 2014 SAP SE Srdjan Boskovic <srdjan.boskovic@sap.com>
//
// SPDX-License-Identifier: Apache-2.0

#ifndef NodeRfc_Pool_H
#define NodeRfc_Pool_H

#include <mutex>
#include <set>
#include "Client.h"
#include "Log.h"
#include "nwrfcsdk.h"

using namespace Napi;

namespace node_rfc {
extern Napi::Env __env;
extern Log _log;

typedef std::set<RFC_CONNECTION_HANDLE> ConnectionSetType;
class Pool : public Napi::ObjectWrap<Pool> {
 public:
  friend class Client;
  friend class AcquireAsync;
  friend class ReleaseAsync;
  friend class CheckPoolAsync;
  friend class SetPoolAsync;
  static Napi::Object Init(Napi::Env env, Napi::Object exports);
  // cppcheck-suppress noExplicitConstructor
  Pool(const Napi::CallbackInfo& info);
  ~Pool(void);

  const std::string log_id() const { return "Pool " + std::to_string(id); }

 private:
  Napi::Value IdGetter(const Napi::CallbackInfo& info);
  Napi::Value Acquire(const Napi::CallbackInfo& info);
  Napi::Value Release(const Napi::CallbackInfo& info);
  Napi::Value Ready(const Napi::CallbackInfo& info);
  Napi::Value CloseAll(const Napi::CallbackInfo& info);
  void closeConnections();
  void releaseClient(RFC_CONNECTION_HANDLE connectionHandle);
  std::string updateLeasedHandle(RFC_CONNECTION_HANDLE old_handle,
                                 RFC_CONNECTION_HANDLE new_handle);
  Napi::ObjectReference poolConfiguration;
  Napi::ObjectReference connectionParameters;
  Napi::ObjectReference clientOptions;
  Napi::ObjectReference poolOptions;

  Napi::Value ConfigGetter(const Napi::CallbackInfo& info);
  Napi::Value StatusGetter(const Napi::CallbackInfo& info);

  static uint_t _id;
  uint_t id;

  void init() {
    id = Pool::_id++;

    // Pool options
    ready_low = POOL_READY_LOW;
    ready_high = POOL_READY_HIGH;
    fill_requests = 0;

    connReady = {};
    connLeased = {};
  };

  ConnectionParamsStruct client_params = ConnectionParamsStruct(0, nullptr);
  ClientOptionsStruct client_options;

  // Pool options
  uint_t ready_low;
  uint_t ready_high;
  uint_t fill_requests;

  // Connections pool
  std::mutex leaseMutex;
  void lockMutex();
  void unlockMutex();
  ConnectionSetType connReady;
  ConnectionSetType connLeased;
};
}  // namespace node_rfc

#endif

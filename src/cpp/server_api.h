
// SPDX-FileCopyrightText: 2014 SAP SE Srdjan Boskovic <srdjan.boskovic@sap.com>
//
// SPDX-License-Identifier: Apache-2.0

#ifndef NodeRfc_ServerAPI_H
#define NodeRfc_ServerAPI_H

#include <napi.h>
#include <condition_variable>
#include "Server.h"
#include "nwrfcsdk.h"

namespace node_rfc {

//
// Tsfn type definitions
//

class ServerRequestBaton;
using DataType = ServerRequestBaton*;
void JSHandlerCall(Napi::Env env,
                   Napi::Function callback,
                   std::nullptr_t* context,
                   DataType requestBaton);
typedef Napi::
    TypedThreadSafeFunction<std::nullptr_t, ServerRequestBaton, JSHandlerCall>
        ServerRequestTsfn;

class AuthRequestHandler;
using DataTypeAuth = AuthRequestHandler*;
void JSAuthCall(Napi::Env env,
                Napi::Function callback,
                std::nullptr_t* context,
                DataTypeAuth requestBaton);
using AuthRequestTsfn = Napi::
    TypedThreadSafeFunction<std::nullptr_t, AuthRequestHandler, JSAuthCall>;

//
// HandlerFunction
//

// When JS function handler is registered in Server::AddFunction,
// the HandlerFunction instance is created, with TSFN reference
// to JS function handler
class HandlerFunction {
 public:
  Server* server;
  // Parameters from metadataLookup
  std::string abap_func_name;
  RFC_ABAP_NAME abap_func_name_sapuc;
  RFC_FUNCTION_DESC_HANDLE func_desc_handle;
  ServerRequestTsfn tsfnRequest;
  std::string jsFunctionName;
  // Installed JS handler functions
  static std::unordered_map<std::string, HandlerFunction*> installed_functions;

  HandlerFunction(Server* server,
                  const std::string& _abap_func_name,
                  RFC_ABAP_NAME _abap_func_name_sapuc,
                  RFC_FUNCTION_DESC_HANDLE func_desc_handle,
                  ServerRequestTsfn tsfnRequest,
                  const std::string& jsFunctionName);
  ~HandlerFunction();

  // get request id for request baton
  std::string next_request_id();

  // Register JS handler function
  static Napi::Value add_function(Server* server,
                                  Napi::Env env,
                                  Napi::String abapFunctionName,
                                  Napi::Function jsFunction);

  // Called by genericRequestHandler, to find JS handler function reference
  static HandlerFunction* get_function(RFC_CONNECTION_HANDLE conn_handle,
                                       RFC_FUNCTION_HANDLE func_handle,
                                       RFC_ERROR_INFO* errorInfo);

  // Called by metadataLookup RFC SDK server interface, to find
  // function description of ABAP function requested by ABAP client
  static RFC_RC set_function_handle(SAP_UC const* abap_func_name,
                                    RFC_ATTRIBUTES rfc_attributes,
                                    RFC_FUNCTION_DESC_HANDLE* func_desc_handle);

  // Un-register JS handler function
  static Napi::Value remove_function(Napi::Function jsFunction);

  // Called by Server destructor, to clean-up TSFN instances
  static void release(Server* server);
};

//
// AuthRequestHandler
//

class AuthRequestHandler {
 private:
  // set when auth handler registered
  AuthRequestTsfn tsfn;
  // set by SAP NW RFC SDK API
  RFC_CONNECTION_HANDLE auth_connection_handle = nullptr;
  RFC_SECURITY_ATTRIBUTES* secAttributes = nullptr;
  RFC_ERROR_INFO* errorInfo = nullptr;
  // set when JS auth handler registered
  bool tsfn_created = false;

  // js handler synchronization
  bool js_call_completed = false;
  std::mutex js_call_mutex;

  std::condition_variable js_call_condition;

  std::string jsHandlerError;

 public:
  AuthRequestHandler();

  bool is_registered();

  void unregister();

  void registerJsHandler(Napi::FunctionReference& func);

  // ABAP request data to JS
  Napi::Value getRequestData();

  // JS response data to ABAP
  void setResponseData(Napi::Value jsAuthResponse);

  RFC_RC SAP_API callJS(RFC_CONNECTION_HANDLE conn_handle,
                        RFC_SECURITY_ATTRIBUTES* secAttributes,
                        RFC_ERROR_INFO* errorInfo);
  void wait();

  void done(const std::string& errorObj = "");

  // start
  void start(RFC_ERROR_INFO* errorInfo);
};

//
// GenericFunctionHandler
//

class GenericFunctionHandler;
class ServerRequestBaton;

//
// NW RFC SDK Server API
//

class sapnwrfcServerAPI {
 public:
  //
  // sapnwrfc native api, invoking sapnwrfc api handler functions
  //
  // (mandatory)
  static RFC_RC SAP_API metadataLookup(SAP_UC const* func_name,
                                       RFC_ATTRIBUTES rfc_attributes,
                                       RFC_FUNCTION_DESC_HANDLE* func_handle);

  static RFC_RC SAP_API genericRequestHandler(RFC_CONNECTION_HANDLE conn_handle,
                                              RFC_FUNCTION_HANDLE func_handle,
                                              RFC_ERROR_INFO* errorInfo);
  // (optional)
  static RFC_RC SAP_API authHandler(RFC_CONNECTION_HANDLE rfcHandle,
                                    RFC_SECURITY_ATTRIBUTES* secAttributes,
                                    RFC_ERROR_INFO* errorInfo);
  // bgRFC
  static RFC_RC SAP_API bgRfcCheck(RFC_CONNECTION_HANDLE rfcHandle,
                                   const RFC_UNIT_IDENTIFIER* identifier);
  static RFC_RC SAP_API bgRfcCommit(RFC_CONNECTION_HANDLE rfcHandle,
                                    const RFC_UNIT_IDENTIFIER* identifier);
  static RFC_RC SAP_API bgRfcRollback(RFC_CONNECTION_HANDLE rfcHandle,
                                      const RFC_UNIT_IDENTIFIER* identifier);
  static RFC_RC SAP_API bgRfcConfirm(RFC_CONNECTION_HANDLE rfcHandle,
                                     const RFC_UNIT_IDENTIFIER* identifier);
  static RFC_RC SAP_API bgRfcGetState(RFC_CONNECTION_HANDLE rfcHandle,
                                      const RFC_UNIT_IDENTIFIER* identifier,
                                      RFC_UNIT_STATE* unitState);
  // sapnwrfc api handler functions
  static AuthRequestHandler authorizationHandler;
  static GenericFunctionHandler genericFunctionHandler;
};

//
// ServerRequestBaton
//

class ServerRequestBaton {
 private:
  bool server_call_completed = false;
  std::mutex server_call_mutex;
  std::condition_variable server_call_condition;

 public:
  // Parameters from genericRequestHandler
  RFC_CONNECTION_HANDLE request_connection_handle;
  RFC_FUNCTION_HANDLE func_handle;
  RFC_ERROR_INFO* errorInfo;
  // JS server function to handle the request
  HandlerFunction* handlerFunction;
  // JS server function result
  std::string jsHandlerError;

  RfmErrorPath errorPath;

  ClientOptionsStruct client_options;

  // Server request id
  std::string request_id;

  ServerRequestBaton(RFC_CONNECTION_HANDLE conn_handle,
                     RFC_FUNCTION_HANDLE func_handle,
                     RFC_ERROR_INFO* errorInfo,
                     HandlerFunction* handlerFunction);

  void wait();

  void done(const std::string& errorObj);

  Napi::Value getServerRequestContext();

  // Transform JavaScript parameters' data to ABAP
  void setResponseData(Napi::Env env, Napi::Value jsResult);
};

}  // namespace node_rfc
#endif
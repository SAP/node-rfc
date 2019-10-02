#include "client.h"

////////////////////////////////////////////////////////////////////////////////
// RFC ERRORS
////////////////////////////////////////////////////////////////////////////////
namespace node_rfc
{
Napi::Value Client::RfcLibError(RFC_ERROR_INFO *errorInfo)
{
    Napi::EscapableHandleScope scope(__env);

    Napi::Object errorObj = Napi::Object::New(__env);
    (errorObj).Set(Napi::String::New(__env, "name"), "RfcLibError");
    (errorObj).Set(Napi::String::New(__env, "code"), Napi::Number::New(__env, errorInfo->code));
    (errorObj).Set(Napi::String::New(__env, "key"), wrapString(errorInfo->key));
    (errorObj).Set(Napi::String::New(__env, "message"), wrapString(errorInfo->message));

    return scope.Escape(errorObj);
}

Napi::Value Client::AbapError(RFC_ERROR_INFO *errorInfo)
{
    Napi::EscapableHandleScope scope(__env);

    Napi::Object errorObj = Napi::Object::New(__env);
    (errorObj).Set(Napi::String::New(__env, "name"), "ABAPError");
    (errorObj).Set(Napi::String::New(__env, "code"), Napi::Number::New(__env, errorInfo->code));
    (errorObj).Set(Napi::String::New(__env, "key"), wrapString(errorInfo->key));
    (errorObj).Set(Napi::String::New(__env, "message"), wrapString(errorInfo->message));
    (errorObj).Set(Napi::String::New(__env, "abapMsgClass"), wrapString(errorInfo->abapMsgClass));
    (errorObj).Set(Napi::String::New(__env, "abapMsgType"), wrapString(errorInfo->abapMsgType));
    (errorObj).Set(Napi::String::New(__env, "abapMsgNumber"), wrapString(errorInfo->abapMsgNumber));
    (errorObj).Set(Napi::String::New(__env, "abapMsgV1"), wrapString(errorInfo->abapMsgV1));
    (errorObj).Set(Napi::String::New(__env, "abapMsgV2"), wrapString(errorInfo->abapMsgV2));
    (errorObj).Set(Napi::String::New(__env, "abapMsgV3"), wrapString(errorInfo->abapMsgV3));
    (errorObj).Set(Napi::String::New(__env, "abapMsgV4"), wrapString(errorInfo->abapMsgV4));

    return scope.Escape(errorObj);
}

Napi::Value Client::wrapError(RFC_ERROR_INFO *errorInfo)
{
    Napi::EscapableHandleScope scope(__env);

    char cBuf[256];

    switch (errorInfo->group)
    {
    case OK: // 0: should never happen
        Napi::Error::Fatal("Error handling invoked with the RFC error group OK", "node-rfc internal error");
        break;

    case LOGON_FAILURE:            // 3: Error message raised when logon fails
    case COMMUNICATION_FAILURE:    // 4: Problems with the network connection (or backend broke down and killed the connection)
    case EXTERNAL_RUNTIME_FAILURE: // 5: Problems in the RFC runtime of the external program (i.e "this" library)
        return scope.Escape(RfcLibError(errorInfo));
        break;

    case ABAP_APPLICATION_FAILURE:       // 1: ABAP Exception raised in ABAP function modules
    case ABAP_RUNTIME_FAILURE:           // 2: ABAP Message raised in ABAP function modules or in ABAP runtime of the backend (e.g Kernel)
    case EXTERNAL_APPLICATION_FAILURE:   // 6: Problems in the external program (e.g in the external server implementation)
    case EXTERNAL_AUTHORIZATION_FAILURE: // 7: Problems raised in the authorization check handler provided by the external server implementation
        return scope.Escape(AbapError(errorInfo));
        break;
    }

    // unknown error group
    sprintf(cBuf, "Unknown error group: %u", errorInfo->group);
    Napi::Error::Fatal(cBuf, "node-rfc internal error");
}
} // namespace node_rfc
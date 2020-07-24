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

#ifndef NodeRfc_SDK_H_
#define NodeRfc_SDK_H_

#include "noderfc.h"
#include <sstream>

using namespace Napi;

#define ERROR_PATH_NAME_LEN 48
namespace node_rfc
{
    extern Napi::Env __env;

    // Basic string operations
    Napi::Value wrapString(SAP_UC *uc, int length = -1);
    SAP_UC *fillString(const Napi::String napistr);

    //
    // Client connection parameters internal representation
    //
    typedef struct _ConnectionParamsStruct
    {
        uint_t paramSize = 0;
        RFC_CONNECTION_PARAMETER *connectionParams = NULL;
        //_ConnectionParamsStruct(uint_t paramSize, RFC_CONNECTION_PARAMETER *connectionParams)
        //    : paramSize(paramSize), connectionParams(connectionParams)
        //{
        //    DEBUG("ConnectionParamsStruct %u", paramSize);
        //}

        ~_ConnectionParamsStruct()
        {
            //DEBUG("~ConnectionParamsStruct ", paramSize);
            if (connectionParams != NULL)
            {
                for (uint_t i = 0; i < this->paramSize; i++)
                {
                    free((void *)connectionParams[i].name);
                    free((void *)connectionParams[i].value);
                }
                free(connectionParams);
                connectionParams = NULL;
            }
        }
    } ConnectionParamsStruct;

    //
    // Client options internal representation
    //
    typedef struct _ClientOptionsStruct
    {
        uint_t bcd = CLIENT_OPTION_BCD_STRING;
        bool stateless = false;
        RFC_DIRECTION filter_param_type = (RFC_DIRECTION)0;
        Napi::FunctionReference bcdFunction;
        Napi::FunctionReference dateToABAP;
        Napi::FunctionReference dateFromABAP;
        Napi::FunctionReference timeToABAP;
        Napi::FunctionReference timeFromABAP;

        Napi::Value _Value(Napi::Env env)
        {
            Napi::Object options = Napi::Object::New(env);

            //stateless
            options.Set(CLIENT_OPTION_KEY_STATELESS, Napi::Boolean::New(env, stateless));

            // filter
            options.Set(CLIENT_OPTION_KEY_FILTER, Napi::Number::New(env, filter_param_type));

            // bcd
            if (bcd == CLIENT_OPTION_BCD_STRING)
            {
                options.Set(CLIENT_OPTION_KEY_BCD, "string");
            }
            else if (bcd == CLIENT_OPTION_BCD_NUMBER)
            {
                options.Set(CLIENT_OPTION_KEY_BCD, "number");
            }
            else if (bcd == CLIENT_OPTION_BCD_FUNCTION)
            {
                options.Set(CLIENT_OPTION_KEY_BCD, bcdFunction.Value());
            }
            else
            {
                options.Set(CLIENT_OPTION_KEY_BCD, "?");
            }

            // date
            Napi::Object odate = Napi::Object::New(env);
            if (!dateToABAP.IsEmpty())
            {
                odate.Set("toABAP", dateToABAP.Value());
            }
            else
            {
                odate.Set("toABAP", "string");
            }
            if (!dateFromABAP.IsEmpty())
            {
                odate.Set("fromABAP", dateFromABAP.Value());
            }
            else
            {
                odate.Set("fromABAP", "string");
            }
            options.Set(CLIENT_OPTION_KEY_DATE, odate);

            // time
            Napi::Object otime = Napi::Object::New(env);
            if (!timeToABAP.IsEmpty())
            {
                otime.Set("toABAP", timeToABAP.Value());
            }
            else
            {
                otime.Set("toABAP", "string");
            }
            if (!timeFromABAP.IsEmpty())
            {
                otime.Set("fromABAP", timeFromABAP.Value());
            }
            else
            {
                otime.Set("fromABAP", "string");
            }
            options.Set(CLIENT_OPTION_KEY_TIME, otime);

            Napi::EscapableHandleScope scope(env);
            return scope.Escape(options);
        }

        _ClientOptionsStruct &operator=(_ClientOptionsStruct &pool_client_options) // note: passed by copy
        {
            bcd = pool_client_options.bcd;
            stateless = pool_client_options.stateless;
            filter_param_type = pool_client_options.filter_param_type;
            // bcd
            if (!pool_client_options.bcdFunction)
            {
                bcdFunction = Napi::Persistent(pool_client_options.bcdFunction.Value());
            }
            // date
            if (!pool_client_options.dateToABAP.IsEmpty())
            {
                dateToABAP = Napi::Persistent(pool_client_options.dateToABAP.Value());
            }
            if (!pool_client_options.dateFromABAP.IsEmpty())
            {
                dateFromABAP = Napi::Persistent(pool_client_options.dateFromABAP.Value());
            }
            // time
            if (!pool_client_options.timeToABAP.IsEmpty())
            {
                timeToABAP = Napi::Persistent(pool_client_options.timeToABAP.Value());
            }
            if (!pool_client_options.timeFromABAP.IsEmpty())
            {
                timeFromABAP = Napi::Persistent(pool_client_options.timeFromABAP.Value());
            }
            return *this;
        };

        ~_ClientOptionsStruct()
        {
            //DEBUG("~ClientOptionsStruct");
            // bcd
            if (!bcdFunction.IsEmpty())
            {
                bcdFunction.Unref();
            }
            // date
            if (!dateToABAP.IsEmpty())
            {
                dateToABAP.Unref();
            }
            if (!dateFromABAP.IsEmpty())
            {
                dateFromABAP.Unref();
            }
            // time
            if (!timeToABAP.IsEmpty())
            {
                timeToABAP.Unref();
            }
            if (!timeFromABAP.IsEmpty())
            {
                timeFromABAP.Unref();
            }
        }
    } ClientOptionsStruct;

    typedef struct _RfmErrorPath
    {
        SAP_UC functionName[ERROR_PATH_NAME_LEN];
        SAP_UC parameterName[ERROR_PATH_NAME_LEN];
        SAP_UC tableName[ERROR_PATH_NAME_LEN];
        int64_t table_line = -1;
        SAP_UC structureName[ERROR_PATH_NAME_LEN];
        SAP_UC fieldName[ERROR_PATH_NAME_LEN];

        void clear()
        {
            functionName[0] = 0;
            parameterName[0] = 0;
            resetPath();
        }

        void resetPath()
        {
            table_line = -1;
            tableName[0] = 0;
            structureName[0] = 0;
            fieldName[0] = 0;
        }

        void setName(RFCTYPE typ, SAP_UC *cName)
        {
            if (typ == RFCTYPE_STRUCTURE)
            {
                strcpyU(structureName, cName);
            }
            else if (typ == RFCTYPE_TABLE)
            {
                strcpyU(tableName, cName);
            }
            else
            {
                strcpyU(fieldName, cName);
            }
        }

        void setFunctionName(SAP_UC *funcName)
        {
            clear();
            strcpyU(functionName, funcName);
        }

        void setParameterName(SAP_UC *pName)
        {
            resetPath();
            strcpyU(parameterName, pName);
        }

        void setFieldName(SAP_UC *fName)
        {
            strcpyU(fieldName, fName);
        }

        Napi::Object getpath()
        {
            Napi::Object path = Napi::Object::New(node_rfc::__env);
            path.Set("rfm", wrapString(functionName));
            path.Set("parameter", wrapString(parameterName));
            if (*tableName)
            {
                path.Set("table", wrapString(tableName));
                path.Set("table_line", table_line);
            }
            if (*structureName)
            {
                path.Set("structure", wrapString(structureName));
            }
            if (*fieldName)
            {
                path.Set("field", wrapString(fieldName));
            }
            return path;
        }

        std::string pathstr()
        {
            std::ostringstream ss;
            Napi::Object obj = this->getpath();
            Napi::Array objKeys = obj.GetPropertyNames();
            ss << "rfmPath: {\n";
            for (uint_t ii = 0; ii < objKeys.Length(); ii++)
            {
                std::string key = objKeys.Get(ii).As<Napi::String>().Utf8Value();
                std::string val = obj.Get(key).ToString().Utf8Value();
                ss << "  " << key << ": " << val << ", \n";
            }
            ss << "}\n";
            return ss.str();
        }

    } RfmErrorPath;

    // RFC ERRORS
    Napi::Object RfcLibError(RFC_ERROR_INFO *errorInfo);
    Napi::Object AbapError(RFC_ERROR_INFO *errorInfo);
    Napi::Value rfcSdkError(RFC_ERROR_INFO *errorInfo, RfmErrorPath *errorPath = NULL);
    Napi::Value nodeRfcError(std::string message, RfmErrorPath *errorPath = NULL);

} // namespace node_rfc
#endif

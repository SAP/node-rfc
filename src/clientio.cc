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

#include "nwrfcsdk.h"
#include "Client.h"

namespace node_rfc
{
    extern Napi::Env __env;

    ////////////////////////////////////////////////////////////////////////////////
    // FILL FUNCTIONS (to RFC)
    ////////////////////////////////////////////////////////////////////////////////

    SAP_UC *Client::fillString(std::string str)
    {
        RFC_RC rc;
        RFC_ERROR_INFO errorInfo;
        SAP_UC *sapuc;
        uint_t sapucSize, resultLen = 0;

        sapucSize = str.length() + 1;

        // printf("%s: %u\n", &str[0], sapucSize);

        sapuc = (SAP_UC *)mallocU(sapucSize);
        memsetU((SAP_UTF16 *)sapuc, 0, sapucSize);
        rc = RfcUTF8ToSAPUC((RFC_BYTE *)&str[0], str.length(), sapuc, &sapucSize, &resultLen, &errorInfo);

        if (rc != RFC_OK)
            Napi::Error::Fatal("fillString", "node-rfc internal error");

        return sapuc;
    }

    Napi::Value Client::fillFunctionParameter(RFC_FUNCTION_DESC_HANDLE functionDescHandle, RFC_FUNCTION_HANDLE functionHandle, Napi::String name, Napi::Value value)
    {
        Napi::EscapableHandleScope scope(value.Env());

        RFC_RC rc;
        RFC_ERROR_INFO errorInfo;
        RFC_PARAMETER_DESC paramDesc;
        SAP_UC *cName = fillString(name);
        rc = RfcGetParameterDescByName(functionDescHandle, cName, &paramDesc, &errorInfo);
        free(cName);
        if (rc != RFC_OK)
        {
            return scope.Escape(wrapError(&errorInfo));
        }
        return scope.Escape(fillVariable(paramDesc.type, functionHandle, paramDesc.name, value, paramDesc.typeDescHandle));
    }

    Napi::Value Client::fillStructure(RFC_STRUCTURE_HANDLE structHandle, RFC_TYPE_DESC_HANDLE functionDescHandle, SAP_UC *cName, Napi::Value value)
    {
        RFC_RC rc;
        RFC_ERROR_INFO errorInfo;

        Napi::EscapableHandleScope scope(value.Env());

        Napi::Object structObj = value.ToObject(); // ->ToObject();
        Napi::Array structNames = structObj.GetPropertyNames();
        uint_t structSize = structNames.Length();

        RFC_FIELD_DESC fieldDesc;

        Napi::Value retVal = value.Env().Undefined();

        for (uint_t i = 0; i < structSize; i++)
        {
            Napi::String name = structNames.Get(i).ToString();
            Napi::Value value = structObj.Get(name);

            SAP_UC *cValue = fillString(name);
            rc = RfcGetFieldDescByName(functionDescHandle, cValue, &fieldDesc, &errorInfo);
            free(cValue);
            if (rc != RFC_OK)
            {
                retVal = wrapError(&errorInfo);
                break;
            }
            retVal = fillVariable(fieldDesc.type, structHandle, fieldDesc.name, value, fieldDesc.typeDescHandle);
            if (!retVal.IsUndefined())
            {
                break;
            }
        }
        return retVal;
    }

    Napi::Value Client::fillVariable(RFCTYPE typ, RFC_FUNCTION_HANDLE functionHandle, SAP_UC *cName, Napi::Value value, RFC_TYPE_DESC_HANDLE functionDescHandle)
    {
        Napi::EscapableHandleScope scope(value.Env());
        RFC_RC rc = RFC_OK;
        RFC_ERROR_INFO errorInfo;
        SAP_UC *cValue;
        switch (typ)
        {
        case RFCTYPE_STRUCTURE:
        {
            RFC_STRUCTURE_HANDLE structHandle;
            rc = RfcGetStructure(functionHandle, cName, &structHandle, &errorInfo);
            if (rc != RFC_OK)
            {
                return scope.Escape(wrapError(&errorInfo));
            }
            Napi::Value rv = fillStructure(structHandle, functionDescHandle, cName, value);
            if (!rv.IsUndefined())
            {
                return scope.Escape(rv);
            }
            break;
        }
        case RFCTYPE_TABLE:
        {
            RFC_TABLE_HANDLE tableHandle;
            rc = RfcGetTable(functionHandle, cName, &tableHandle, &errorInfo);

            if (rc != RFC_OK)
            {
                break;
            }
            if (!value.IsArray())
            {
                char err[256];
                std::string fieldName = wrapString(cName).ToString().Utf8Value();
                sprintf(err, "Array expected when filling field %s of type %d", &fieldName[0], typ);
                return scope.Escape(Napi::TypeError::New(value.Env(), err).Value());
            }
            Napi::Array array = value.As<Napi::Array>();
            uint_t rowCount = array.Length();

            for (uint_t i = 0; i < rowCount; i++)
            {
                RFC_STRUCTURE_HANDLE structHandle = RfcAppendNewRow(tableHandle, &errorInfo);
                Napi::Value line = array.Get(i);
                if (line.IsBuffer() || line.IsString() || line.IsNumber())
                {
                    Napi::Object lineObj = Napi::Object::New(value.Env());
                    lineObj.Set(Napi::String::New(value.Env(), ""), line);
                    line = lineObj;
                }
                Napi::Value rv = fillStructure(structHandle, functionDescHandle, cName, line);
                if (!rv.IsUndefined())
                {
                    return scope.Escape(rv);
                }
            }
            break;
        }
        case RFCTYPE_CHAR:
        {
            if (!value.IsString())
            {
                char err[256];
                std::string fieldName = wrapString(cName).ToString().Utf8Value();
                sprintf(err, "Char expected when filling field %s of type %d", &fieldName[0], typ);
                return scope.Escape(Napi::TypeError::New(value.Env(), err).Value());
            }
            cValue = fillString(value.As<Napi::String>());
            rc = RfcSetChars(functionHandle, cName, cValue, strlenU((SAP_UTF16 *)cValue), &errorInfo);
            free(cValue);
            break;
        }
        case RFCTYPE_BYTE:
        {
            if (!value.IsBuffer())
            {
                char err[256];
                std::string fieldName = wrapString(cName).ToString().Utf8Value();
                sprintf(err, "Buffer expected when filling field '%s' of type %d", &fieldName[0], typ);
                return scope.Escape(Napi::TypeError::New(value.Env(), err).Value());
            }

            Napi::Buffer<char> buf = value.As<Napi::Buffer<char>>();
            uint_t size = buf.ByteLength();
            SAP_RAW *byteValue = (SAP_RAW *)malloc(size);
            memcpy(byteValue, buf.Data(), size);

            rc = RfcSetBytes(functionHandle, cName, byteValue, size, &errorInfo);
            free(byteValue);
            break;
        }
        case RFCTYPE_XSTRING:
        {
            if (!value.IsBuffer())
            {
                char err[256];
                std::string fieldName = wrapString(cName).ToString().Utf8Value();
                sprintf(err, "Buffer expected when filling field '%s' of type %d", &fieldName[0], typ);
                return scope.Escape(Napi::TypeError::New(value.Env(), err).Value());
            }

            Napi::Buffer<char> buf = value.As<Napi::Buffer<char>>();
            uint_t size = buf.ByteLength();
            SAP_RAW *byteValue = (SAP_RAW *)malloc(size);
            memcpy(byteValue, buf.Data(), size);

            rc = RfcSetXString(functionHandle, cName, byteValue, size, &errorInfo);
            free(byteValue);
            break;
        }
        case RFCTYPE_STRING:
        {
            if (!value.IsString())
            {
                char err[256];
                std::string fieldName = wrapString(cName).ToString().Utf8Value();
                sprintf(err, "Char expected when filling field %s of type %d", &fieldName[0], typ);
                return scope.Escape(Napi::TypeError::New(value.Env(), err).Value());
            }
            cValue = fillString(value.ToString());
            rc = RfcSetString(functionHandle, cName, cValue, strlenU((SAP_UTF16 *)cValue), &errorInfo);
            free(cValue);
            break;
        }
        case RFCTYPE_NUM:
        {
            if (!value.IsString())
            {
                char err[256];
                std::string fieldName = wrapString(cName).ToString().Utf8Value();
                sprintf(err, "Char expected when filling field %s of type %d", &fieldName[0], typ);
                return scope.Escape(Napi::TypeError::New(value.Env(), err).Value());
            }
            cValue = fillString(value.ToString());
            rc = RfcSetNum(functionHandle, cName, cValue, strlenU((SAP_UTF16 *)cValue), &errorInfo);
            free(cValue);
            break;
        }
        case RFCTYPE_BCD: // fallthrough
        case RFCTYPE_DECF16:
        case RFCTYPE_DECF34:
        case RFCTYPE_FLOAT:
        {
            if (!value.IsNumber() && !value.IsObject() && !value.IsString())
            {
                char err[256];
                std::string fieldName = wrapString(cName).ToString().Utf8Value();
                sprintf(err, "Number, number object or string expected when filling field %s of type %d", &fieldName[0], typ);
                return scope.Escape(Napi::TypeError::New(value.Env(), err).Value());
            }
            cValue = fillString(value.ToString());
            rc = RfcSetString(functionHandle, cName, cValue, strlenU((SAP_UTF16 *)cValue), &errorInfo);
            free(cValue);
            break;
        }
        case RFCTYPE_INT: // fallthrough
        case RFCTYPE_INT1:
        case RFCTYPE_INT2:
        case RFCTYPE_INT8:
        {
            if (!value.IsNumber())
            {
                char err[256];
                std::string fieldName = wrapString(cName).ToString().Utf8Value();
                sprintf(err, "Integer number expected when filling field %s of type %d", &fieldName[0], typ);
                return scope.Escape(Napi::TypeError::New(value.Env(), err).Value());
            }

            // https://github.com/mhdawson/node-sqlite3/pull/3
            double numDouble = value.ToNumber().DoubleValue();
            if ((int64_t)numDouble != numDouble) // or std::trunc(numDouble) == numDouble;
            {
                char err[256];
                std::string fieldName = wrapString(cName).ToString().Utf8Value();
                sprintf(err, "Integer number expected when filling field %s of type %d, got %a", &fieldName[0], typ, numDouble);
                return scope.Escape(Napi::TypeError::New(value.Env(), err).Value());
            }
            RFC_INT rfcInt = (RFC_INT)value.As<Napi::Number>().Int64Value();
            //int64_t rfcInt = value.As<Napi::Number>().Int64Value();
            //printf("typ: %d value: %d %u", typ, rfcInt, UINT8_MAX);
            if (typ == RFCTYPE_INT8)
            {
                rc = RfcSetInt8(functionHandle, cName, rfcInt, &errorInfo);
            }
            else
            {
                if (
                    (typ == RFCTYPE_INT1 && rfcInt > UINT8_MAX) ||
                    (typ == RFCTYPE_INT2 && ((rfcInt > INT16_MAX) || (rfcInt < INT16_MIN))))
                {
                    char err[256];
                    std::string fieldName = wrapString(cName).ToString().Utf8Value();
                    sprintf(err, "Overflow or other error when filling integer field %s of type %d, value: %d", &fieldName[0], typ, rfcInt);
                    return scope.Escape(Napi::TypeError::New(value.Env(), err).Value());
                }

                rc = RfcSetInt(functionHandle, cName, rfcInt, &errorInfo);
            }
            break;
        }
        case RFCTYPE_UTCLONG:
        {
            if (!value.IsString())
            {
                char err[256];
                std::string fieldName = wrapString(cName).ToString().Utf8Value();
                sprintf(err, "UTCLONG string expected when filling field %s of type %d", &fieldName[0], typ);
                return scope.Escape(Napi::TypeError::New(value.Env(), err).Value());
            }
            cValue = fillString(value.ToString());
            rc = RfcSetString(functionHandle, cName, cValue, strlenU((SAP_UTF16 *)cValue), &errorInfo);
            free(cValue);
            break;
        }
        case RFCTYPE_DATE:
        {
            if (!client_options.dateToABAP.IsEmpty())
            {
                // YYYYMMDD format expected
                value = client_options.dateToABAP.Call({value});
            }
            if (!value.IsString())
            {
                char err[256];
                std::string fieldName = wrapString(cName).ToString().Utf8Value();
                sprintf(err, "ABAP date format YYYYMMDD expected when filling field %s of type %d", &fieldName[0], typ);
                return scope.Escape(Napi::TypeError::New(value.Env(), err).Value());
            }
            cValue = fillString(value.ToString());
            rc = RfcSetDate(functionHandle, cName, cValue, &errorInfo);
            free(cValue);
            break;
        }
        case RFCTYPE_TIME:
        {
            if (!client_options.timeToABAP.IsEmpty())
            {
                // HHMMSS format expected
                value = client_options.timeToABAP.Call({value});
            }
            if (!value.IsString())
            {
                char err[256];
                std::string fieldName = wrapString(cName).ToString().Utf8Value();
                sprintf(err, "ABAP time format HHMMSS expected when filling field %s of type %d", &fieldName[0], typ);
                return scope.Escape(Napi::TypeError::New(value.Env(), err).Value());
            }
            cValue = fillString(value.ToString());
            rc = RfcSetTime(functionHandle, cName, cValue, &errorInfo);
            free(cValue);
            break;
        }
        default:
        {
            char err[256];
            std::string fieldName = wrapString(cName).ToString().Utf8Value();
            sprintf(err, "Unknown RFC type %u when filling %s", typ, &fieldName[0]);
            return scope.Escape(Napi::TypeError::New(value.Env(), err).Value());
            break;
        }
        }
        if (rc != RFC_OK)
        {
            return scope.Escape(wrapError(&errorInfo));
        }
        return scope.Env().Undefined();
    }

    ////////////////////////////////////////////////////////////////////////////////
    // WRAP FUNCTIONS (from RFC)
    ////////////////////////////////////////////////////////////////////////////////

    Napi::Value Client::wrapResult(RFC_FUNCTION_DESC_HANDLE functionDescHandle, RFC_FUNCTION_HANDLE functionHandle)
    {
        DEBUG("wrapResult");
        Napi::EscapableHandleScope scope(node_rfc::__env);

        RFC_PARAMETER_DESC paramDesc;

        uint_t paramCount = 0;

        RfcGetParameterCount(functionDescHandle, &paramCount, NULL);
        Napi::Object resultObj = Napi::Object::New(node_rfc::__env);

        for (uint_t i = 0; i < paramCount; i++)
        {
            RfcGetParameterDescByIndex(functionDescHandle, i, &paramDesc, NULL);
            DEBUG("paramDesc %u %u %u", paramDesc.direction, client_options.filter_param_type, paramDesc.direction & client_options.filter_param_type);
            if ((paramDesc.direction & client_options.filter_param_type) == 0)
            {
                Napi::String name = wrapString(paramDesc.name).As<Napi::String>();
                Napi::Value value = wrapVariable(paramDesc.type, functionHandle, paramDesc.name, paramDesc.nucLength, paramDesc.typeDescHandle);
                (resultObj).Set(name, value);
            }
        }
        return scope.Escape(resultObj);
    }

    Napi::Value Client::wrapStructure(RFC_TYPE_DESC_HANDLE typeDesc, RFC_STRUCTURE_HANDLE structHandle)
    {
        Napi::EscapableHandleScope scope(node_rfc::__env);

        RFC_RC rc;
        RFC_ERROR_INFO errorInfo;
        RFC_FIELD_DESC fieldDesc;
        uint_t fieldCount;
        rc = RfcGetFieldCount(typeDesc, &fieldCount, &errorInfo);
        if (rc != RFC_OK)
        {
            Napi::Error::New(node_rfc::__env, wrapError(&errorInfo).ToString()).ThrowAsJavaScriptException();
        }

        Napi::Object resultObj = Napi::Object::New(node_rfc::__env);

        for (uint_t i = 0; i < fieldCount; i++)
        {
            rc = RfcGetFieldDescByIndex(typeDesc, i, &fieldDesc, &errorInfo);
            if (rc != RFC_OK)
            {
                Napi::Error::New(node_rfc::__env, wrapError(&errorInfo).ToString()).ThrowAsJavaScriptException();
            }
            (resultObj).Set(wrapString(fieldDesc.name), wrapVariable(fieldDesc.type, structHandle, fieldDesc.name, fieldDesc.nucLength, fieldDesc.typeDescHandle));
        }

        if (fieldCount == 1)
        {
            Napi::String fieldName = resultObj.GetPropertyNames().Get((uint32_t)0).As<Napi::String>();
            if (fieldName.Utf8Value().size() == 0)
            {
                return scope.Escape(resultObj.Get(fieldName));
            }
        }

        return scope.Escape(resultObj);
    }

    Napi::Value Client::wrapVariable(RFCTYPE typ, RFC_FUNCTION_HANDLE functionHandle, SAP_UC *cName, uint_t cLen, RFC_TYPE_DESC_HANDLE typeDesc)
    {
        Napi::EscapableHandleScope scope(node_rfc::__env);

        Napi::Value resultValue;

        RFC_RC rc = RFC_OK;
        RFC_ERROR_INFO errorInfo;
        RFC_STRUCTURE_HANDLE structHandle;

        switch (typ)
        {
        case RFCTYPE_STRUCTURE:
        {
            rc = RfcGetStructure(functionHandle, cName, &structHandle, &errorInfo);
            if (rc != RFC_OK)
            {
                break;
            }
            resultValue = wrapStructure(typeDesc, structHandle);
            break;
        }
        case RFCTYPE_TABLE:
        {
            RFC_TABLE_HANDLE tableHandle;
            rc = RfcGetTable(functionHandle, cName, &tableHandle, &errorInfo);
            if (rc != RFC_OK)
            {
                break;
            }
            uint_t rowCount;
            rc = RfcGetRowCount(tableHandle, &rowCount, &errorInfo);

            Napi::Array table = Napi::Array::New(node_rfc::__env);

            while (rowCount-- > 0)
            {
                RfcMoveTo(tableHandle, rowCount, NULL);
                Napi::Value row = wrapStructure(typeDesc, tableHandle);
                RfcDeleteCurrentRow(tableHandle, &errorInfo);
                (table).Set(rowCount, row);
            }
            resultValue = table;
            break;
        }
        case RFCTYPE_CHAR:
        {
            RFC_CHAR *charValue = (RFC_CHAR *)mallocU(cLen);
            rc = RfcGetChars(functionHandle, cName, charValue, cLen, &errorInfo);
            if (rc != RFC_OK)
            {
                break;
            }
            resultValue = wrapString(charValue, cLen);
            free(charValue);
            break;
        }
        case RFCTYPE_STRING:
        {
            uint_t resultLen = 0, strLen = 0;
            RfcGetStringLength(functionHandle, cName, &strLen, &errorInfo);
            SAP_UC *stringValue = (RFC_CHAR *)mallocU(strLen + 1);
            rc = RfcGetString(functionHandle, cName, stringValue, strLen + 1, &resultLen, &errorInfo);
            if (rc != RFC_OK)
            {
                break;
            }
            resultValue = wrapString(stringValue, strLen);
            free(stringValue);
            break;
        }
        case RFCTYPE_NUM:
        {
            RFC_NUM *numValue = (RFC_CHAR *)mallocU(cLen);
            rc = RfcGetNum(functionHandle, cName, numValue, cLen, &errorInfo);
            if (rc != RFC_OK)
            {
                free(numValue);
                break;
            }
            resultValue = wrapString(numValue, cLen);
            free(numValue);
            break;
        }
        case RFCTYPE_BYTE:
        {
            SAP_RAW *byteValue = (SAP_RAW *)malloc(cLen);

            //std::string fieldName = wrapString(cName).ToString().Utf8Value();
            //printf("\nbout %d %s cLen: %u", rc, &fieldName[0], cLen);

            rc = RfcGetBytes(functionHandle, cName, byteValue, cLen, &errorInfo);
            if (rc != RFC_OK)
            {
                free(byteValue);
                break;
            }
            resultValue = Napi::Buffer<char>::New(node_rfc::__env, reinterpret_cast<char *>(byteValue), cLen); // .As<Napi::Uint8Array>(); // as a buffer
            //resultValue = Napi::String::New(env, reinterpret_cast<const char *>(byteValue)); // or as a string
            // do not free byteValue - it will be freed when the buffer is garbage collected
            break;
        }

        case RFCTYPE_XSTRING:
        {
            SAP_RAW *byteValue;
            uint_t strLen, resultLen;
            rc = RfcGetStringLength(functionHandle, cName, &strLen, &errorInfo);

            byteValue = (SAP_RAW *)malloc(strLen + 1);
            byteValue[strLen] = '\0';

            rc = RfcGetXString(functionHandle, cName, byteValue, strLen, &resultLen, &errorInfo);

            //std::string fieldName = wrapString(cName).ToString().Utf8Value();
            //printf("\nxout %d %s cLen: %u strLen %u resultLen %u", rc, &fieldName[0], cLen, strLen, resultLen);

            if (rc != RFC_OK)
            {
                free(byteValue);
                break;
            }
            resultValue = Napi::Buffer<char>::New(node_rfc::__env, reinterpret_cast<char *>(byteValue), resultLen); // as a buffer
            //resultValue = Napi::String::New(node_rfc::__env, reinterpret_cast<const char *>(byteValue)); // or as a string
            // do not free byteValue - it will be freed when the buffer is garbage collected
            break;
        }
        case RFCTYPE_BCD:
        {
            // An upper bound for the length of the _string representation_
            // of the BCD is given by (2*cLen)-1 (each digit is encoded in 4bit,
            // the first 4 bit are reserved for the sign)
            // Furthermore, a sign char, a decimal separator char may be present
            // => (2*cLen)+1
            uint_t resultLen;
            uint_t strLen = 2 * cLen + 1;
            SAP_UC *sapuc = (SAP_UC *)mallocU(strLen + 1);
            rc = RfcGetString(functionHandle, cName, sapuc, strLen + 1, &resultLen, &errorInfo);
            if (rc == 23) // Buffer too small, use returned requried result length
            {
                //std::string fieldName = wrapString(cName).ToString().Utf8Value();
                //printf("\nWarning: Buffer for BCD type %d to small when wrapping %s\ncLen=%u, buffer=%u, trying with %u", typ, &fieldName[0], cLen, strLen, resultLen);
                free(sapuc);
                strLen = resultLen;
                sapuc = (SAP_UC *)mallocU(strLen + 1);
                rc = RfcGetString(functionHandle, cName, sapuc, strLen + 1, &resultLen, &errorInfo);
            }
            if (rc != RFC_OK)
            {
                free(sapuc);
                break;
            }
            resultValue = wrapString(sapuc, resultLen).ToString();
            free(sapuc);

            if (client_options.bcd == CLIENT_OPTION_BCD_FUNCTION)
            {
                resultValue = client_options.bcdFunction.Call({resultValue});
            }
            else if (client_options.bcd == CLIENT_OPTION_BCD_NUMBER)
            {
                resultValue = resultValue.ToNumber();
            }
            break;
        }
        case RFCTYPE_FLOAT:
        {
            RFC_FLOAT floatValue;
            rc = RfcGetFloat(functionHandle, cName, &floatValue, &errorInfo);
            resultValue = Napi::Number::New(node_rfc::__env, floatValue);
            break;
        }
        case RFCTYPE_DECF16:
        case RFCTYPE_DECF34:
        {
            // An upper bound for the length of the _string representation_
            // of the BCD is given by (2*cLen)-1 (each digit is encoded in 4bit,
            // the first 4 bit are reserved for the sign)
            // Furthermore, a sign char, a decimal separator char may be present
            // => (2*cLen)+1
            // and exponent char, sign and exponent
            // => +9
            uint_t resultLen;
            uint_t strLen = 2 * cLen + 10;
            SAP_UC *sapuc = (SAP_UC *)mallocU(strLen + 1);
            rc = RfcGetString(functionHandle, cName, sapuc, strLen + 1, &resultLen, &errorInfo);
            if (rc == 23) // Buffer too small, use returned requried result length
            {
                //std::string fieldName = wrapString(cName).ToString().Utf8Value();
                //printf("\nWarning: Buffer for BCD type %d to small when wrapping %s\ncLen=%u, buffer=%u, trying with %u", typ, &fieldName[0], cLen, strLen, resultLen);
                free(sapuc);
                strLen = resultLen;
                sapuc = (SAP_UC *)mallocU(strLen + 1);
                rc = RfcGetString(functionHandle, cName, sapuc, strLen + 1, &resultLen, &errorInfo);
            }
            if (rc != RFC_OK)
            {
                free(sapuc);
                break;
            }
            resultValue = wrapString(sapuc, resultLen).ToString();
            free(sapuc);

            if (client_options.bcd == CLIENT_OPTION_BCD_FUNCTION)
            {
                resultValue = client_options.bcdFunction.Call({resultValue});
            }
            else if (client_options.bcd == CLIENT_OPTION_BCD_NUMBER)
            {
                resultValue = resultValue.ToNumber();
            }
            break;
        }
        case RFCTYPE_INT:
        {
            RFC_INT intValue;
            rc = RfcGetInt(functionHandle, cName, &intValue, &errorInfo);
            if (rc != RFC_OK)
            {
                break;
            }
            resultValue = Napi::Number::New(node_rfc::__env, intValue);
            break;
        }
        case RFCTYPE_INT1:
        {
            RFC_INT1 intValue;
            rc = RfcGetInt1(functionHandle, cName, &intValue, &errorInfo);
            if (rc != RFC_OK)
            {
                break;
            }
            resultValue = Napi::Number::New(node_rfc::__env, intValue);
            break;
        }
        case RFCTYPE_INT2:
        {
            RFC_INT2 intValue;
            rc = RfcGetInt2(functionHandle, cName, &intValue, &errorInfo);
            if (rc != RFC_OK)
            {
                break;
            }
            resultValue = Napi::Number::New(node_rfc::__env, intValue);
            break;
        }
        case RFCTYPE_INT8:
        {
            RFC_INT8 intValue;
            rc = RfcGetInt8(functionHandle, cName, &intValue, &errorInfo);
            if (rc != RFC_OK)
            {
                break;
            }
            resultValue = Napi::Number::New(node_rfc::__env, (double)intValue);
            break;
        }
        case RFCTYPE_UTCLONG:
        {
            uint_t resultLen = 0, strLen = 27;
            SAP_UC *stringValue = (RFC_CHAR *)mallocU(strLen + 1);
            rc = RfcGetString(functionHandle, cName, stringValue, strLen + 1, &resultLen, &errorInfo);
            if (rc != RFC_OK)
            {
                break;
            }
            stringValue[19] = '.';
            resultValue = wrapString(stringValue, strLen);
            free(stringValue);
            break;
        }
        case RFCTYPE_DATE:
        {
            RFC_DATE dateValue;
            rc = RfcGetDate(functionHandle, cName, dateValue, &errorInfo);
            if (rc != RFC_OK)
            {
                break;
            }
            resultValue = wrapString(dateValue, 8);
            if (!client_options.dateFromABAP.IsEmpty())
            {
                resultValue = client_options.dateFromABAP.Call({resultValue});
            }
            break;
        }
        case RFCTYPE_TIME:
        {
            RFC_TIME timeValue;
            rc = RfcGetTime(functionHandle, cName, timeValue, &errorInfo);
            if (rc != RFC_OK)
            {
                break;
            }
            resultValue = wrapString(timeValue, 6);
            if (!client_options.timeFromABAP.IsEmpty())
            {
                resultValue = client_options.timeFromABAP.Call({resultValue});
            }
            break;
        }
        default:
            char err[256];
            std::string fieldName = wrapString(cName).ToString().Utf8Value();
            sprintf(err, "Unknown RFC type %d when wrapping %s", typ, &fieldName[0]);
            Napi::TypeError::New(node_rfc::__env, err).ThrowAsJavaScriptException();

            break;
        }
        if (rc != RFC_OK)
        {
            return scope.Escape(wrapError(&errorInfo));
        }

        return scope.Escape(resultValue);
    }
} // namespace node_rfc

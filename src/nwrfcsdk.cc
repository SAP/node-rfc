// SPDX-FileCopyrightText: 2014 SAP SE Srdjan Boskovic <srdjan.boskovic@sap.com>
//
// SPDX-License-Identifier: Apache-2.0
// language governing permissions and limitations under the License.

#include "nwrfcsdk.h"

namespace node_rfc
{
    extern Napi::Env __env;

    ////////////////////////////////////////////////////////////////////////////////
    // Set Parameters (to SDK)
    ////////////////////////////////////////////////////////////////////////////////

    SAP_UC *setString(const Napi::String napistr)
    {
        RFC_RC rc;
        RFC_ERROR_INFO errorInfo;
        SAP_UC *sapuc;
        uint_t sapucSize, resultLen = 0;

        std::string sstr = std::string(napistr);
        // std::string str = napistr.Utf8Value();
        sapucSize = sstr.length() + 1;

        sapuc = (SAP_UC *)mallocU(sapucSize);
        memsetU((SAP_UTF16 *)sapuc, 0, sapucSize);
        rc = RfcUTF8ToSAPUC((RFC_BYTE *)&sstr[0], sapucSize - 1, sapuc, &sapucSize, &resultLen, &errorInfo);
        // EDEBUG("fill: ", sstr, " sapucSize: ", sapucSize, " resultLen: ", resultLen, " code: ", errorInfo.code);
        if (rc != RFC_OK)
        {
            Napi::Error::Fatal("setString", "NodeJS string could not be parsed to ABAP string");
        }
        return sapuc;
    }

    SAP_UC *setString(std::string sstr)
    {
        RFC_RC rc;
        RFC_ERROR_INFO errorInfo;
        SAP_UC *sapuc;
        uint_t sapucSize, resultLen = 0;
        sapucSize = sstr.length() + 1;

        sapuc = (SAP_UC *)mallocU(sapucSize);
        memsetU((SAP_UTF16 *)sapuc, 0, sapucSize);
        rc = RfcUTF8ToSAPUC((RFC_BYTE *)&sstr[0], sapucSize - 1, sapuc, &sapucSize, &resultLen, &errorInfo);
        EDEBUG("fill: ", sstr, " sapucSize: ", sapucSize, " resultLen: ", resultLen, " code: ", errorInfo.code);

        if (rc != RFC_OK)
        {
            Napi::Error::Fatal("setString", "NodeJS string could not be parsed to ABAP string");
        }
        return sapuc;
    }

    Napi::Value setRfmParameter(RFC_FUNCTION_DESC_HANDLE functionDescHandle, RFC_FUNCTION_HANDLE functionHandle, Napi::String name, Napi::Value value, RfmErrorPath *errorPath, ClientOptionsStruct *client_options)
    {
        Napi::EscapableHandleScope scope(value.Env());

        RFC_RC rc;
        RFC_ERROR_INFO errorInfo;
        RFC_PARAMETER_DESC paramDesc;
        SAP_UC *cName = setString(name);
        errorPath->setParameterName(cName);
        rc = RfcGetParameterDescByName(functionDescHandle, cName, &paramDesc, &errorInfo);
        free(cName);
        if (rc != RFC_OK)
        {
            return scope.Escape(rfcSdkError(&errorInfo, errorPath));
        }
        return scope.Escape(setVariable(paramDesc.type, functionHandle, paramDesc.name, value, paramDesc.typeDescHandle, errorPath, client_options));
    }

    Napi::Value setStructure(RFC_STRUCTURE_HANDLE structHandle, RFC_TYPE_DESC_HANDLE functionDescHandle, SAP_UC *cName, Napi::Value value, RfmErrorPath *errorPath, ClientOptionsStruct *client_options)
    {
        RFC_RC rc;
        RFC_ERROR_INFO errorInfo;

        Napi::EscapableHandleScope scope(value.Env());

        Napi::Object structObj = value.ToObject();
        Napi::Array structNames = structObj.GetPropertyNames();
        uint_t structSize = structNames.Length();

        RFC_FIELD_DESC fieldDesc;

        Napi::Value retVal = value.Env().Undefined();

        for (uint_t i = 0; i < structSize; i++)
        {
            Napi::String name = structNames.Get(i).ToString();
            Napi::Value value = structObj.Get(name);

            SAP_UC *cValue = setString(name);
            rc = RfcGetFieldDescByName(functionDescHandle, cValue, &fieldDesc, &errorInfo);
            free(cValue);
            if (rc != RFC_OK)
            {
                errorPath->setFieldName(fieldDesc.name);
                retVal = rfcSdkError(&errorInfo, errorPath);
                break;
            }
            retVal = setVariable(fieldDesc.type, structHandle, fieldDesc.name, value, fieldDesc.typeDescHandle, errorPath, client_options);
            if (!retVal.IsUndefined())
            {
                break;
            }
        }
        return retVal;
    }

    Napi::Value setVariable(RFCTYPE typ, RFC_FUNCTION_HANDLE functionHandle, SAP_UC *cName, Napi::Value value, RFC_TYPE_DESC_HANDLE functionDescHandle, RfmErrorPath *errorPath, ClientOptionsStruct *client_options)
    {
        Napi::EscapableHandleScope scope(value.Env());
        RFC_RC rc = RFC_OK;
        RFC_ERROR_INFO errorInfo;
        SAP_UC *cValue;

        errorPath->setName(typ, cName);

        switch (typ)
        {
        case RFCTYPE_STRUCTURE:
        {
            RFC_STRUCTURE_HANDLE structHandle;
            rc = RfcGetStructure(functionHandle, cName, &structHandle, &errorInfo);
            if (rc != RFC_OK)
            {
                return scope.Escape(rfcSdkError(&errorInfo, errorPath));
            }
            Napi::Value rv = setStructure(structHandle, functionDescHandle, cName, value, errorPath, client_options);
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
                std::ostringstream err;
                err << "Array expected from NodeJS, for RFM table of type " << typ;
                return nodeRfcError(err.str(), errorPath);
            }
            Napi::Array array = value.As<Napi::Array>();
            uint_t rowCount = array.Length();

            for (uint_t i = 0; i < rowCount; i++)
            {
                errorPath->table_line = i;
                RFC_STRUCTURE_HANDLE structHandle = RfcAppendNewRow(tableHandle, &errorInfo);
                Napi::Value line = array.Get(i);
                if (line.IsBuffer() || line.IsString() || line.IsNumber())
                {
                    Napi::Object lineObj = Napi::Object::New(value.Env());
                    lineObj.Set(Napi::String::New(value.Env(), ""), line);
                    line = lineObj;
                }
                Napi::Value rv = setStructure(structHandle, functionDescHandle, cName, line, errorPath, client_options);
                if (!rv.IsUndefined())
                {
                    return scope.Escape(rv);
                }
            }
            break;
        }
        case RFCTYPE_BYTE:
        {
            if (!value.IsBuffer())
            {
                std::ostringstream err;
                err << "Buffer expected from NodeJS for the field of type " << typ;
                return nodeRfcError(err.str(), errorPath);
            }

            Napi::Buffer<SAP_RAW> js_buf = value.As<Napi::Buffer<SAP_RAW>>();
            uint_t js_buf_bytelen = js_buf.ByteLength();
            SAP_RAW *byteValue = (SAP_RAW *)malloc(js_buf_bytelen);
            memcpy(byteValue, js_buf.Data(), js_buf_bytelen);

            // excessive padding bytes sent from NodeJS, are silently trimmed by SDK to ABAP field length
            rc = RfcSetBytes(functionHandle, cName, byteValue, js_buf_bytelen, &errorInfo);
            free(byteValue);
            break;
        }
        case RFCTYPE_XSTRING:
        {
            if (!value.IsBuffer())
            {
                std::ostringstream err;
                err << "Buffer expected from NodeJS for the field of type " << typ;
                return nodeRfcError(err.str(), errorPath);
            }

            Napi::Buffer<SAP_RAW> js_buf = value.As<Napi::Buffer<SAP_RAW>>();
            uint_t js_buf_bytelen = js_buf.ByteLength();
            SAP_RAW *byteValue = (SAP_RAW *)malloc(js_buf_bytelen);
            memcpy(byteValue, js_buf.Data(), js_buf_bytelen);

            rc = RfcSetXString(functionHandle, cName, byteValue, js_buf_bytelen, &errorInfo);
            free(byteValue);
            break;
        }
        case RFCTYPE_CHAR:
        case RFCTYPE_STRING:
        {
            if (!value.IsString())
            {
                std::ostringstream err;
                err << "String expected from NodeJS for the field of type " << typ;
                return nodeRfcError(err.str(), errorPath);
            }
            cValue = setString(value.ToString());
            rc = RfcSetString(functionHandle, cName, cValue, strlenU(cValue), &errorInfo);
            free(cValue);
            break;
        }
        case RFCTYPE_NUM:
        {
            if (!value.IsString())
            {
                std::ostringstream err;
                err << "Char expected from NodeJS for the field of type " << typ;
                return nodeRfcError(err.str(), errorPath);
            }
            cValue = setString(value.ToString());
            rc = RfcSetNum(functionHandle, cName, cValue, strlenU(cValue), &errorInfo);
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
                std::ostringstream err;
                err << "Number, number object or string expected from NodeJS for the field of type " << typ;
                return nodeRfcError(err.str(), errorPath);
            }
            cValue = setString(value.ToString());
            rc = RfcSetString(functionHandle, cName, cValue, strlenU(cValue), &errorInfo);
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
                std::ostringstream err;
                err << "Integer number expected from NodeJS for the field of type " << typ;
                return nodeRfcError(err.str(), errorPath);
            }

            // https://github.com/mhdawson/node-sqlite3/pull/3
            double numDouble = value.ToNumber().DoubleValue();
            if ((int64_t)numDouble != numDouble) // or std::trunc(numDouble) == numDouble;
            {
                std::ostringstream err;
                err << "Integer number expected from NodeJS for the field of type " << typ << ", got " << value.ToString().Utf8Value();
                return nodeRfcError(err.str(), errorPath);
            }
            RFC_INT rfcInt = (RFC_INT)value.As<Napi::Number>().Int64Value();
            // int64_t rfcInt = value.As<Napi::Number>().Int64Value();
            // printf("typ: %d value: %d %u", typ, rfcInt, UINT8_MAX);
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
                    std::ostringstream err;
                    err << "Overflow or other error when putting NodeJS value " << rfcInt << " into integer field of type " << typ;
                    return nodeRfcError(err.str(), errorPath);
                }

                rc = RfcSetInt(functionHandle, cName, rfcInt, &errorInfo);
            }
            break;
        }
        case RFCTYPE_UTCLONG:
        {
            if (!value.IsString())
            {
                std::ostringstream err;
                err << "UTCLONG string expected from NodeJS for the field of type " << typ;
                return nodeRfcError(err.str(), errorPath);
            }
            cValue = setString(value.ToString());
            rc = RfcSetString(functionHandle, cName, cValue, strlenU(cValue), &errorInfo);
            free(cValue);
            break;
        }
        case RFCTYPE_DATE:
        {
            if (!client_options->dateToABAP.IsEmpty())
            {
                // YYYYMMDD format expected
                value = client_options->dateToABAP.Call({value});
            }
            if (!value.IsString())
            {
                std::ostringstream err;
                err << "ABAP date format YYYYMMDD expected from NodeJS for the field of type " << typ;
                return nodeRfcError(err.str(), errorPath);
            }
            cValue = setString(value.ToString());
            rc = RfcSetDate(functionHandle, cName, cValue, &errorInfo);
            free(cValue);
            break;
        }
        case RFCTYPE_TIME:
        {
            if (!client_options->timeToABAP.IsEmpty())
            {
                // HHMMSS format expected
                value = client_options->timeToABAP.Call({value});
            }
            if (!value.IsString())
            {
                std::ostringstream err;
                err << "ABAP time format HHMMSS expected from NodeJS for the field of type " << typ;
                return nodeRfcError(err.str(), errorPath);
            }
            cValue = setString(value.ToString());
            rc = RfcSetTime(functionHandle, cName, cValue, &errorInfo);
            free(cValue);
            break;
        }
        default:
        {
            std::ostringstream err;

            err << "Unknown RFC type from NodeJS " << typ;
            return nodeRfcError(err.str(), errorPath);
        }
        }
        if (rc != RFC_OK)
        {
            return scope.Escape(rfcSdkError(&errorInfo, errorPath));
        }
        return scope.Env().Undefined();
    }

    ////////////////////////////////////////////////////////////////////////////////
    // Get Parameters (from SDK)
    ////////////////////////////////////////////////////////////////////////////////

    Napi::Value wrapString(SAP_UC *uc, int length)
    {
        RFC_ERROR_INFO errorInfo;

        Napi::EscapableHandleScope scope(node_rfc::__env);

        if (length == -1)
        {
            length = strlenU(uc);
        }
        if (length == 0)
        {
            return scope.Escape(Napi::String::New(node_rfc::__env, ""));
        }
        // try with 3 bytes per unicode character
        uint_t utf8Size = length * 3;
        char *utf8 = (char *)malloc(utf8Size + 1);
        utf8[0] = '\0';
        uint_t resultLen = 0;
        RfcSAPUCToUTF8(uc, length, (RFC_BYTE *)utf8, &utf8Size, &resultLen, &errorInfo);
        // EDEBUG("len: ", length, " utf8Size: ", utf8Size, " resultLen: ", resultLen, " ", errorInfo.code);
        if (errorInfo.code != RFC_OK)
        {
            // not enough, try with 5
            free(utf8);
            utf8Size = length * 5;
            utf8 = (char *)malloc(utf8Size + 1);
            utf8[0] = '\0';
            resultLen = 0;
            RfcSAPUCToUTF8(uc, length, (RFC_BYTE *)utf8, &utf8Size, &resultLen, &errorInfo);
            if (errorInfo.code != RFC_OK)
            {
                free(utf8);
                return node_rfc::__env.Undefined();
            }
        }

        // trim trailing whitespaces
        int i = resultLen - 1;
        while (i >= 0 && isspace(utf8[i]))
        {
            i--;
        }
        utf8[i + 1] = '\0';
        Napi::Value resultValue = Napi::String::New(node_rfc::__env, std::string(utf8, i + 1));
        free((char *)utf8);
        return scope.Escape(resultValue);
    }

    ValuePair getRfmParameters(RFC_FUNCTION_DESC_HANDLE functionDescHandle, RFC_FUNCTION_HANDLE functionHandle, RfmErrorPath *errorPath, ClientOptionsStruct *client_options)
    {
        Napi::EscapableHandleScope scope(node_rfc::__env);

        RFC_PARAMETER_DESC paramDesc;

        uint_t paramCount = 0;

        RfcGetParameterCount(functionDescHandle, &paramCount, NULL);
        Napi::Object resultObj = Napi::Object::New(node_rfc::__env);

        for (uint_t i = 0; i < paramCount; i++)
        {
            RfcGetParameterDescByIndex(functionDescHandle, i, &paramDesc, NULL);
            if ((paramDesc.direction & client_options->filter_param_type) == 0)
            {
                Napi::String name = wrapString(paramDesc.name).As<Napi::String>();
                errorPath->setParameterName(paramDesc.name);
                // DEBUG("param type ", paramDesc.type, " name ", wrapString(paramDesc.name).As<Napi::String>().Utf8Value(), " direction ", paramDesc.direction, " filter ", paramDesc.direction & client_options.filter_param_type);
                ValuePair result = getVariable(paramDesc.type, functionHandle, paramDesc.name, paramDesc.nucLength, paramDesc.typeDescHandle, errorPath, client_options);
                if (!result.first.IsUndefined())
                {
                    return result;
                }
                (resultObj).Set(name, result.second);
            }
        }
        return ValuePair(ENV_UNDEFINED, scope.Escape(resultObj));
    }

    ValuePair getStructure(RFC_TYPE_DESC_HANDLE typeDesc, RFC_STRUCTURE_HANDLE structHandle, RfmErrorPath *errorPath, ClientOptionsStruct *client_options)
    {
        Napi::EscapableHandleScope scope(node_rfc::__env);

        Napi::Object resultObj = Napi::Object::New(node_rfc::__env);

        RFC_RC rc;
        RFC_ERROR_INFO errorInfo;
        RFC_FIELD_DESC fieldDesc;
        uint_t fieldCount;
        rc = RfcGetFieldCount(typeDesc, &fieldCount, &errorInfo);
        if (rc != RFC_OK)
        {
            return ValuePair(rfcSdkError(&errorInfo, errorPath), ENV_UNDEFINED);
        }

        for (uint_t i = 0; i < fieldCount; i++)
        {
            rc = RfcGetFieldDescByIndex(typeDesc, i, &fieldDesc, &errorInfo);
            if (rc != RFC_OK)
            {
                errorPath->setFieldName(fieldDesc.name);
                return ValuePair(rfcSdkError(&errorInfo, errorPath), ENV_UNDEFINED);
            }
            ValuePair result = getVariable(fieldDesc.type, structHandle, fieldDesc.name, fieldDesc.nucLength, fieldDesc.typeDescHandle, errorPath, client_options);
            if (!result.first.IsUndefined())
            {
                return result;
            }
            // EDEBUG("F: ", wrapString(fieldDesc.name).As<Napi::String>().Utf8Value());
            (resultObj).Set(wrapString(fieldDesc.name), result.second);
        }

        if (fieldCount == 1)
        {
            Napi::String fieldName = resultObj.GetPropertyNames().Get((uint_t)0).As<Napi::String>();
            if (fieldName.Utf8Value().size() == 0)
            {
                return ValuePair(ENV_UNDEFINED, scope.Escape(resultObj.Get(fieldName)));
            }
        }

        return ValuePair(ENV_UNDEFINED, scope.Escape(resultObj));
    }

    ValuePair getVariable(RFCTYPE typ, RFC_FUNCTION_HANDLE functionHandle, SAP_UC *cName, uint_t cLen, RFC_TYPE_DESC_HANDLE typeDesc, RfmErrorPath *errorPath, ClientOptionsStruct *client_options)
    {
        Napi::EscapableHandleScope scope(node_rfc::__env);
        Napi::Value resultValue = ENV_UNDEFINED;

        RFC_RC rc = RFC_OK;
        RFC_ERROR_INFO errorInfo;
        RFC_STRUCTURE_HANDLE structHandle;

        errorPath->setName(typ, cName);

        switch (typ)
        {
        case RFCTYPE_STRUCTURE:
        {
            rc = RfcGetStructure(functionHandle, cName, &structHandle, &errorInfo);
            if (rc != RFC_OK)
            {
                break;
            }

            ValuePair result = getStructure(typeDesc, structHandle, errorPath, client_options);
            if (!result.first.IsUndefined())
            {
                return result;
            }
            resultValue = result.second;
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
                errorPath->table_line = rowCount;
                RfcMoveTo(tableHandle, rowCount, NULL);
                ValuePair result = getStructure(typeDesc, tableHandle, errorPath, client_options);
                if (!result.first.IsUndefined())
                {
                    return result;
                }
                RfcDeleteCurrentRow(tableHandle, &errorInfo);
                (table).Set(rowCount, result.second);
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

            rc = RfcGetBytes(functionHandle, cName, byteValue, cLen, &errorInfo);
            if (rc != RFC_OK)
            {
                free(byteValue);
                break;
            }
            resultValue = Napi::Buffer<SAP_RAW>::New(node_rfc::__env, reinterpret_cast<SAP_RAW *>(byteValue), cLen, [](Env env, SAP_RAW *byteValue)
                                                     { free(byteValue); });
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

            if (rc != RFC_OK)
            {
                free(byteValue);
                break;
            }
            resultValue = Napi::Buffer<SAP_RAW>::New(node_rfc::__env, reinterpret_cast<SAP_RAW *>(byteValue), resultLen, [](Env env, SAP_RAW *byteValue)
                                                     { free(byteValue); });
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
                EDEBUG("Warning: Buffer for BCD type ", typ, " too small, for ", errorPath->pathstr());
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

            if (client_options->bcd == CLIENT_OPTION_BCD_FUNCTION)
            {
                resultValue = client_options->bcdFunction.Call({resultValue});
            }
            else if (client_options->bcd == CLIENT_OPTION_BCD_NUMBER)
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
                EDEBUG("Warning: Buffer for BCD type ", typ, " too small, for ", errorPath->pathstr());
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

            if (client_options->bcd == CLIENT_OPTION_BCD_FUNCTION)
            {
                resultValue = client_options->bcdFunction.Call({resultValue});
            }
            else if (client_options->bcd == CLIENT_OPTION_BCD_NUMBER)
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
            if (!client_options->dateFromABAP.IsEmpty())
            {
                resultValue = client_options->dateFromABAP.Call({resultValue});
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
            if (!client_options->timeFromABAP.IsEmpty())
            {
                resultValue = client_options->timeFromABAP.Call({resultValue});
            }
            break;
        }
        default:
            std::ostringstream err;
            err << "RFC type from ABAP not supported" << typ;
            return ValuePair(ENV_UNDEFINED, nodeRfcError(err.str(), errorPath));
            break;
        }

        if (rc != RFC_OK)
        {
            return ValuePair(scope.Escape(rfcSdkError(&errorInfo, errorPath)), ENV_UNDEFINED);
        }

        if (resultValue.IsUndefined())
        {
            return ValuePair(scope.Escape(nodeRfcError("Non-unicode ABAP string", errorPath)), ENV_UNDEFINED);
        }

        return ValuePair(ENV_UNDEFINED, scope.Escape(resultValue));
    }

    ////////////////////////////////////////////////////////////////////////////////
    // NODE-RFC ERRORS
    ////////////////////////////////////////////////////////////////////////////////

    Napi::Value nodeRfcError(std::string message, RfmErrorPath *errorPath)
    {
        Napi::EscapableHandleScope scope(node_rfc::__env);
        Napi::Object errorObj = Napi::Object::New(node_rfc::__env);
        errorObj.Set("name", "nodeRfcError");
        errorObj.Set("message", message);
        if (errorPath != NULL)
        {
            errorObj.Set("rfmPath", errorPath->getpath());
        }
        return scope.Escape(errorObj);
    }

    ////////////////////////////////////////////////////////////////////////////////
    // NWRFC SDK ERRORS
    ////////////////////////////////////////////////////////////////////////////////

    Napi::Object RfcLibError(RFC_ERROR_INFO *errorInfo)
    {
        Napi::Object errorObj = Napi::Object::New(node_rfc::__env);
        (errorObj).Set("name", "RfcLibError");
        (errorObj).Set("group", Napi::Number::New(node_rfc::__env, errorInfo->group));
        (errorObj).Set("code", Napi::Number::New(node_rfc::__env, errorInfo->code));
        (errorObj).Set("codeString", wrapString((SAP_UC *)RfcGetRcAsString(errorInfo->code)));
        (errorObj).Set("key", wrapString(errorInfo->key));
        (errorObj).Set("message", wrapString(errorInfo->message));
        return errorObj;
    }

    Napi::Object AbapError(RFC_ERROR_INFO *errorInfo)
    {
        Napi::Object errorObj = Napi::Object::New(node_rfc::__env);
        (errorObj).Set("name", "ABAPError");
        (errorObj).Set("group", Napi::Number::New(node_rfc::__env, errorInfo->group));
        (errorObj).Set("code", Napi::Number::New(node_rfc::__env, errorInfo->code));
        (errorObj).Set("codeString", wrapString((SAP_UC *)RfcGetRcAsString(errorInfo->code)));
        (errorObj).Set("key", wrapString(errorInfo->key));
        (errorObj).Set("message", wrapString(errorInfo->message));
        (errorObj).Set("abapMsgClass", wrapString(errorInfo->abapMsgClass));
        (errorObj).Set("abapMsgType", wrapString(errorInfo->abapMsgType));
        (errorObj).Set("abapMsgNumber", wrapString(errorInfo->abapMsgNumber));
        (errorObj).Set("abapMsgV1", wrapString(errorInfo->abapMsgV1));
        (errorObj).Set("abapMsgV2", wrapString(errorInfo->abapMsgV2));
        (errorObj).Set("abapMsgV3", wrapString(errorInfo->abapMsgV3));
        (errorObj).Set("abapMsgV4", wrapString(errorInfo->abapMsgV4));

        return errorObj;
    }

    Napi::Value rfcSdkError(RFC_ERROR_INFO *errorInfo, RfmErrorPath *errorPath)
    {
        Napi::EscapableHandleScope scope(node_rfc::__env);
        Napi::Object errorObj;

        switch (errorInfo->group)
        {
        case OK: // 0: should never happen
            errorObj = nodeRfcError("rfcSdkError invoked with the RFC error group OK").As<Napi::Object>();
            break;

        case LOGON_FAILURE:            // 3: Error message raised when logon fails
        case COMMUNICATION_FAILURE:    // 4: Problems with the network connection (or backend broke down and killed the connection)
        case EXTERNAL_RUNTIME_FAILURE: // 5: Problems in the RFC runtime of the external program (i.e "this" library)
            errorObj = RfcLibError(errorInfo);
            break;

        case ABAP_APPLICATION_FAILURE:       // 1: ABAP Exception raised in ABAP function modules
        case ABAP_RUNTIME_FAILURE:           // 2: ABAP Message raised in ABAP function modules or in ABAP runtime of the backend (e.g Kernel)
        case EXTERNAL_APPLICATION_FAILURE:   // 6: Problems in the external program (e.g in the external server implementation)
        case EXTERNAL_AUTHORIZATION_FAILURE: // 7: Problems raised in the authorization check handler provided by the external server implementation
            errorObj = AbapError(errorInfo);
            break;
        default:
            std::ostringstream err;
            err << "wrapError invoked with an unknown err group:" << errorInfo->group;
            errorObj = nodeRfcError(err.str()).As<Napi::Object>();
        }

        if (errorPath != NULL)
        {
            errorObj.Set("rfmPath", errorPath->getpath());
        }
        return scope.Escape(errorObj);
    }

    ////////////////////////////////////////////////////////////////////////////////
    // Connection parameters and client options parsers
    ////////////////////////////////////////////////////////////////////////////////

    void getConnectionParams(Napi::Object clientParamsObject, ConnectionParamsStruct *clientParams)
    {

        Napi::Array paramNames = clientParamsObject.GetPropertyNames();
        clientParams->paramSize = paramNames.Length();
        if (clientParams->paramSize == 0)
        {
            Napi::TypeError::New(node_rfc::__env, "Client connection parameters missing").ThrowAsJavaScriptException();
            return;
        }
        DEBUG("getConnectionParams ", clientParams->paramSize);
        clientParams->connectionParams = static_cast<RFC_CONNECTION_PARAMETER *>(malloc(clientParams->paramSize * sizeof(RFC_CONNECTION_PARAMETER)));
        for (uint_t ii = 0; ii < clientParams->paramSize; ii++)
        {
            Napi::String name = paramNames.Get(ii).ToString();
            Napi::String value = clientParamsObject.Get(name).ToString();
            DEBUG("getConnectionParams ", name.Utf8Value(), ": ", value.Utf8Value());
            clientParams->connectionParams[ii].name = setString(name);
            clientParams->connectionParams[ii].value = setString(value);
        }
    }

    void checkClientOptions(Napi::Object clientOptionsObject, ClientOptionsStruct *client_options)
    {
        char errmsg[ERRMSG_LENGTH];
        Napi::Array props = clientOptionsObject.GetPropertyNames();
        for (uint_t ii = 0; ii < props.Length(); ii++)
        {
            std::string key = props.Get(ii).ToString().Utf8Value();
            Napi::Value opt = clientOptionsObject.Get(key).As<Napi::Value>();

            // Client option: "bcd"
            if (key.compare(std::string(CLIENT_OPTION_KEY_BCD)) == 0)
            {
                if (opt.IsFunction())
                {
                    client_options->bcd = CLIENT_OPTION_BCD_FUNCTION;
                    client_options->bcdFunction = Napi::Persistent(opt.As<Napi::Function>());
                }
                else if (opt.IsString())
                {
                    std::string bcdString = opt.ToString().Utf8Value();
                    if (bcdString.compare(std::string("number")) == 0)
                    {
                        client_options->bcd = CLIENT_OPTION_BCD_NUMBER;
                    }
                    else
                    {
                        snprintf(errmsg, ERRMSG_LENGTH - 1, "Client option \"%s\" value not allowed: \"%s\"; see %s", CLIENT_OPTION_KEY_BCD, &bcdString[0], USAGE_URL);
                        Napi::TypeError::New(node_rfc::__env, errmsg).ThrowAsJavaScriptException();
                    }
                }
            }

            // Client option: "date"
            else if (key.compare(std::string(CLIENT_OPTION_KEY_DATE)) == 0)
            {
                if (!opt.IsObject())
                {
                    opt = node_rfc::__env.Null();
                }
                else
                {
                    Napi::Value toABAP = opt.As<Napi::Object>().Get("toABAP");
                    Napi::Value fromABAP = opt.As<Napi::Object>().Get("fromABAP");
                    if (!toABAP.IsFunction() || !fromABAP.IsFunction())
                    {
                        snprintf(errmsg, ERRMSG_LENGTH - 1, "Client option \"%s\" is not an object with toABAP() and fromABAP() functions; see %s", CLIENT_OPTION_KEY_DATE, USAGE_URL);
                        Napi::TypeError::New(node_rfc::__env, errmsg).ThrowAsJavaScriptException();
                    }
                    else
                    {
                        client_options->dateToABAP = Napi::Persistent(toABAP.As<Napi::Function>());
                        client_options->dateFromABAP = Napi::Persistent(fromABAP.As<Napi::Function>());
                    }
                }
            }

            // Client option: "time"
            else if (key.compare(std::string(CLIENT_OPTION_KEY_TIME)) == 0)
            {
                if (!opt.IsObject())
                {
                    opt = node_rfc::__env.Null();
                }
                else
                {
                    Napi::Value toABAP = opt.As<Napi::Object>().Get("toABAP");
                    Napi::Value fromABAP = opt.As<Napi::Object>().Get("fromABAP");
                    if (!toABAP.IsFunction() || !fromABAP.IsFunction())
                    {
                        snprintf(errmsg, ERRMSG_LENGTH - 1, "Client option \"%s\" is not an object with toABAP() and fromABAP() functions; see %s", CLIENT_OPTION_KEY_TIME, USAGE_URL);
                        Napi::TypeError::New(node_rfc::__env, errmsg).ThrowAsJavaScriptException();
                        ;
                    }
                    else
                    {
                        client_options->timeToABAP = Napi::Persistent(toABAP.As<Napi::Function>());
                        client_options->timeFromABAP = Napi::Persistent(fromABAP.As<Napi::Function>());
                    }
                }
            }

            // Client option: "filter"
            else if (key.compare(std::string(CLIENT_OPTION_KEY_FILTER)) == 0)
            {
                client_options->filter_param_type = (RFC_DIRECTION)clientOptionsObject.Get(key).As<Napi::Number>().Uint32Value();
                if (((int)client_options->filter_param_type < 1) || ((int)client_options->filter_param_type) > 4)
                {
                    snprintf(errmsg, ERRMSG_LENGTH - 1, "Client option \"%s\" value allowed: \"%u\"; see %s", CLIENT_OPTION_KEY_FILTER, (uint_t)client_options->filter_param_type, USAGE_URL);
                    Napi::TypeError::New(node_rfc::__env, errmsg).ThrowAsJavaScriptException();
                }
            }

            // Client option: "stateless"
            else if (key.compare(std::string(CLIENT_OPTION_KEY_STATELESS)) == 0)
            {
                if (!clientOptionsObject.Get(key).IsBoolean())
                {
                    snprintf(errmsg, ERRMSG_LENGTH - 1, "Client option \"%s\" requires a boolean value; see %s", CLIENT_OPTION_KEY_STATELESS, USAGE_URL);
                    Napi::TypeError::New(node_rfc::__env, errmsg).ThrowAsJavaScriptException();
                }
                client_options->stateless = clientOptionsObject.Get(key).As<Napi::Boolean>();
            }

            // Client option: "timeout"
            else if (key.compare(std::string(CLIENT_OPTION_KEY_TIMEOUT)) == 0)
            {
                if (!clientOptionsObject.Get(key).IsNumber())
                {
                    snprintf(errmsg, ERRMSG_LENGTH - 1, "Client option \"%s\" requires a number of seconds; see %s", CLIENT_OPTION_KEY_TIMEOUT, USAGE_URL);
                    Napi::TypeError::New(node_rfc::__env, errmsg).ThrowAsJavaScriptException();
                }
                client_options->timeout = clientOptionsObject.Get(key).As<Napi::Number>();
            }

            // Client option: unknown
            else
            {
                snprintf(errmsg, ERRMSG_LENGTH - 1, "Client option not allowed: \"%s\"; see %s", key.c_str(), USAGE_URL);
                Napi::TypeError::New(node_rfc::__env, errmsg).ThrowAsJavaScriptException();
            }
        }
    }

} // namespace node_rfc

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

#include "rfcio.h"
#include "error.h"

namespace node_rfc
{

extern Napi::Env __genv;

////////////////////////////////////////////////////////////////////////////////
// FILL FUNCTIONS (to RFC)
////////////////////////////////////////////////////////////////////////////////

SAP_UC *fillString(const Napi::String napistr)
{
    RFC_RC rc;
    RFC_ERROR_INFO errorInfo;
    SAP_UC *sapuc;
    unsigned int sapucSize, resultLen = 0;

    //std::string str = std::string(napistr);
    std::string str = napistr.Utf8Value();
    sapucSize = str.length() + 1;

    // printf("%s: %u\n", &str[0], sapucSize);

    sapuc = mallocU(sapucSize);
    memsetU(sapuc, 0, sapucSize);
    rc = RfcUTF8ToSAPUC((RFC_BYTE *)&str[0], str.length(), sapuc, &sapucSize, &resultLen, &errorInfo);

    if (rc != RFC_OK)
        Napi::Error::Fatal("fillString", "node-rfc internal error");

    return sapuc;
}

Napi::Value fillFunctionParameter(RFC_FUNCTION_DESC_HANDLE functionDescHandle, RFC_FUNCTION_HANDLE functionHandle, Napi::String name, Napi::Value value)
{
    Napi::EscapableHandleScope scope(__genv);

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

Napi::Value fillVariable(RFCTYPE typ, RFC_FUNCTION_HANDLE functionHandle, SAP_UC *cName, Napi::Value value, RFC_TYPE_DESC_HANDLE functionDescHandle)
{
    Napi::EscapableHandleScope scope(__genv);
    RFC_RC rc = RFC_OK;
    RFC_ERROR_INFO errorInfo;
    RFC_STRUCTURE_HANDLE structHandle;
    RFC_TABLE_HANDLE tableHandle;
    SAP_UC *cValue;

    // FIXME https://github.com/nodejs/node-addon-api/issues/265
    bool isNumber, isInteger;
    float numFloat;

    switch (typ)
    {
    case RFCTYPE_STRUCTURE:
    {
        rc = RfcGetStructure(functionHandle, cName, &structHandle, &errorInfo);
        if (rc != RFC_OK)
        {
            break;
        }
        Napi::Object structObj = value.ToObject(); // ->ToObject();
        Napi::Array structNames = structObj.GetPropertyNames();
        unsigned int structSize = structNames.Length();

        RFC_FIELD_DESC fieldDesc;
        for (unsigned int i = 0; i < structSize; i++)
        {
            Napi::String name = structNames.Get(i).ToString();
            Napi::Value value = structObj.Get(name);

            cValue = fillString(name); // cValue = cName
            rc = RfcGetFieldDescByName(functionDescHandle, cValue, &fieldDesc, &errorInfo);
            free(cValue);
            if (rc != RFC_OK)
            {
                return scope.Escape(wrapError(&errorInfo));
            }
            Napi::Value fillError = fillVariable(fieldDesc.type, structHandle, fieldDesc.name, value, fieldDesc.typeDescHandle);
            if (fillError != scope.Env().Null())
            {
                return scope.Escape(fillError);
            }
        }
        break;
    }
    case RFCTYPE_TABLE:
    {
        rc = RfcGetTable(functionHandle, cName, &tableHandle, &errorInfo);

        if (rc != RFC_OK)
        {
            break;
        }
        Napi::Array array = value.As<Napi::Array>();
        unsigned int rowCount = array.Length();

        for (unsigned int i = 0; i < rowCount; i++)
        {
            structHandle = RfcAppendNewRow(tableHandle, &errorInfo);
            if (structHandle == NULL)
            {
                rc = RFC_INVALID_HANDLE;
                break;
            }

            // FIXME: DRY from RFCTYPE_STRUCTURE!
            Napi::Object structObj = array.Get(i).As<Napi::Object>();
            Napi::Array structNames = structObj.GetPropertyNames();
            unsigned int structSize = structNames.Length();

            RFC_FIELD_DESC fieldDesc;
            for (unsigned int i = 0; i < structSize; i++)
            {
                Napi::String name = structNames.Get(i).ToString();
                Napi::Value value = structObj.Get(name);

                cValue = fillString(name); // cValue = cName
                rc = RfcGetFieldDescByName(functionDescHandle, cValue, &fieldDesc, &errorInfo);
                free(cValue);
                if (rc != RFC_OK)
                {
                    return scope.Escape(wrapError(&errorInfo));
                }
                Napi::Value fillError = fillVariable(fieldDesc.type, structHandle, fieldDesc.name, value, fieldDesc.typeDescHandle);
                if (fillError != scope.Env().Null())
                {
                    return scope.Escape(fillError);
                }
            }
            // FIXME: END DRY
        }
        break;
    }
    case RFCTYPE_CHAR:
        if (!value.IsString())
        {
            char err[256];
            std::string fieldName = wrapString(cName).ToString().Utf8Value();
            sprintf(err, "Char expected when filling field %s of type %d", &fieldName[0], typ);
            return scope.Escape(Napi::TypeError::New(__genv, err).Value());
        }
        cValue = fillString(value.As<Napi::String>());
        rc = RfcSetChars(functionHandle, cName, cValue, strlenU(cValue), &errorInfo);
        free(cValue);
        break;
    case RFCTYPE_BYTE:
    {
        std::string str = value.ToString().Utf8Value();
        char *asciiValue = &str[0];
        unsigned int size = str.length();
        SAP_RAW *byteValue = (SAP_RAW *)malloc(size);
        memcpy(byteValue, reinterpret_cast<SAP_RAW *>(asciiValue), size);
        rc = RfcSetBytes(functionHandle, cName, byteValue, size, &errorInfo);
        free(byteValue);
        break;
    }
    case RFCTYPE_XSTRING:
    {
        std::string str = value.ToString().Utf8Value();
        char *asciiValue = &str[0];
        unsigned int size = str.length();
        SAP_RAW *byteValue = (SAP_RAW *)malloc(size);
        memcpy(byteValue, reinterpret_cast<SAP_RAW *>(asciiValue), size);
        rc = RfcSetXString(functionHandle, cName, byteValue, size, &errorInfo);
        free(byteValue);
        break;
    }
    case RFCTYPE_STRING:
        if (!value.IsString())
        {
            char err[256];
            std::string fieldName = wrapString(cName).ToString().Utf8Value();
            sprintf(err, "Char expected when filling field %s of type %d", &fieldName[0], typ);
            return scope.Escape(Napi::TypeError::New(__genv, err).Value());
        }
        cValue = fillString(value.ToString());
        rc = RfcSetString(functionHandle, cName, cValue, strlenU(cValue), &errorInfo);
        free(cValue);
        break;
    case RFCTYPE_NUM:
        if (!value.IsString())
        {
            char err[256];
            std::string fieldName = wrapString(cName).ToString().Utf8Value();
            sprintf(err, "Char expected when filling field %s of type %d", &fieldName[0], typ);
            return scope.Escape(Napi::TypeError::New(__genv, err).Value());
        }
        cValue = fillString(value.ToString());
        rc = RfcSetNum(functionHandle, cName, cValue, strlenU(cValue), &errorInfo);
        free(cValue);
        break;
    case RFCTYPE_BCD: // fallthrough
    case RFCTYPE_FLOAT:
        if (!value.IsNumber() && !value.IsObject() && !value.IsString())
        {
            char err[256];
            std::string fieldName = wrapString(cName).ToString().Utf8Value();
            sprintf(err, "Number, number object or string expected when filling field %s of type %d", &fieldName[0], typ);
            return scope.Escape(Napi::TypeError::New(__genv, err).Value());
        }
        cValue = fillString(value.ToString());
        rc = RfcSetString(functionHandle, cName, cValue, strlenU(cValue), &errorInfo);
        free(cValue);
        break;
    case RFCTYPE_INT:
    case RFCTYPE_INT1:
    case RFCTYPE_INT2:
        // FIXME https://github.com/nodejs/node-addon-api/issues/265
        isNumber = value.IsNumber();
        if (isNumber)
        {
            // Napi::Function isIntegerFunction = value.Env().Global().Get("Number").As<Napi::Object>().Get("IsInteger").As<Napi::Function>();
            // bool isInt = isIntegerFunction.MakeCallback(__genv.Global(), {value}).ToBoolean().Value();
            // printf("%u %f : %u", value.ToNumber(), value.ToNumber(), isInt);

            numFloat = value.ToNumber().FloatValue();
            isInteger = (int)numFloat == numFloat;
            // or isInteger = std::trunc(numFloat) == numFloat;
        }
        else
        {
            char err[256];
            std::string fieldName = wrapString(cName).ToString().Utf8Value();
            sprintf(err, "Integer number expected when filling field %s of type %d", &fieldName[0], typ);
            return scope.Escape(Napi::TypeError::New(__genv, err).Value());
        }
        if (!isInteger)
        {
            char err[256];
            std::string fieldName = wrapString(cName).ToString().Utf8Value();
            sprintf(err, "Integer number expected when filling field %s of type %d, got %f", &fieldName[0], typ, numFloat);
            return scope.Escape(Napi::TypeError::New(__genv, err).Value());
        }
        rc = RfcSetInt(functionHandle, cName, RFC_INT(numFloat), &errorInfo);
        break;
    case RFCTYPE_DATE:
        // https: //github.com/nodejs/node-addon-api/issues/57#issuecomment-398970543
        if (!value.IsString())
        {
            char err[256];
            std::string fieldName = wrapString(cName).ToString().Utf8Value();
            sprintf(err, "Date object or string expected when filling field %s of type %d", &fieldName[0], typ);
            return scope.Escape(Napi::TypeError::New(__genv, err).Value());
        }
        //cValue = fillString(value.strftime('%Y%m%d'));
        cValue = fillString(value.ToString());
        rc = RfcSetDate(functionHandle, cName, cValue, &errorInfo);
        free(cValue);
        break;
    case RFCTYPE_TIME:
        if (!value.IsString())
        {
            char err[256];
            std::string fieldName = wrapString(cName).ToString().Utf8Value();
            sprintf(err, "Char expected when filling field %s of type %d", &fieldName[0], typ);
            return scope.Escape(Napi::TypeError::New(__genv, err).Value());
        }
        //cValue = fillString(value.strftime('%H%M%S'))
        cValue = fillString(value.ToString());
        rc = RfcSetTime(functionHandle, cName, cValue, &errorInfo);
        free(cValue);
        break;
    default:
        char err[256];
        std::string fieldName = wrapString(cName).ToString().Utf8Value();
        sprintf(err, "Unknown RFC type %u when filling %s", typ, &fieldName[0]);
        return scope.Escape(Napi::TypeError::New(__genv, err).Value());
        break;
    }
    if (rc != RFC_OK)
    {
        return scope.Escape(wrapError(&errorInfo));
    }
    return scope.Env().Null();
} // namespace node_rfc

////////////////////////////////////////////////////////////////////////////////
// WRAP FUNCTIONS (from RFC)
////////////////////////////////////////////////////////////////////////////////

Napi::Value wrapResult(RFC_FUNCTION_DESC_HANDLE functionDescHandle, RFC_FUNCTION_HANDLE functionHandle, bool rstrip)
{
    Napi::EscapableHandleScope scope(__genv);

    RFC_PARAMETER_DESC paramDesc;
    unsigned int paramCount = 0;

    RfcGetParameterCount(functionDescHandle, &paramCount, NULL);
    Napi::Object resultObj = Napi::Object::New(__genv);

    for (unsigned int i = 0; i < paramCount; i++)
    {
        RfcGetParameterDescByIndex(functionDescHandle, i, &paramDesc, NULL);
        Napi::String name = wrapString(paramDesc.name).As<Napi::String>();
        Napi::Value value = wrapVariable(paramDesc.type, functionHandle, paramDesc.name, paramDesc.nucLength, paramDesc.typeDescHandle, rstrip);
        (resultObj).Set(name, value);
    }
    return scope.Escape(resultObj);
}

Napi::Value wrapString(SAP_UC *uc, int length, bool rstrip)
{
    RFC_RC rc;
    RFC_ERROR_INFO errorInfo;

    Napi::EscapableHandleScope scope(__genv);

    if (length == -1)
    {
        length = strlenU(uc);
    }
    if (length == 0)
    {
        return Napi::String::New(__genv, "");
    }
    unsigned int utf8Size = length * 3;
    char *utf8 = (char *)malloc(utf8Size + 1);
    utf8[0] = '\0';
    unsigned int resultLen = 0;
    rc = RfcSAPUCToUTF8(uc, length, (RFC_BYTE *)utf8, &utf8Size, &resultLen, &errorInfo);

    if (rc != RFC_OK)
    {
        free((char *)utf8);
        Napi::Error::Fatal("wrapString", "node-rfc internal error");
    }

    if (rstrip && strlen(utf8))
    {
        int i = strlen(utf8) - 1;

        while (i >= 0 && isspace(utf8[i]))
        {
            i--;
        }
        utf8[i + 1] = '\0';
    }

    Napi::Value resultValue = Napi::String::New(__genv, utf8);
    free((char *)utf8);
    return scope.Escape(resultValue);
}

Napi::Value wrapStructure(RFC_TYPE_DESC_HANDLE typeDesc, RFC_STRUCTURE_HANDLE structHandle, bool rstrip)
{
    Napi::EscapableHandleScope scope(__genv);

    RFC_RC rc;
    RFC_ERROR_INFO errorInfo;
    RFC_FIELD_DESC fieldDesc;
    unsigned int fieldCount;
    rc = RfcGetFieldCount(typeDesc, &fieldCount, &errorInfo);
    if (rc != RFC_OK)
    {
        Napi::Error::New(__genv, wrapError(&errorInfo).ToString()).ThrowAsJavaScriptException();
    }

    Napi::Object resultObj = Napi::Object::New(__genv);

    for (unsigned int i = 0; i < fieldCount; i++)
    {
        rc = RfcGetFieldDescByIndex(typeDesc, i, &fieldDesc, &errorInfo);
        if (rc != RFC_OK)
        {
            Napi::Error::New(__genv, wrapError(&errorInfo).ToString()).ThrowAsJavaScriptException();
        }
        (resultObj).Set(wrapString(fieldDesc.name), wrapVariable(fieldDesc.type, structHandle, fieldDesc.name, fieldDesc.nucLength, fieldDesc.typeDescHandle, rstrip));
    }

    return scope.Escape(resultObj);
}

Napi::Value wrapVariable(RFCTYPE typ, RFC_FUNCTION_HANDLE functionHandle, SAP_UC *cName, unsigned int cLen, RFC_TYPE_DESC_HANDLE typeDesc, bool rstrip)
{
    Napi::EscapableHandleScope scope(__genv);

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
        resultValue = wrapStructure(typeDesc, structHandle, rstrip);
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
        //RFC_FIELD_DESC fieldDesc;
        unsigned int rowCount;
        rc = RfcGetRowCount(tableHandle, &rowCount, &errorInfo);
        Napi::Array table = Napi::Array::New(__genv);

        for (unsigned int i = 0; i < rowCount; i++)
        {
            RfcMoveTo(tableHandle, i, NULL);
            structHandle = RfcGetCurrentRow(tableHandle, NULL);
            Napi::Value row = wrapStructure(typeDesc, structHandle, rstrip);
            (table).Set(i, row);
        }

        resultValue = table;
        break;
    }
    case RFCTYPE_CHAR:
    {
        RFC_CHAR *charValue = mallocU(cLen);
        rc = RfcGetChars(functionHandle, cName, charValue, cLen, &errorInfo);
        if (rc != RFC_OK)
        {
            break;
        }
        resultValue = wrapString(charValue, cLen, rstrip);
        free(charValue);
        break;
    }
    case RFCTYPE_STRING:
    {
        unsigned int resultLen = 0, strLen = 0;
        RfcGetStringLength(functionHandle, cName, &strLen, &errorInfo);
        SAP_UC *stringValue = mallocU(strLen + 1);
        rc = RfcGetString(functionHandle, cName, stringValue, strLen + 1, &resultLen, &errorInfo);
        if (rc != RFC_OK)
        {
            break;
        }
        resultValue = wrapString(stringValue);
        free(stringValue);
        break;
    }
    case RFCTYPE_NUM:
    {
        RFC_NUM *numValue = mallocU(cLen);
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
        resultValue = Napi::Buffer<char>::New(__genv, reinterpret_cast<char *>(byteValue), cLen); // as a buffer
        //resultValue = Napi::String::New(env, reinterpret_cast<const char *>(byteValue)); // or as a string
        // do not free byteValue - it will be freed when the buffer is garbage collected
        break;
    }
    case RFCTYPE_XSTRING:
    {
        SAP_RAW *byteValue;
        unsigned int strLen, resultLen;
        rc = RfcGetStringLength(functionHandle, cName, &strLen, &errorInfo);
        byteValue = (SAP_RAW *)malloc(strLen + 1);
        byteValue[strLen] = '\0';
        rc = RfcGetXString(functionHandle, cName, byteValue, strLen, &resultLen, &errorInfo);
        if (rc != RFC_OK)
        {
            free(byteValue);
            break;
        }
        resultValue = Napi::Buffer<char>::New(__genv, reinterpret_cast<char *>(byteValue), resultLen); // as a buffer
        //resultValue = Napi::String::New(env, reinterpret_cast<const char *>(byteValue)); // or as a string
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
        unsigned int resultLen;
        unsigned int strLen = 2 * cLen + 1;
        SAP_UC *sapuc = mallocU(strLen + 1);
        rc = RfcGetString(functionHandle, cName, sapuc, strLen + 1, &resultLen, &errorInfo);
        if (rc != RFC_OK)
        {
            free(sapuc);
            break;
        }
        resultValue = wrapString(sapuc, resultLen);
        free(sapuc);
        break;
    }
    case RFCTYPE_FLOAT:
    {
        RFC_FLOAT floatValue;
        rc = RfcGetFloat(functionHandle, cName, &floatValue, &errorInfo);
        resultValue = Napi::Number::New(__genv, floatValue);
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
        resultValue = Napi::Number::New(__genv, intValue);
        break;
    }
    case RFCTYPE_INT1:
    {
        RFC_INT1 int1Value;
        rc = RfcGetInt1(functionHandle, cName, &int1Value, &errorInfo);
        if (rc != RFC_OK)
        {
            break;
        }
        resultValue = Napi::Number::New(__genv, int1Value);
        break;
    }
    case RFCTYPE_INT2:
    {
        RFC_INT2 int2Value;
        rc = RfcGetInt2(functionHandle, cName, &int2Value, &errorInfo);
        if (rc != RFC_OK)
        {
            break;
        }
        resultValue = Napi::Number::New(__genv, int2Value);
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
        break;
    }
    default:
        char err[256];
        std::string fieldName = wrapString(cName).ToString().Utf8Value();
        sprintf(err, "Unknown RFC type %d when wrapping %s", typ, &fieldName[0]);
        Napi::TypeError::New(__genv, err).ThrowAsJavaScriptException();

        break;
    }
    if (rc != RFC_OK)
    {
        return scope.Escape(wrapError(&errorInfo));
    }

    return scope.Escape(resultValue);
}

} // namespace node_rfc
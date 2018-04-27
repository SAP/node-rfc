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

using namespace v8;

////////////////////////////////////////////////////////////////////////////////
// FILL FUNCTIONS (to RFC)
////////////////////////////////////////////////////////////////////////////////

std::string convertToString(Handle<Value> const &str) {
    static const std::string emptyString;

    v8::String::Utf8Value utf8String(str);
    const char *s = *utf8String;
    if (s != NULL) {
        return std::string(s, utf8String.length());
    }

    return emptyString;
}

SAP_UC* fillString(const Handle<Value> &str) {
    RFC_RC rc;
    RFC_ERROR_INFO errorInfo;
    SAP_UC *sapuc;
    unsigned int sapucSize, resultLen = 0;

    std::string s = convertToString(str);
    const char *cStr = s.c_str();

    sapucSize = strlen(cStr) + 1;
    sapuc = mallocU(sapucSize);
    memsetU(sapuc, 0, sapucSize);
    rc = RfcUTF8ToSAPUC((RFC_BYTE*)cStr, strlen(cStr), sapuc, &sapucSize, &resultLen, &errorInfo);

    if (rc != RFC_OK) {
        printf("Unhandled Error");  // FIXME: error handling
    }

    return sapuc;
}

Local<Value> fillVariable(RFCTYPE typ, RFC_FUNCTION_HANDLE functionHandle, SAP_UC* cName, Handle<Value> value, RFC_TYPE_DESC_HANDLE functionDescHandle) {
    Isolate* isolate = Isolate::GetCurrent();
    EscapableHandleScope scope(isolate);
    RFC_RC rc = RFC_OK;
    RFC_ERROR_INFO errorInfo;
    RFC_STRUCTURE_HANDLE structHandle;
    RFC_TABLE_HANDLE tableHandle;
    SAP_UC *cValue;

    switch (typ) {
    case RFCTYPE_STRUCTURE: {
        rc = RfcGetStructure(functionHandle, cName, &structHandle, &errorInfo);
        if (rc != RFC_OK) {
            break;
        }
        Local<Object> structObj = value->ToObject();
        Local<Array> structNames = structObj->GetPropertyNames();
        unsigned int structSize = structNames->Length();

        RFC_FIELD_DESC fieldDesc;
        for (unsigned int i = 0; i < structSize; i++) {
            Local<Value> name = structNames->Get(i);
            Local<Value> value = structObj->Get(name->ToString());

            cValue = fillString(name);  // cValue = cName
            rc = RfcGetFieldDescByName(functionDescHandle, cValue, &fieldDesc, &errorInfo);
            free(cValue);
            if (rc != RFC_OK) {
                return scope.Escape(wrapError(&errorInfo));
            }
            Local<Value> fillError = fillVariable(fieldDesc.type, structHandle, fieldDesc.name, value, fieldDesc.typeDescHandle);
            if (fillError != Nan::Null()) {
                return scope.Escape(fillError);
            }
        }
        break;
    }
    case RFCTYPE_TABLE: {
        rc = RfcGetTable(functionHandle, cName, &tableHandle, &errorInfo);

        if (rc != RFC_OK) {
            break;
        }
        Local<Array> array = value.As<Array>();
        unsigned int rowCount = array->Length();

        for (unsigned int i=0; i < rowCount; i++) {
            structHandle = RfcAppendNewRow(tableHandle, &errorInfo);
            if (structHandle == NULL) {
                rc = RFC_INVALID_HANDLE;
                break;
            }

            // FIXME: DRY from RFCTYPE_STRUCTURE!
            Local<Object> structObj = Nan::CloneElementAt(array, i).ToLocalChecked();
            Local<Array> structNames = structObj->GetPropertyNames();
            unsigned int structSize = structNames->Length();

            RFC_FIELD_DESC fieldDesc;
            for (unsigned int i = 0; i < structSize; i++) {
                Local<Value> name = structNames->Get(i);
                Local<Value> value = structObj->Get(name->ToString());

                cValue = fillString(name);  // cValue = cName
                rc = RfcGetFieldDescByName(functionDescHandle, cValue, &fieldDesc, &errorInfo);
                free(cValue);
                if (rc != RFC_OK) {
                    return scope.Escape(wrapError(&errorInfo));
                }
                Local<Value> fillError = fillVariable(fieldDesc.type, structHandle, fieldDesc.name, value, fieldDesc.typeDescHandle);
                if (fillError != Nan::Null()) {
                    return scope.Escape(fillError);
                }
            }
            // FIXME: END DRY
        }
        break;
    }
    case RFCTYPE_CHAR:
        if (!value->IsString()) {
            char cBuf[256];
            String::Utf8Value s(wrapString(cName));
            std::string fieldName(*s);
            sprintf(cBuf, "Char expected when filling field %s of type %d", fieldName.c_str(), typ);
            Handle<Value> e = Nan::Error(cBuf);
            return scope.Escape(e->ToObject());
        }
        cValue = fillString(value);
        rc = RfcSetChars(functionHandle, cName, cValue, strlenU(cValue), &errorInfo);
        free(cValue);
        break;
    case RFCTYPE_BYTE: {
        v8::String::Utf8Value asciiValue(value->ToString());
        unsigned int size = asciiValue.length();
        SAP_RAW* byteValue = (SAP_RAW*)malloc(size);
        memcpy(byteValue, reinterpret_cast<SAP_RAW*>(*asciiValue), size);
        rc = RfcSetBytes(functionHandle, cName, byteValue, sizeof(byteValue), &errorInfo);
        free(byteValue);
        break;
    }
    case RFCTYPE_XSTRING: {
        v8::String::Utf8Value asciiValue(value->ToString());
        unsigned int size = asciiValue.length();
        SAP_RAW* byteValue = (SAP_RAW*)malloc(size);
        memcpy(byteValue, reinterpret_cast<SAP_RAW*>(*asciiValue), size);
        rc = RfcSetXString(functionHandle, cName, byteValue, sizeof(byteValue), &errorInfo);
        free(byteValue);
        break;
    }
    case RFCTYPE_STRING:
        if (!value->IsString()) {
            char cBuf[256];
            String::Utf8Value s(wrapString(cName));
            std::string fieldName(*s);
            sprintf(cBuf, "Char expected when filling field %s of type %d", fieldName.c_str(), typ);
            Handle<Value> e = Nan::Error(cBuf);
            return scope.Escape(e->ToObject());
        }
        cValue = fillString(value);
        rc = RfcSetString(functionHandle, cName, cValue, strlenU(cValue), &errorInfo);
        free(cValue);
        break;
    case RFCTYPE_NUM:
        if (!value->IsString()) {
            char cBuf[256];
            String::Utf8Value s(wrapString(cName));
            std::string fieldName(*s);
            sprintf(cBuf, "Char expected when filling field %s of type %d", fieldName.c_str(), typ);
            Handle<Value> e = Nan::Error(cBuf);
            return scope.Escape(e->ToObject());
        }
        cValue = fillString(value);
        rc = RfcSetNum(functionHandle, cName, cValue, strlenU(cValue), &errorInfo);
        free(cValue);
        break;
    case RFCTYPE_BCD:   // fallthrough
    case RFCTYPE_FLOAT:
        if (!value->IsNumber() && !value->IsObject() && !value->IsString()) {
            char cBuf[256];
            String::Utf8Value s(wrapString(cName));
            std::string fieldName(*s);
            sprintf(cBuf, "Number, number object or string expected when filling field %s of type %d", fieldName.c_str(), typ);
            Handle<Value> e = Nan::Error(cBuf);
            return scope.Escape(e->ToObject());
        }
        cValue = fillString(value->ToString());
        rc = RfcSetString(functionHandle, cName, cValue, strlenU(cValue), &errorInfo);
        //rc = RfcSetFloat(functionHandle, cName, value.As<Number>()->Value(), &errorInfo);
        break;
    case RFCTYPE_INT:
    case RFCTYPE_INT1:
    case RFCTYPE_INT2:
        if (!value->IsInt32()) {
            char cBuf[256];
            String::Utf8Value s(wrapString(cName));
            std::string fieldName(*s);
            sprintf(cBuf, "Number expected when filling field %s of type %d", fieldName.c_str(), typ);
            Handle<Value> e = Nan::Error(cBuf);
            return scope.Escape(e->ToObject());
        }
        rc = RfcSetInt(functionHandle, cName, value.As<Integer>()->Value(), &errorInfo);
        break;
    case RFCTYPE_DATE:
        if (!value->IsString()) {
            char cBuf[256];
            String::Utf8Value s(wrapString(cName));
            std::string fieldName(*s);
            sprintf(cBuf, "Char expected when filling field %s of type %d", fieldName.c_str(), typ);
            Handle<Value> e = Nan::Error(cBuf);
            return scope.Escape(e->ToObject());
        }
        //cValue = fillString(value.strftime('%Y%m%d'));
        cValue = fillString(value);
        rc = RfcSetDate(functionHandle, cName, cValue, &errorInfo);
        free(cValue);
        break;
    case RFCTYPE_TIME:
        if (!value->IsString()) {
            char cBuf[256];
            String::Utf8Value s(wrapString(cName));
            std::string fieldName(*s);
            sprintf(cBuf, "Char expected when filling field %s of type %d", fieldName.c_str(), typ);
            Handle<Value> e = Nan::Error(cBuf);
            return scope.Escape(e->ToObject());
        }
        //cValue = fillString(value.strftime('%H%M%S'))
        cValue = fillString(value);
        rc = RfcSetTime(functionHandle, cName, cValue, &errorInfo);
        free(cValue);
        break;
    default:
        char cBuf[256];
        String::Utf8Value s(wrapString(cName));
        std::string fieldName(*s);
        sprintf(cBuf, "Unknown RFC type %d when filling %s", typ, fieldName.c_str());
        Handle<Value> e = Nan::Error(cBuf);
        return scope.Escape(e->ToObject());
        break;
    }
    if (rc != RFC_OK) {
        return scope.Escape(wrapError(&errorInfo));
    }
    return Nan::Null();
}

Handle<Value> fillFunctionParameter(RFC_FUNCTION_DESC_HANDLE functionDescHandle, RFC_FUNCTION_HANDLE functionHandle, Local<Value> name, Local<Value> value) {
    Nan::EscapableHandleScope scope;
    RFC_RC rc;
    RFC_ERROR_INFO errorInfo;
    RFC_PARAMETER_DESC paramDesc;
    SAP_UC *cName = fillString(name);
    rc = RfcGetParameterDescByName(functionDescHandle, cName, &paramDesc, &errorInfo);
    free(cName);
    if (rc != RFC_OK) {
        // invalid parameter name
        return scope.Escape(wrapError(&errorInfo));
    }
    return scope.Escape(fillVariable(paramDesc.type, functionHandle, paramDesc.name, value, paramDesc.typeDescHandle));
}


////////////////////////////////////////////////////////////////////////////////
// WRAP FUNCTIONS (from RFC)
////////////////////////////////////////////////////////////////////////////////

Handle<Object> wrapResult(RFC_FUNCTION_DESC_HANDLE functionDescHandle, RFC_FUNCTION_HANDLE functionHandle, bool rstrip) {
    Nan::EscapableHandleScope scope;

    RFC_PARAMETER_DESC paramDesc;
    unsigned int paramCount = 0;

    RfcGetParameterCount(functionDescHandle, &paramCount, NULL);
    Local<Object> resultObj = Nan::New<Object>();

    for (unsigned int i = 0; i < paramCount; i++) {
        RfcGetParameterDescByIndex(functionDescHandle, i, &paramDesc, NULL);
        Nan::Set(resultObj, wrapString(paramDesc.name), wrapVariable(paramDesc.type, functionHandle, paramDesc.name, paramDesc.nucLength, paramDesc.typeDescHandle, rstrip));
    }

    return scope.Escape(resultObj);
}

Handle<Value> wrapString(SAP_UC* uc, int length, bool rstrip) {
    Nan::EscapableHandleScope scope;

    RFC_RC rc;
    RFC_ERROR_INFO errorInfo;

    if (length == -1) {
        length = strlenU(uc);
    }
    if (length == 0) {
        return scope.Escape(Nan::New("").ToLocalChecked());
    }
    unsigned int utf8Size = length * 2;
    char *utf8 = (char*) malloc(utf8Size + 1);
    utf8[0] = '\0';

    unsigned int resultLen = 0;
    rc = RfcSAPUCToUTF8(uc, length, (RFC_BYTE*) utf8, &utf8Size, &resultLen, &errorInfo);

    if (rc != RFC_OK) {
        free((void *)utf8);
        return scope.Escape(Nan::New("node-rfc internal error: wrapString").ToLocalChecked());
    }

    if (rstrip && strlen(utf8)) {
        int i = strlen(utf8)-1;

        while (i >= 0 && isspace(utf8[i])) {
            i--;
        }
        utf8[i+1] = '\0';
    }

    Local<Value> resultValue = Nan::New(utf8).ToLocalChecked();
    free((void *)utf8);
    return scope.Escape(resultValue);
}

Handle<Value> wrapStructure(RFC_TYPE_DESC_HANDLE typeDesc, RFC_STRUCTURE_HANDLE structHandle, bool rstrip) {
    Nan::EscapableHandleScope scope;

    RFC_RC rc;
    RFC_ERROR_INFO errorInfo;
    RFC_FIELD_DESC fieldDesc;
    unsigned int fieldCount;
    rc = RfcGetFieldCount(typeDesc, &fieldCount, &errorInfo);
    if (rc != RFC_OK) {
      // FIXME error handling
      printf("Unhandled Error");
    }

    Local<Object> resultObj = Nan::New<Object>();

    for (unsigned int i = 0; i < fieldCount; i++) {
        rc = RfcGetFieldDescByIndex(typeDesc, i, &fieldDesc, &errorInfo);
        if (rc != RFC_OK) {
          // FIXME error handling
          printf("Unhandled Error");
        }
        Nan::Set(resultObj, wrapString(fieldDesc.name), wrapVariable(fieldDesc.type, structHandle, fieldDesc.name, fieldDesc.nucLength, fieldDesc.typeDescHandle, rstrip));
    }

    return scope.Escape(resultObj);
}

Handle<Value> wrapVariable(RFCTYPE typ, RFC_FUNCTION_HANDLE functionHandle, SAP_UC* cName, unsigned int cLen, RFC_TYPE_DESC_HANDLE typeDesc, bool rstrip) {
    Nan::EscapableHandleScope scope;

    Local<Value> resultValue;

    RFC_RC rc = RFC_OK;
    RFC_ERROR_INFO errorInfo;
    RFC_STRUCTURE_HANDLE structHandle;

    switch(typ) {
        case RFCTYPE_STRUCTURE: {
            rc = RfcGetStructure(functionHandle, cName, &structHandle, &errorInfo);
            if (rc != RFC_OK) {
                break;
            }
            resultValue = wrapStructure(typeDesc, structHandle, rstrip);
            break;
        }
        case RFCTYPE_TABLE: {
            RFC_TABLE_HANDLE tableHandle;
            rc = RfcGetTable(functionHandle, cName, &tableHandle, &errorInfo);
            if (rc != RFC_OK) {
                break;
            }
            //RFC_FIELD_DESC fieldDesc;
            unsigned int rowCount;
            rc = RfcGetRowCount(tableHandle, &rowCount, &errorInfo);
            Handle<Array> table = Nan::New<Array>();

            for (unsigned int i=0; i < rowCount; i++) {
                RfcMoveTo(tableHandle, i, NULL);
                structHandle = RfcGetCurrentRow(tableHandle, NULL);
                Handle<Value> row = wrapStructure(typeDesc, structHandle, rstrip);
                Nan::Set(table, Nan::New(i), row);
            }

            resultValue = table;
            break;
        }
        case RFCTYPE_CHAR: {
            RFC_CHAR* charValue = mallocU(cLen);
            rc = RfcGetChars(functionHandle, cName, charValue, cLen, &errorInfo);
            if (rc != RFC_OK) {
                break;
            }
            resultValue = wrapString(charValue, cLen, rstrip);
            free(charValue);
            break;
        }
        case RFCTYPE_STRING: {
            unsigned int resultLen = 0, strLen = 0;
            RfcGetStringLength(functionHandle, cName, &strLen, &errorInfo);
            SAP_UC* stringValue = mallocU(strLen+1);
            rc = RfcGetString(functionHandle, cName, stringValue, strLen+1, &resultLen, &errorInfo);
            if (rc != RFC_OK) {
                break;
            }
            resultValue = wrapString(stringValue);
            free(stringValue);
            break;
        }
        case RFCTYPE_NUM: {
            RFC_NUM* numValue = mallocU(cLen);
            rc = RfcGetNum(functionHandle, cName, numValue, cLen, &errorInfo);
            if (rc != RFC_OK) {
                free(numValue);
                break;
            }
            resultValue = wrapString(numValue, cLen);
            free(numValue);
            break;
        }
        case RFCTYPE_BYTE: {
            SAP_RAW* byteValue = (SAP_RAW*) malloc(cLen);
            rc = RfcGetBytes(functionHandle, cName, byteValue, cLen, &errorInfo);
            if (rc != RFC_OK) {
                free(byteValue);
                break;
            }
            resultValue = Nan::NewBuffer(reinterpret_cast<char*>(byteValue), cLen).ToLocalChecked();
            // do not free byteValue - it will be freed when the buffer is garbage collected
            break;
        }
        case RFCTYPE_XSTRING: {
            SAP_RAW* byteValue;
            unsigned int strLen, resultLen;
            rc = RfcGetStringLength(functionHandle, cName, &strLen, &errorInfo);
            byteValue = (SAP_RAW*) malloc(strLen+1);
            byteValue[strLen] = '\0';
            rc = RfcGetXString(functionHandle, cName, byteValue, strLen, &resultLen, &errorInfo);
            if (rc != RFC_OK) {
                free(byteValue);
                break;
            }
            resultValue = Nan::New(reinterpret_cast<const char*>(byteValue)).ToLocalChecked();
            free(byteValue);
            break;
        }
        case RFCTYPE_BCD: {
            // An upper bound for the length of the _string representation_
            // of the BCD is given by (2*cLen)-1 (each digit is encoded in 4bit,
            // the first 4 bit are reserved for the sign)
            // Furthermore, a sign char, a decimal separator char may be present 
            // => (2*cLen)+1
            unsigned int resultLen;
            unsigned int strLen = 2 * cLen + 1;
            SAP_UC *sapuc = mallocU(strLen+1);
            rc = RfcGetString(functionHandle, cName, sapuc, strLen+1, &resultLen, &errorInfo);
            if (rc != RFC_OK) {
                free(sapuc);
                break;
            }
            resultValue = wrapString(sapuc, resultLen);
            free(sapuc);
            break;
        }   
        case RFCTYPE_FLOAT: {
            RFC_FLOAT floatValue;
            rc = RfcGetFloat(functionHandle, cName, &floatValue, &errorInfo);
            resultValue = Nan::New(floatValue);
            break;
        }
        case RFCTYPE_INT: {
            RFC_INT intValue;
            rc = RfcGetInt(functionHandle, cName, &intValue, &errorInfo);
            if (rc != RFC_OK) {
                break;
            }
            resultValue = Nan::New(intValue);
            break;
        }
        case RFCTYPE_INT1: {
            RFC_INT1 int1Value;
            rc = RfcGetInt1(functionHandle, cName, &int1Value, &errorInfo);
            if (rc != RFC_OK) {
                break;
            }
            resultValue = Nan::New(int1Value);
            break;
        }
        case RFCTYPE_INT2: {
            RFC_INT2 int2Value;
            rc = RfcGetInt2(functionHandle, cName, &int2Value, &errorInfo);
            if (rc != RFC_OK) {
                break;
            }
            resultValue = Nan::New(int2Value);
            break;
        }
        case RFCTYPE_DATE: {
            RFC_DATE dateValue;
            rc = RfcGetDate(functionHandle, cName, dateValue, &errorInfo);
            if (rc != RFC_OK) {
                break;
            }
            resultValue = wrapString(dateValue, 8);
            break;
        }
        case RFCTYPE_TIME: {
            RFC_TIME timeValue;
            rc = RfcGetTime(functionHandle, cName, timeValue, &errorInfo);
            if (rc != RFC_OK) {
                break;
            }
            resultValue = wrapString(timeValue, 6); // FIXME: use v8::Date object
            break;
        }
        default:
            resultValue = Nan::New("FIXME UNKNOWN TYPE").ToLocalChecked();
            char cBuf[256];
            String::Utf8Value s(wrapString(cName));
            std::string fieldName(*s);
            sprintf(cBuf, "Unknown RFC type %d when wrapping %s", typ, fieldName.c_str());
            Handle<Value> e = Nan::Error(cBuf);
            return scope.Escape(e->ToObject());
            break;
    }
    if (rc != RFC_OK) {
        return scope.Escape(wrapError(&errorInfo));
    }

    return scope.Escape(resultValue);
}

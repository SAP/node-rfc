// SPDX-FileCopyrightText: 2014 SAP SE Srdjan Boskovic <srdjan.boskovic@sap.com>
//
// SPDX-License-Identifier: Apache-2.0
// language governing permissions and limitations under the License.

#include "nwrfcsdk.h"

namespace node_rfc {

////////////////////////////////////////////////////////////////////////////////
// Set Parameters (to SDK)
////////////////////////////////////////////////////////////////////////////////

SAP_UC* setString(const Napi::String napistr) {
  RFC_RC rc;
  RFC_ERROR_INFO errorInfo;
  uint_t sapucSize, resultLen = 0;

  std::string sstr = std::string(napistr);
  // std::string str = napistr.Utf8Value();
  sapucSize = sstr.length() + 1;

  SAP_UC* sapuc = new SAP_UC[sapucSize];
  memsetU(sapuc, 0, sapucSize);
  rc = RfcUTF8ToSAPUC((RFC_BYTE*)&sstr[0],
                      sapucSize - 1,
                      sapuc,
                      &sapucSize,
                      &resultLen,
                      &errorInfo);

  if (rc != RFC_OK) {
    delete[] sapuc;
    _log.fatal(
        logClass::nwrfc,
        "NodeJS string could not be parsed to ABAP string: '" + sstr + "'",
        "\nsapucSize: ",
        sapucSize,
        " resultLen ",
        resultLen,
        " Error group: ",
        errorInfo.group,
        " code: ",
        errorInfo.code);
    Napi::Error::Fatal("setString",
                       "NodeJS string could not be parsed to ABAP string");
  }
  return sapuc;
}

SAP_UC* setString(std::string sstr) {
  RFC_RC rc;
  RFC_ERROR_INFO errorInfo;
  uint_t sapucSize, resultLen = 0;
  sapucSize = sstr.length() + 1;

  SAP_UC* sapuc = new SAP_UC[sapucSize];
  memsetU(sapuc, 0, sapucSize);
  rc = RfcUTF8ToSAPUC((RFC_BYTE*)&sstr[0],
                      sapucSize - 1,
                      sapuc,
                      &sapucSize,
                      &resultLen,
                      &errorInfo);

  if (rc != RFC_OK) {
    delete[] sapuc;
    _log.fatal(
        logClass::nwrfc,
        "NodeJS string could not be parsed to ABAP string: '" + sstr + "'",
        "\nsapucSize: ",
        sapucSize,
        " resultLen ",
        resultLen,
        " Error group: ",
        errorInfo.group,
        " code: ",
        errorInfo.code);
    Napi::Error::Fatal("setString",
                       "NodeJS string could not be parsed to ABAP string");
  }
  return sapuc;
}

Napi::Value setRfmParameter(RFC_FUNCTION_DESC_HANDLE functionDescHandle,
                            RFC_FUNCTION_HANDLE functionHandle,
                            Napi::String name,
                            Napi::Value value,
                            RfmErrorPath* errorPath,
                            ClientOptionsStruct* client_options) {
  Napi::EscapableHandleScope scope(value.Env());

  RFC_RC rc;
  RFC_ERROR_INFO errorInfo;
  RFC_PARAMETER_DESC paramDesc;
  SAP_UC* cName = setString(name);
  errorPath->setParameterName(cName);
  rc = RfcGetParameterDescByName(
      functionDescHandle, cName, &paramDesc, &errorInfo);
  delete[] cName;
  if (rc != RFC_OK) {
    return scope.Escape(rfcSdkError(&errorInfo, errorPath));
  }
  return scope.Escape(setVariable(paramDesc.type,
                                  functionHandle,
                                  paramDesc.name,
                                  value,
                                  paramDesc.typeDescHandle,
                                  errorPath,
                                  client_options));
}

Napi::Value setStructure(RFC_STRUCTURE_HANDLE structHandle,
                         RFC_TYPE_DESC_HANDLE functionDescHandle,
                         Napi::Value value,
                         RfmErrorPath* errorPath,
                         ClientOptionsStruct* client_options) {
  RFC_RC rc;
  RFC_ERROR_INFO errorInfo;

  Napi::EscapableHandleScope scope(value.Env());

  Napi::Object structObj = value.ToObject();
  Napi::Array structNames = structObj.GetPropertyNames();
  uint_t structSize = structNames.Length();

  RFC_FIELD_DESC fieldDesc;

  Napi::Value retVal = value.Env().Undefined();

  for (uint_t i = 0; i < structSize; i++) {
    Napi::String name = structNames.Get(i).ToString();
    Napi::Value _value = structObj.Get(name);

    SAP_UC* cValue = setString(name);
    rc = RfcGetFieldDescByName(
        functionDescHandle, cValue, &fieldDesc, &errorInfo);
    delete[] cValue;
    if (rc != RFC_OK) {
      errorPath->setFieldName(fieldDesc.name);
      retVal = rfcSdkError(&errorInfo, errorPath);
      break;
    }
    retVal = setVariable(fieldDesc.type,
                         structHandle,
                         fieldDesc.name,
                         _value,
                         fieldDesc.typeDescHandle,
                         errorPath,
                         client_options);
    if (!retVal.IsUndefined()) {
      break;
    }
  }
  return retVal;
}

Napi::Value setVariable(RFCTYPE typ,
                        RFC_FUNCTION_HANDLE functionHandle,
                        SAP_UC* cName,
                        Napi::Value value,
                        RFC_TYPE_DESC_HANDLE functionDescHandle,
                        RfmErrorPath* errorPath,
                        ClientOptionsStruct* client_options) {
  Napi::EscapableHandleScope scope(value.Env());
  RFC_RC rc = RFC_OK;
  RFC_ERROR_INFO errorInfo;
  SAP_UC* cValue;

  errorPath->setName(typ, cName);

  switch (typ) {
    case RFCTYPE_STRUCTURE: {
      RFC_STRUCTURE_HANDLE structHandle;
      rc = RfcGetStructure(functionHandle, cName, &structHandle, &errorInfo);
      if (rc != RFC_OK) {
        return scope.Escape(rfcSdkError(&errorInfo, errorPath));
      }
      Napi::Value rv = setStructure(
          structHandle, functionDescHandle, value, errorPath, client_options);
      if (!rv.IsUndefined()) {
        return scope.Escape(rv);
      }
      break;
    }
    case RFCTYPE_TABLE: {
      RFC_TABLE_HANDLE tableHandle;
      rc = RfcGetTable(functionHandle, cName, &tableHandle, &errorInfo);

      if (rc != RFC_OK) {
        break;
      }
      if (!value.IsArray()) {
        return nodeRfcError(
            "Array expected from NodeJS, for ABAP RFM table of type " +
                std::to_string(typ),
            errorPath);
      }
      Napi::Array array = value.As<Napi::Array>();
      uint_t rowCount = array.Length();

      for (uint_t i = 0; i < rowCount; i++) {
        errorPath->table_line = i;
        RFC_STRUCTURE_HANDLE structHandle =
            RfcAppendNewRow(tableHandle, &errorInfo);
        Napi::Value line = array.Get(i);
        if (line.IsBuffer() || line.IsString() || line.IsNumber()) {
          Napi::Object lineObj = Napi::Object::New(value.Env());
          lineObj.Set(Napi::String::New(value.Env(), ""), line);
          line = lineObj;
        }
        Napi::Value rv = setStructure(
            structHandle, functionDescHandle, line, errorPath, client_options);
        if (!rv.IsUndefined()) {
          return scope.Escape(rv);
        }
      }
      break;
    }
    case RFCTYPE_BYTE: {
      if (!value.IsBuffer()) {
        return nodeRfcError(
            "Buffer expected from NodeJS for ABAP field of type " +
                std::to_string(typ),
            errorPath);
      }

      Napi::Buffer<SAP_RAW> js_buf = value.As<Napi::Buffer<SAP_RAW>>();
      uint_t js_buf_bytelen = js_buf.ByteLength();
      SAP_RAW* byteValue = new SAP_RAW[js_buf_bytelen];
      memcpy(byteValue, js_buf.Data(), js_buf_bytelen);

      // excessive padding bytes sent from NodeJS, are silently trimmed by SDK
      // to ABAP field length
      rc = RfcSetBytes(
          functionHandle, cName, byteValue, js_buf_bytelen, &errorInfo);
      delete[] byteValue;
      break;
    }
    case RFCTYPE_XSTRING: {
      if (!value.IsBuffer()) {
        return nodeRfcError(
            "Buffer expected from NodeJS for ABAP field of type " +
                std::to_string(typ),
            errorPath);
      }

      Napi::Buffer<SAP_RAW> js_buf = value.As<Napi::Buffer<SAP_RAW>>();
      uint_t js_buf_bytelen = js_buf.ByteLength();
      SAP_RAW* byteValue = new SAP_RAW[js_buf_bytelen];
      memcpy(byteValue, js_buf.Data(), js_buf_bytelen);

      rc = RfcSetXString(
          functionHandle, cName, byteValue, js_buf_bytelen, &errorInfo);
      delete[] byteValue;
      break;
    }
    case RFCTYPE_CHAR:
    case RFCTYPE_STRING: {
      if (!value.IsString()) {
        return nodeRfcError(
            "String expected from NodeJS for ABAP field of type " +
                std::to_string(typ),
            errorPath);
      }
      cValue = setString(value.ToString());
      rc = RfcSetString(
          functionHandle, cName, cValue, strlenU(cValue), &errorInfo);
      delete[] cValue;
      break;
    }
    case RFCTYPE_NUM: {
      if (!value.IsString()) {
        return nodeRfcError(
            "Char expected from NodeJS for ABAP field of type " +
                std::to_string(typ),
            errorPath);
      }
      cValue = setString(value.ToString());
      rc =
          RfcSetNum(functionHandle, cName, cValue, strlenU(cValue), &errorInfo);
      delete[] cValue;
      break;
    }
    case RFCTYPE_BCD:  // fallthrough
    case RFCTYPE_DECF16:
    case RFCTYPE_DECF34:
    case RFCTYPE_FLOAT: {
      if (!value.IsNumber() && !value.IsObject() && !value.IsString()) {
        return nodeRfcError("Number, number object or string expected from "
                            "NodeJS for ABAP field of type " +
                                std::to_string(typ),
                            errorPath);
      }
      cValue = setString(value.ToString());
      rc = RfcSetString(
          functionHandle, cName, cValue, strlenU(cValue), &errorInfo);
      delete[] cValue;
      break;
    }
    case RFCTYPE_INT:  // fallthrough
    case RFCTYPE_INT1:
    case RFCTYPE_INT2:
    case RFCTYPE_INT8: {
      if (!value.IsNumber()) {
        return nodeRfcError(
            "Integer number expected from NodeJS for ABAP field of type " +
                std::to_string(typ),
            errorPath);
      }

      // https://github.com/mhdawson/node-sqlite3/pull/3
      double numDouble = value.ToNumber().DoubleValue();
      if ((int64_t)numDouble !=
          numDouble)  // or std::trunc(numDouble) == numDouble;
      {
        return nodeRfcError(
            "Integer number expected from NodeJS for ABAP field of type " +
                std::to_string(typ) + ", got " + value.ToString().Utf8Value(),
            errorPath);
      }
      RFC_INT rfcInt = (RFC_INT)value.As<Napi::Number>().Int64Value();
      // int64_t rfcInt = value.As<Napi::Number>().Int64Value();
      // printf("typ: %d value: %d %u", typ, rfcInt, UINT8_MAX);
      if (typ == RFCTYPE_INT8) {
        rc = RfcSetInt8(functionHandle, cName, rfcInt, &errorInfo);
      } else {
        if ((typ == RFCTYPE_INT1 && rfcInt > UINT8_MAX) ||
            (typ == RFCTYPE_INT2 &&
             ((rfcInt > INT16_MAX) || (rfcInt < INT16_MIN)))) {
          return nodeRfcError(
              "Overflow or other error when putting NodeJS value " +
                  std::to_string(rfcInt) + " into ABAP integer field of type " +
                  std::to_string(typ),
              errorPath);
        }

        rc = RfcSetInt(functionHandle, cName, rfcInt, &errorInfo);
      }
      break;
    }
    case RFCTYPE_UTCLONG: {
      if (!value.IsString()) {
        return nodeRfcError(
            "UTCLONG string expected from NodeJS for ABAP field of type " +
                std::to_string(typ),
            errorPath);
      }
      cValue = setString(value.ToString());
      rc = RfcSetString(
          functionHandle, cName, cValue, strlenU(cValue), &errorInfo);
      delete[] cValue;
      break;
    }
    case RFCTYPE_DATE: {
      if (!client_options->dateToABAP.IsEmpty()) {
        // YYYYMMDD format expected
        value = client_options->dateToABAP.Call({value});
      }
      if (!value.IsString()) {
        return nodeRfcError("Date format YYYYMMDD expected from NodeJS "
                            "for ABAP field of type " +
                                std::to_string(typ),
                            errorPath);
      }
      cValue = setString(value.ToString());
      rc = RfcSetDate(functionHandle, cName, cValue, &errorInfo);
      delete[] cValue;
      break;
    }
    case RFCTYPE_TIME: {
      if (!client_options->timeToABAP.IsEmpty()) {
        // HHMMSS format expected
        value = client_options->timeToABAP.Call({value});
      }
      if (!value.IsString()) {
        return nodeRfcError("Time format HHMMSS expected from NodeJS for "
                            "ABAP field of type " +
                                std::to_string(typ),
                            errorPath);
      }
      cValue = setString(value.ToString());
      rc = RfcSetTime(functionHandle, cName, cValue, &errorInfo);
      delete[] cValue;
      break;
    }
    default: {
      return nodeRfcError("Unknown RFC type from NodeJS " + std::to_string(typ),
                          errorPath);
    }
  }
  if (rc != RFC_OK) {
    return scope.Escape(rfcSdkError(&errorInfo, errorPath));
  }
  return scope.Env().Undefined();
}

////////////////////////////////////////////////////////////////////////////////
// Get Parameters (from SDK)
////////////////////////////////////////////////////////////////////////////////

Napi::Value wrapString(const SAP_UC* uc, int length) {
  RFC_ERROR_INFO errorInfo;

  Napi::EscapableHandleScope scope(node_rfc::__env);

  if (length == -1) {
    length = strlenU(uc);
  }
  if (length == 0) {
    return scope.Escape(Napi::String::New(node_rfc::__env, ""));
  }
  // try with 3 bytes per unicode character
  uint_t utf8Size = length * 3;
  RFC_BYTE* utf8 = new RFC_BYTE[utf8Size + 1];
  utf8[0] = '\0';
  uint_t resultLen = 0;
  RfcSAPUCToUTF8(uc, length, utf8, &utf8Size, &resultLen, &errorInfo);
  _log.record(logClass::nwrfc,
              logLevel::all,
              "wrapString len: ",
              length,
              " utf8Size: ",
              utf8Size,
              " resultLen: ",
              resultLen,
              " Error group:",
              errorInfo.group,
              " code: ",
              errorInfo.code);
  if (errorInfo.code != RFC_OK) {
    // not enough, try with 5
    delete[] utf8;
    utf8Size = length * 5;
    utf8 = new RFC_BYTE[utf8Size + 1];
    utf8[0] = '\0';
    resultLen = 0;
    RfcSAPUCToUTF8(uc, length, utf8, &utf8Size, &resultLen, &errorInfo);
    if (errorInfo.code != RFC_OK) {
      delete[] utf8;
      return node_rfc::__env.Undefined();
    }
  }

  // trim trailing whitespaces
  int i = resultLen - 1;
  while (i >= 0 && isspace(utf8[i])) {
    i--;
  }
  utf8[i + 1] = '\0';
  Napi::Value resultValue = Napi::String::New(
      node_rfc::__env, std::string(reinterpret_cast<char*>(utf8), i + 1));
  delete[] utf8;
  return scope.Escape(resultValue);
}

ValuePair getRfmParameters(RFC_FUNCTION_DESC_HANDLE functionDescHandle,
                           RFC_FUNCTION_HANDLE functionHandle,
                           RfmErrorPath* errorPath,
                           ClientOptionsStruct* client_options) {
  Napi::EscapableHandleScope scope(node_rfc::__env);

  RFC_PARAMETER_DESC paramDesc;

  uint_t paramCount = 0;

  RfcGetParameterCount(functionDescHandle, &paramCount, nullptr);
  Napi::Object resultObj = Napi::Object::New(node_rfc::__env);

  for (uint_t i = 0; i < paramCount; i++) {
    RfcGetParameterDescByIndex(functionDescHandle, i, &paramDesc, nullptr);
    if ((paramDesc.direction & client_options->filter_param_type) == 0) {
      Napi::String name = wrapString(paramDesc.name).As<Napi::String>();
      errorPath->setParameterName(paramDesc.name);
      // DEBUG("param type ", paramDesc.type, " name ",
      // wrapString(paramDesc.name).As<Napi::String>().Utf8Value(), " direction
      // ", paramDesc.direction, " filter ", paramDesc.direction &
      // client_options.filter_param_type);
      ValuePair result = getVariable(paramDesc.type,
                                     functionHandle,
                                     paramDesc.name,
                                     paramDesc.nucLength,
                                     paramDesc.typeDescHandle,
                                     errorPath,
                                     client_options);
      if (!result.first.IsUndefined()) {
        return result;
      }
      (resultObj).Set(name, result.second);
    }
  }
  return ValuePair(ENV_UNDEFINED, scope.Escape(resultObj));
}

ValuePair getStructure(RFC_TYPE_DESC_HANDLE typeDesc,
                       RFC_STRUCTURE_HANDLE structHandle,
                       RfmErrorPath* errorPath,
                       ClientOptionsStruct* client_options) {
  Napi::EscapableHandleScope scope(node_rfc::__env);

  Napi::Object resultObj = Napi::Object::New(node_rfc::__env);

  RFC_RC rc;
  RFC_ERROR_INFO errorInfo;
  RFC_FIELD_DESC fieldDesc;
  uint_t fieldCount;
  rc = RfcGetFieldCount(typeDesc, &fieldCount, &errorInfo);
  if (rc != RFC_OK) {
    return ValuePair(rfcSdkError(&errorInfo, errorPath), ENV_UNDEFINED);
  }

  for (uint_t i = 0; i < fieldCount; i++) {
    rc = RfcGetFieldDescByIndex(typeDesc, i, &fieldDesc, &errorInfo);
    if (rc != RFC_OK) {
      errorPath->setFieldName(fieldDesc.name);
      return ValuePair(rfcSdkError(&errorInfo, errorPath), ENV_UNDEFINED);
    }
    ValuePair result = getVariable(fieldDesc.type,
                                   structHandle,
                                   fieldDesc.name,
                                   fieldDesc.nucLength,
                                   fieldDesc.typeDescHandle,
                                   errorPath,
                                   client_options);
    if (!result.first.IsUndefined()) {
      return result;
    }
    (resultObj).Set(wrapString(fieldDesc.name), result.second);
  }

  if (fieldCount == 1) {
    Napi::String fieldName =
        resultObj.GetPropertyNames().Get((uint_t)0).As<Napi::String>();
    if (fieldName.Utf8Value().size() == 0) {
      return ValuePair(ENV_UNDEFINED, scope.Escape(resultObj.Get(fieldName)));
    }
  }

  return ValuePair(ENV_UNDEFINED, scope.Escape(resultObj));
}

ValuePair getVariable(RFCTYPE typ,
                      RFC_FUNCTION_HANDLE functionHandle,
                      SAP_UC* cName,
                      uint_t cLen,
                      RFC_TYPE_DESC_HANDLE typeDesc,
                      RfmErrorPath* errorPath,
                      ClientOptionsStruct* client_options) {
  Napi::EscapableHandleScope scope(node_rfc::__env);
  Napi::Value resultValue = ENV_UNDEFINED;

  RFC_RC rc = RFC_OK;
  RFC_ERROR_INFO errorInfo;
  RFC_STRUCTURE_HANDLE structHandle;

  errorPath->setName(typ, cName);

  switch (typ) {
    case RFCTYPE_STRUCTURE: {
      rc = RfcGetStructure(functionHandle, cName, &structHandle, &errorInfo);
      if (rc != RFC_OK) {
        break;
      }

      ValuePair result =
          getStructure(typeDesc, structHandle, errorPath, client_options);
      if (!result.first.IsUndefined()) {
        return result;
      }
      resultValue = result.second;
      break;
    }
    case RFCTYPE_TABLE: {
      RFC_TABLE_HANDLE tableHandle;
      rc = RfcGetTable(functionHandle, cName, &tableHandle, &errorInfo);
      if (rc != RFC_OK) {
        break;
      }
      uint_t rowCount;
      rc = RfcGetRowCount(tableHandle, &rowCount, &errorInfo);

      Napi::Array table = Napi::Array::New(node_rfc::__env);

      while (rowCount-- > 0) {
        errorPath->table_line = rowCount;
        RfcMoveTo(tableHandle, rowCount, nullptr);
        ValuePair result =
            getStructure(typeDesc, tableHandle, errorPath, client_options);
        if (!result.first.IsUndefined()) {
          return result;
        }
        RfcDeleteCurrentRow(tableHandle, &errorInfo);
        (table).Set(rowCount, result.second);
      }
      resultValue = table;
      break;
    }
    case RFCTYPE_CHAR: {
      RFC_CHAR* charValue = new RFC_CHAR[cLen];
      rc = RfcGetChars(functionHandle, cName, charValue, cLen, &errorInfo);
      if (rc != RFC_OK) {
        break;
      }
      resultValue = wrapString(charValue, cLen);
      delete[] charValue;
      break;
    }
    case RFCTYPE_STRING: {
      uint_t resultLen = 0, strLen = 0;
      RfcGetStringLength(functionHandle, cName, &strLen, &errorInfo);
      SAP_UC* stringValue = new RFC_CHAR[strLen + 1];
      rc = RfcGetString(functionHandle,
                        cName,
                        stringValue,
                        strLen + 1,
                        &resultLen,
                        &errorInfo);
      if (rc != RFC_OK) {
        break;
      }
      resultValue = wrapString(stringValue, strLen);
      delete[] stringValue;
      break;
    }
    case RFCTYPE_NUM: {
      RFC_NUM* numValue = new RFC_CHAR[cLen];
      rc = RfcGetNum(functionHandle, cName, numValue, cLen, &errorInfo);
      if (rc != RFC_OK) {
        delete[] numValue;
        break;
      }
      resultValue = wrapString(numValue, cLen);
      delete[] numValue;
      break;
    }
    case RFCTYPE_BYTE: {
      SAP_RAW* byteValue = new SAP_RAW[cLen];

      rc = RfcGetBytes(functionHandle, cName, byteValue, cLen, &errorInfo);
      if (rc != RFC_OK) {
        delete[] byteValue;
        break;
      }
      resultValue = Napi::Buffer<SAP_RAW>::New(
          node_rfc::__env, byteValue, cLen, [](void*, SAP_RAW* byteValue) {
            delete[] byteValue;
          });
      break;
    }

    case RFCTYPE_XSTRING: {
      uint_t strLen, resultLen;
      RfcGetStringLength(functionHandle, cName, &strLen, &errorInfo);

      SAP_RAW* byteValue = new SAP_RAW[strLen + 1];
      byteValue[strLen] = '\0';

      rc = RfcGetXString(
          functionHandle, cName, byteValue, strLen, &resultLen, &errorInfo);

      if (rc != RFC_OK) {
        delete[] byteValue;
        break;
      }
      resultValue = Napi::Buffer<SAP_RAW>::New(
          node_rfc::__env, byteValue, resultLen, [](void*, SAP_RAW* byteValue) {
            delete[] byteValue;
          });
      break;
    }
    case RFCTYPE_BCD: {
      // An upper bound for the length of the _string representation_
      // of the BCD is given by (2*cLen)-1 (each digit is encoded in 4bit,
      // the first 4 bit are reserved for the sign)
      // Furthermore, a sign char, a decimal separator char may be present
      // => (2*cLen)+1
      uint_t resultLen;
      uint_t strLen = 2 * cLen + 1;
      SAP_UC* sapuc = new SAP_UC[strLen + 1];
      rc = RfcGetString(
          functionHandle, cName, sapuc, strLen + 1, &resultLen, &errorInfo);
      if (rc == 23)  // Buffer too small, use returned requried result length
      {
        _log.warning(logClass::nwrfc,
                     "Buffer for BCD type ",
                     typ,
                     " too small, for ",
                     errorPath->pathstr());
        delete[] sapuc;
        strLen = resultLen;
        sapuc = new SAP_UC[strLen + 1];
        rc = RfcGetString(
            functionHandle, cName, sapuc, strLen + 1, &resultLen, &errorInfo);
      }
      if (rc != RFC_OK) {
        delete[] sapuc;
        break;
      }
      resultValue = wrapString(sapuc, resultLen).ToString();
      delete[] sapuc;

      if (client_options->bcd == CLIENT_OPTION_BCD_FUNCTION) {
        resultValue = client_options->bcdFunction.Call({resultValue});
      } else if (client_options->bcd == CLIENT_OPTION_BCD_NUMBER) {
        resultValue = resultValue.ToNumber();
      }
      break;
    }
    case RFCTYPE_FLOAT: {
      RFC_FLOAT floatValue;
      rc = RfcGetFloat(functionHandle, cName, &floatValue, &errorInfo);
      resultValue = Napi::Number::New(node_rfc::__env, floatValue);
      break;
    }
    case RFCTYPE_DECF16:
    case RFCTYPE_DECF34: {
      // An upper bound for the length of the _string representation_
      // of the BCD is given by (2*cLen)-1 (each digit is encoded in 4bit,
      // the first 4 bit are reserved for the sign)
      // Furthermore, a sign char, a decimal separator char may be present
      // => (2*cLen)+1
      // and exponent char, sign and exponent
      // => +9
      uint_t resultLen;
      uint_t strLen = 2 * cLen + 10;
      SAP_UC* sapuc = new SAP_UC[strLen + 1];
      rc = RfcGetString(
          functionHandle, cName, sapuc, strLen + 1, &resultLen, &errorInfo);
      if (rc == 23)  // Buffer too small, use returned requried result length
      {
        _log.warning(logClass::nwrfc,
                     "Buffer for BCD type ",
                     typ,
                     " too small, for ",
                     errorPath->pathstr());
        delete[] sapuc;
        strLen = resultLen;
        sapuc = new SAP_UC[strLen + 1];
        rc = RfcGetString(
            functionHandle, cName, sapuc, strLen + 1, &resultLen, &errorInfo);
      }
      if (rc != RFC_OK) {
        delete[] sapuc;
        break;
      }
      resultValue = wrapString(sapuc, resultLen).ToString();
      delete[] sapuc;

      if (client_options->bcd == CLIENT_OPTION_BCD_FUNCTION) {
        resultValue = client_options->bcdFunction.Call({resultValue});
      } else if (client_options->bcd == CLIENT_OPTION_BCD_NUMBER) {
        resultValue = resultValue.ToNumber();
      }
      break;
    }
    case RFCTYPE_INT: {
      RFC_INT intValue;
      rc = RfcGetInt(functionHandle, cName, &intValue, &errorInfo);
      if (rc != RFC_OK) {
        break;
      }
      resultValue = Napi::Number::New(node_rfc::__env, intValue);
      break;
    }
    case RFCTYPE_INT1: {
      RFC_INT1 intValue;
      rc = RfcGetInt1(functionHandle, cName, &intValue, &errorInfo);
      if (rc != RFC_OK) {
        break;
      }
      resultValue = Napi::Number::New(node_rfc::__env, intValue);
      break;
    }
    case RFCTYPE_INT2: {
      RFC_INT2 intValue;
      rc = RfcGetInt2(functionHandle, cName, &intValue, &errorInfo);
      if (rc != RFC_OK) {
        break;
      }
      resultValue = Napi::Number::New(node_rfc::__env, intValue);
      break;
    }
    case RFCTYPE_INT8: {
      RFC_INT8 intValue;
      rc = RfcGetInt8(functionHandle, cName, &intValue, &errorInfo);
      if (rc != RFC_OK) {
        break;
      }
      resultValue = Napi::Number::New(node_rfc::__env, intValue);
      break;
    }
    case RFCTYPE_UTCLONG: {
      uint_t resultLen = 0, strLen = 27;
      SAP_UC* stringValue = new SAP_UC[strLen + 1];
      rc = RfcGetString(functionHandle,
                        cName,
                        stringValue,
                        strLen + 1,
                        &resultLen,
                        &errorInfo);
      if (rc != RFC_OK) {
        break;
      }
      stringValue[19] = '.';
      resultValue = wrapString(stringValue, strLen);
      delete[] stringValue;
      break;
    }
    case RFCTYPE_DATE: {
      RFC_DATE dateValue;
      rc = RfcGetDate(functionHandle, cName, dateValue, &errorInfo);
      if (rc != RFC_OK) {
        break;
      }
      resultValue = wrapString(dateValue, 8);
      if (!client_options->dateFromABAP.IsEmpty()) {
        resultValue = client_options->dateFromABAP.Call({resultValue});
      }
      break;
    }
    case RFCTYPE_TIME: {
      RFC_TIME timeValue;
      rc = RfcGetTime(functionHandle, cName, timeValue, &errorInfo);
      if (rc != RFC_OK) {
        break;
      }
      resultValue = wrapString(timeValue, 6);
      if (!client_options->timeFromABAP.IsEmpty()) {
        resultValue = client_options->timeFromABAP.Call({resultValue});
      }
      break;
    }
    default:
      return ValuePair(
          ENV_UNDEFINED,
          nodeRfcError("RFC type from ABAP not supported" + std::to_string(typ),
                       errorPath));
      break;
  }

  if (rc != RFC_OK) {
    return ValuePair(scope.Escape(rfcSdkError(&errorInfo, errorPath)),
                     ENV_UNDEFINED);
  }

  if (resultValue.IsUndefined()) {
    return ValuePair(
        scope.Escape(nodeRfcError("Non-unicode ABAP string", errorPath)),
        ENV_UNDEFINED);
  }

  return ValuePair(ENV_UNDEFINED, scope.Escape(resultValue));
}

////////////////////////////////////////////////////////////////////////////////
// NODE-RFC ERRORS
////////////////////////////////////////////////////////////////////////////////

Napi::Value nodeRfcError(std::string message, RfmErrorPath* errorPath) {
  Napi::EscapableHandleScope scope(node_rfc::__env);
  Napi::Object errorObj = Napi::Object::New(node_rfc::__env);
  errorObj.Set("name", "nodeRfcError");
  errorObj.Set("message", message);
  if (errorPath != nullptr) {
    errorObj.Set("rfmPath", errorPath->getpath());
  }
  return scope.Escape(errorObj);
}

////////////////////////////////////////////////////////////////////////////////
// NWRFC SDK ERRORS
////////////////////////////////////////////////////////////////////////////////

Napi::Object RfcLibError(RFC_ERROR_INFO* errorInfo) {
  Napi::Object errorObj = Napi::Object::New(node_rfc::__env);
  (errorObj).Set("name", "RfcLibError");
  (errorObj).Set("group", Napi::Number::New(node_rfc::__env, errorInfo->group));
  (errorObj).Set("code", Napi::Number::New(node_rfc::__env, errorInfo->code));
  (errorObj).Set("codeString", wrapString(RfcGetRcAsString(errorInfo->code)));
  (errorObj).Set("key", wrapString(errorInfo->key));
  (errorObj).Set("message", wrapString(errorInfo->message));
  return errorObj;
}

Napi::Object AbapError(RFC_ERROR_INFO* errorInfo) {
  Napi::Object errorObj = Napi::Object::New(node_rfc::__env);
  (errorObj).Set("name", "ABAPError");
  (errorObj).Set("group", Napi::Number::New(node_rfc::__env, errorInfo->group));
  (errorObj).Set("code", Napi::Number::New(node_rfc::__env, errorInfo->code));
  (errorObj).Set("codeString", wrapString(RfcGetRcAsString(errorInfo->code)));
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

Napi::Value rfcSdkError(RFC_ERROR_INFO* errorInfo, RfmErrorPath* errorPath) {
  Napi::EscapableHandleScope scope(node_rfc::__env);
  Napi::Object errorObj;

  switch (errorInfo->group) {
    case OK:  // 0: should never happen
      errorObj = nodeRfcError("rfcSdkError invoked with the RFC error group OK")
                     .As<Napi::Object>();
      break;

    case LOGON_FAILURE:          // 3: Error message raised when logon fails
    case COMMUNICATION_FAILURE:  // 4: Problems with the network connection (or
                                 // backend broke down and killed the
                                 // connection)
    case EXTERNAL_RUNTIME_FAILURE:  // 5: Problems in the RFC runtime of the
                                    // external program (i.e "this" library)
      errorObj = RfcLibError(errorInfo);
      break;

    case ABAP_APPLICATION_FAILURE:  // 1: ABAP Exception raised in ABAP function
                                    // modules
    case ABAP_RUNTIME_FAILURE:      // 2: ABAP Message raised in ABAP function
                                    // modules or in ABAP runtime of the backend
                                    // (e.g Kernel)
    case EXTERNAL_APPLICATION_FAILURE:    // 6: Problems in the external program
                                          // (e.g in the external server
                                          // implementation)
    case EXTERNAL_AUTHORIZATION_FAILURE:  // 7: Problems raised in the
                                          // authorization check handler
                                          // provided by the external server
                                          // implementation
      errorObj = AbapError(errorInfo);
      break;
    default:

      errorObj = nodeRfcError("wrapError invoked with an unknown err group:" +
                              std::to_string(errorInfo->group))
                     .As<Napi::Object>();
  }

  if (errorPath != nullptr) {
    errorObj.Set("rfmPath", errorPath->getpath());
  }
  return scope.Escape(errorObj);
}

////////////////////////////////////////////////////////////////////////////////
// Connection parameters and client options parsers
////////////////////////////////////////////////////////////////////////////////

void getConnectionParams(Napi::Object clientParamsObject,
                         ConnectionParamsStruct* clientParams) {
  Napi::Array paramNames = clientParamsObject.GetPropertyNames();
  clientParams->paramSize = paramNames.Length();
  if (clientParams->paramSize == 0) {
    Napi::TypeError::New(node_rfc::__env,
                         "Client connection parameters missing")
        .ThrowAsJavaScriptException();
  }
  clientParams->connectionParams =
      new RFC_CONNECTION_PARAMETER[clientParams->paramSize *
                                   sizeof(RFC_CONNECTION_PARAMETER)];
  for (uint_t ii = 0; ii < clientParams->paramSize; ii++) {
    Napi::String name = paramNames.Get(ii).ToString();
    Napi::String value = clientParamsObject.Get(name).ToString();
    clientParams->connectionParams[ii].name = setString(name);
    clientParams->connectionParams[ii].value = setString(value);
  }
}

void checkClientOptions(Napi::Object clientOptionsObject,
                        ClientOptionsStruct* client_options) {
  if (clientOptionsObject.Has("clientOptions")) {
    // server constructor call
    clientOptionsObject =
        clientOptionsObject.Get("clientOptions").As<Napi::Object>();
  }

  char errmsg[ERRMSG_LENGTH];
  Napi::Array props = clientOptionsObject.GetPropertyNames();
  for (uint_t ii = 0; ii < props.Length(); ii++) {
    std::string key = props.Get(ii).ToString().Utf8Value();
    Napi::Value opt = clientOptionsObject.Get(key).As<Napi::Value>();

    // Client option: "bcd"
    if (key == CLIENT_OPTION_BCD) {
      if (opt.IsFunction()) {
        client_options->bcd = CLIENT_OPTION_BCD_FUNCTION;
        client_options->bcdFunction =
            Napi::Persistent(opt.As<Napi::Function>());
      } else if (opt.IsString()) {
        std::string bcdString = opt.ToString().Utf8Value();
        if (bcdString == "number") {
          client_options->bcd = CLIENT_OPTION_BCD_NUMBER;
        } else {
          snprintf(errmsg,
                   ERRMSG_LENGTH - 1,
                   "Client option \"%s\" value not allowed: \"%s\"",
                   CLIENT_OPTION_BCD,
                   &bcdString[0]);
          Napi::TypeError::New(node_rfc::__env, errmsg)
              .ThrowAsJavaScriptException();
        }
      }
    }

    // Client option: "date"
    else if (key == CLIENT_OPTION_DATE) {
      if (!opt.IsObject()) {
        opt = node_rfc::__env.Null();
      } else {
        Napi::Value toABAP = opt.As<Napi::Object>().Get("toABAP");
        Napi::Value fromABAP = opt.As<Napi::Object>().Get("fromABAP");
        if (!toABAP.IsFunction() || !fromABAP.IsFunction()) {
          snprintf(errmsg,
                   ERRMSG_LENGTH - 1,
                   "Client option \"%s\" is not an object with toABAP() and "
                   "fromABAP() functions",
                   CLIENT_OPTION_DATE);
          Napi::TypeError::New(node_rfc::__env, errmsg)
              .ThrowAsJavaScriptException();
        } else {
          client_options->dateToABAP =
              Napi::Persistent(toABAP.As<Napi::Function>());
          client_options->dateFromABAP =
              Napi::Persistent(fromABAP.As<Napi::Function>());
        }
      }
    }

    // Client option: "time"
    else if (key == CLIENT_OPTION_TIME) {
      if (!opt.IsObject()) {
        opt = node_rfc::__env.Null();
      } else {
        Napi::Value toABAP = opt.As<Napi::Object>().Get("toABAP");
        Napi::Value fromABAP = opt.As<Napi::Object>().Get("fromABAP");
        if (!toABAP.IsFunction() || !fromABAP.IsFunction()) {
          snprintf(errmsg,
                   ERRMSG_LENGTH - 1,
                   "Client option \"%s\" is not an object with toABAP() and "
                   "fromABAP() functions",
                   CLIENT_OPTION_TIME);
          Napi::TypeError::New(node_rfc::__env, errmsg)
              .ThrowAsJavaScriptException();
          ;
        } else {
          client_options->timeToABAP =
              Napi::Persistent(toABAP.As<Napi::Function>());
          client_options->timeFromABAP =
              Napi::Persistent(fromABAP.As<Napi::Function>());
        }
      }
    }

    // Client option: "filter"
    else if (key == CLIENT_OPTION_FILTER) {
      client_options->filter_param_type =
          (RFC_DIRECTION)clientOptionsObject.Get(key)
              .As<Napi::Number>()
              .Uint32Value();
      if (((int)client_options->filter_param_type < 1) ||
          ((int)client_options->filter_param_type) > 4) {
        snprintf(errmsg,
                 ERRMSG_LENGTH - 1,
                 "Client option \"%s\" value allowed: \"%u\"",
                 CLIENT_OPTION_FILTER,
                 (uint_t)client_options->filter_param_type);
        Napi::TypeError::New(node_rfc::__env, errmsg)
            .ThrowAsJavaScriptException();
      }
    }

    // Client option: "stateless"
    else if (key == CLIENT_OPTION_STATELESS) {
      if (!opt.IsBoolean()) {
        snprintf(errmsg,
                 ERRMSG_LENGTH - 1,
                 "Client option \"%s\" requires a boolean value",
                 CLIENT_OPTION_STATELESS);
        Napi::TypeError::New(node_rfc::__env, errmsg)
            .ThrowAsJavaScriptException();
      }
      client_options->stateless = opt.As<Napi::Boolean>();
    }

    // Client option: "timeout"
    else if (key == CLIENT_OPTION_TIMEOUT) {
      if (!opt.IsNumber()) {
        snprintf(errmsg,
                 ERRMSG_LENGTH - 1,
                 "Client option \"%s\" requires a number of seconds",
                 CLIENT_OPTION_TIMEOUT);
        Napi::TypeError::New(node_rfc::__env, errmsg)
            .ThrowAsJavaScriptException();
      }
      client_options->timeout = opt.As<Napi::Number>();
    }

    else if (key == SRV_OPTION_LOG_LEVEL) {
      _log.set_log_level(logClass::client, opt);
    }

    // Client option: unknown
    else {
      snprintf(errmsg,
               ERRMSG_LENGTH - 1,
               "Client option not allowed: \"%s\"",
               key.c_str());
      Napi::TypeError::New(node_rfc::__env, errmsg)
          .ThrowAsJavaScriptException();
    }
  }
}

Napi::Value getConnectionAttributes(Napi::Env env,
                                    RFC_CONNECTION_HANDLE connectionHandle) {
  Napi::Object infoObj = Napi::Object::New(env);
  RFC_ERROR_INFO errorInfo;
  RFC_ATTRIBUTES connInfo;
  RFC_RC rc =
      RfcGetConnectionAttributes(connectionHandle, &connInfo, &errorInfo);

  if (rc != RFC_OK || errorInfo.code != RFC_OK) {
    return rfcSdkError(&errorInfo);
  }
  CONNECTION_INFO_SET(dest);
  CONNECTION_INFO_SET(host);
  CONNECTION_INFO_SET(partnerHost)
  CONNECTION_INFO_SET(sysNumber);
  CONNECTION_INFO_SET(sysId);
  CONNECTION_INFO_SET(client);
  CONNECTION_INFO_SET(user);
  CONNECTION_INFO_SET(language);
  CONNECTION_INFO_SET(trace);
  CONNECTION_INFO_SET(isoLanguage);
  CONNECTION_INFO_SET(codepage);
  CONNECTION_INFO_SET(partnerCodepage);
  CONNECTION_INFO_SET(rfcRole);
  CONNECTION_INFO_SET(type);
  CONNECTION_INFO_SET(partnerType);
  CONNECTION_INFO_SET(rel);
  CONNECTION_INFO_SET(partnerRel);
  CONNECTION_INFO_SET(kernelRel);
  CONNECTION_INFO_SET(cpicConvId);
  CONNECTION_INFO_SET(progName);
  CONNECTION_INFO_SET(partnerBytesPerChar);
  CONNECTION_INFO_SET(partnerSystemCodepage);
  CONNECTION_INFO_SET(partnerIP);
  CONNECTION_INFO_SET(partnerIPv6);

  return infoObj;
}

}  // namespace node_rfc

// SPDX-FileCopyrightText: 2014 SAP SE Srdjan Boskovic <srdjan.boskovic@sap.com>
//
// SPDX-License-Identifier: Apache-2.0

import Decimal from "decimal.js";

//  RFC return codes
export enum RFC_RC {
    RFC_OK, ///< Everything O.K. Used by every function
    RFC_COMMUNICATION_FAILURE, ///< Error in Network & Communication layer
    RFC_LOGON_FAILURE, ///< Unable to logon to SAP system. Invalid password, user locked, etc.
    RFC_ABAP_RUNTIME_FAILURE, ///< SAP system runtime error (SYSTEM_FAILURE): Shortdump on the backend side
    RFC_ABAP_MESSAGE, ///< The called function module raised an E-, A- or X-Message
    RFC_ABAP_EXCEPTION, ///< The called function module raised an Exception (RAISE or MESSAGE ... RAISING)
    RFC_CLOSED, ///< Connection closed by the other side
    RFC_CANCELED, ///< No longer used
    RFC_TIMEOUT, ///< Time out
    RFC_MEMORY_INSUFFICIENT, ///< Memory insufficient
    RFC_VERSION_MISMATCH, ///< Version mismatch
    RFC_INVALID_PROTOCOL, ///< The received data has an unsupported format
    RFC_SERIALIZATION_FAILURE, ///< A problem while serializing or deserializing RFM parameters
    RFC_INVALID_HANDLE, ///< An invalid handle was passed to an API call
    RFC_RETRY, ///< RfcListenAndDispatch did not receive an RFC request during the timeout period
    RFC_EXTERNAL_FAILURE, ///< Error in external custom code. (E.g. in the function handlers or tRFC handlers.) Results in SYSTEM_FAILURE
    RFC_EXECUTED, ///< Inbound tRFC Call already executed (needs to be returned from RFC_ON_CHECK_TRANSACTION in case the TID is already known and successfully processed before.)
    RFC_NOT_FOUND, ///< Function or structure definition not found (Metadata API)
    RFC_NOT_SUPPORTED, ///< The operation is not supported on that handle
    RFC_ILLEGAL_STATE, ///< The operation is not supported on that handle at the current point of time (e.g. trying a callback on a server handle, while not in a call)
    RFC_INVALID_PARAMETER, ///< An invalid parameter was passed to an API call, (e.g. invalid name, type or length)
    RFC_CODEPAGE_CONVERSION_FAILURE, ///< Codepage conversion error
    RFC_CONVERSION_FAILURE, ///< Error while converting a parameter to the correct data type
    RFC_BUFFER_TOO_SMALL, ///< The given buffer was to small to hold the entire parameter. Data has been truncated.
    RFC_TABLE_MOVE_BOF, ///< Trying to move the current position before the first row of the table
    RFC_TABLE_MOVE_EOF, ///< Trying to move the current position after the last row of the table
    RFC_START_SAPGUI_FAILURE, ///< Failed to start and attach SAPGUI to the RFC connection
    RFC_ABAP_CLASS_EXCEPTION, ///< The called function module raised a class based exception
    RFC_UNKNOWN_ERROR, ///< "Something" went wrong, but I don't know what...
    RFC_AUTHORIZATION_FAILURE, ///< Authorization check error
    RFC_AUTHENTICATION_FAILURE, ///< The authentication handler (RFC_ON_AUTHENTICATION_CHECK) failed to authenticate the user trying to log on
    RFC_CRYPTOLIB_FAILURE, ///< Error when dealing with functions provided by the cryptolibrary
    RFC_IO_FAILURE, ///< Error when dealing with io functions, streams etc
    RFC_LOCKING_FAILURE, ///< Requesting or freeing critical sections or mutex failed
    //_RFC_RC_max_value, ///< Don't use
}

// RFC connection parameters

export enum EnumSncQop {
    DigSig = "1",
    DigSigEnc = "2",
    DigSigEncUserAuth = "3",
    BackendDefault = "8",
    Maximum = "9",
}

export enum EnumTrace {
    Off = "0",
    Brief = "1",
    Verbose = "2",
    Full = "3",
}

type RfcConnectionParametersAllowed =
    | "ABAP_DEBUG"
    | "ALIAS_USER"
    | "ASHOST"
    | "ASXML"
    | "CFIT"
    | "CLIENT"
    | "CODEPAGE"
    | "COMPRESSION_TYPE"
    | "DELTA"
    | "DEST"
    | "EXTIDDATA"
    | "EXTIDTYPE"
    | "GETSSO2"
    | "GROUP"
    | "GWHOST"
    | "GWSERV"
    | "LANG"
    | "LCHECK"
    | "LOGON_GROUP_CHECK_INTERVAL"
    | "MAX_REG_COUNT"
    | "MSHOST"
    | "MSSERV"
    | "MYSAPSSO2"
    | "NO_COMPRESSION"
    | "ON_CCE"
    | "PASSWD"
    | "PASSWORD_CHANGE_ENFORCED"
    | "PCS"
    | "PROGRAM_ID"
    | "PROXY_HOST"
    | "PROXY_PASSWD"
    | "PROXY_PORT"
    | "PROXY_USER"
    | "R3NAME"
    | "REG_COUNT"
    | "SAPLOGON_ID"
    | "SAPROUTER"
    | "SERIALIZATION_FORMAT"
    | "SERVER_NAME"
    | "SNC_LIB"
    | "SNC_MODE"
    | "SNC_MYNAME"
    | "SNC_PARTNERNAME"
    | "SNC_PARTNER_NAMES"
    | "SNC_QOP"
    | "SNC_SSO"
    | "SYSID"
    | "SYSNR"
    | "SYS_IDS"
    | "TLS_CLIENT_CERTIFICATE_LOGON"
    | "TLS_CLIENT_PSE"
    | "TLS_SERVER_PARTNER_AUTH"
    | "TLS_SERVER_PSE"
    | "TLS_TRUST_ALL"
    | "TPNAME"
    | "TRACE"
    | "USER"
    | "USE_REPOSITORY_ROUNDTRIP_OPTIMIZATION"
    | "USE_SAPGUI"
    | "USE_SYMBOLIC_NAMES"
    | "USE_TLS"
    | "WSHOST"
    | "WSPORT"
    | "X509CERT"
    | "abap_debug"
    | "alias_user"
    | "ashost"
    | "asxml"
    | "cfit"
    | "client"
    | "codepage"
    | "compression_type"
    | "delta"
    | "dest"
    | "extiddata"
    | "extidtype"
    | "getsso2"
    | "group"
    | "gwhost"
    | "gwserv"
    | "lang"
    | "lcheck"
    | "logon_group_check_interval"
    | "max_reg_count"
    | "mshost"
    | "msserv"
    | "mysapsso2"
    | "no_compression"
    | "on_cce"
    | "passwd"
    | "password_change_enforced"
    | "pcs"
    | "program_id"
    | "proxy_host"
    | "proxy_passwd"
    | "proxy_port"
    | "proxy_user"
    | "r3name"
    | "reg_count"
    | "saplogon_id"
    | "saprouter"
    | "serialization_format"
    | "server_name"
    | "snc_lib"
    | "snc_mode"
    | "snc_myname"
    | "snc_partnername"
    | "snc_partner_names"
    | "snc_qop"
    | "snc_sso"
    | "sysid"
    | "sysnr"
    | "sys_ids"
    | "tls_client_certificate_logon"
    | "tls_client_pse"
    | "tls_server_partner_auth"
    | "tls_server_pse"
    | "tls_trust_all"
    | "tpname"
    | "trace"
    | "user"
    | "use_repository_roundtrip_optimization"
    | "use_sapgui"
    | "use_symbolic_names"
    | "use_tls"
    | "wshost"
    | "wsport"
    | "x509cert";

export enum RfcParameterDirection {
    RFC_IMPORT = 0x01, ///< Import parameter. This corresponds to ABAP IMPORTING parameter.
    RFC_EXPORT = 0x02, ///< Export parameter. This corresponds to ABAP EXPORTING parameter.
    RFC_CHANGING = RFC_IMPORT | RFC_EXPORT, ///< Import and export parameter. This corresponds to ABAP CHANGING parameter.
    RFC_TABLES = 0x04 | RFC_CHANGING, ///< Table parameter. This corresponds to ABAP TABLES parameter.
}

export type RfcConnectionParameters = Partial<
    Record<RfcConnectionParametersAllowed, string>
>;

// RFC data

export type RfcVariable =
    | string
    | number
    | Buffer
    | Date
    | Decimal
    | Uint8Array
    | Uint16Array
    | Uint32Array;
export type RfcArray = Array<RfcVariable>;
export type RfcStructure = {
    [key: string]: RfcVariable | RfcStructure | RfcTable;
};
export type RfcTable = Array<RfcVariable | RfcStructure>;
export type RfcTableOfVariables = Array<RfcVariable>;
export type RfcTableOfStructures = Array<RfcStructure>;
export type RfcParameterValue =
    | RfcVariable
    | RfcArray
    | RfcStructure
    | RfcTable;
export type RfcObject = { [key: string]: RfcParameterValue };

// Logging options

export enum RfcLoggingClass {
    client = 0,
    pool = 1,
    server = 2,
    throughput = 3,
    nwrfc = 4,
    addon = 5,
}

export enum RfcLoggingLevel {
    none = 0,
    fatal = 1,
    error = 2,
    warning = 3,
    info = 4,
    debug = 5,
    all = 6,
}

export enum RFC_UNIT_STATE {
    RFC_UNIT_NOT_FOUND, ///< No information for this unit ID and unit type can be found in the target system. If you are sure, that target system, unit ID and unit type are correct, it means that your previous attempt did not even reach the target system. Send the unit again. However, if you get this status after the Confirm step has already been executed, it means that everything is ok. Don't re-execute in this case!
    RFC_UNIT_IN_PROCESS, ///< Backend system is still in the process of persisting (or executing if type 'T') the payload data. Give it some more time and check the state again later. If this takes "too long", an admin should probably have a look at why there is no progress here.
    RFC_UNIT_COMMITTED, ///< Data has been persisted (or executed if type 'T') ok on receiver side. Confirm event may now be triggered.
    RFC_UNIT_ROLLED_BACK, ///< An error of any type has occurred. Unit needs to be resent.
    RFC_UNIT_CONFIRMED, ///< Temporary state between the Confirm event and the time, when the status data will be erased for good. Nothing to be done. Just delete the payload and status information on your side.
}

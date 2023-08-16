// SPDX-FileCopyrightText: 2014 SAP SE Srdjan Boskovic <srdjan.boskovic@sap.com>
//
// SPDX-License-Identifier: Apache-2.0

#ifndef NodeRfc_Common_H
#define NodeRfc_Common_H

#include <napi.h>
#include <sapnwrfc.h>

// Error messages
#define ERRMSG_LENGTH 255
#define ERROR_PATH_NAME_LEN 48

// Unsigned and pointer types
#define uint_t uint32_t
#define pointer_t uintptr_t

// client binding version
#define NODERFC_VERSION "3.3.0"

// surpress unused parameter warnings
#define UNUSED(x) (void)(x)

//
// Logging options constants
//

#define LOG_LEVEL_KEY "logLevel"

//
// Client options constants
//
#define CLIENT_OPTION_KEY_BCD "bcd"
#define CLIENT_OPTION_KEY_DATE "date"
#define CLIENT_OPTION_KEY_TIME "time"
#define CLIENT_OPTION_KEY_FILTER "filter"
#define CLIENT_OPTION_KEY_STATELESS "stateless"
#define CLIENT_OPTION_KEY_TIMEOUT "timeout"

#define CALL_OPTION_KEY_NOTREQUESTED "notRequested"
#define CALL_OPTION_KEY_TIMEOUT CLIENT_OPTION_KEY_TIMEOUT

#define CLIENT_OPTION_BCD_STRING 0
#define CLIENT_OPTION_BCD_NUMBER 1
#define CLIENT_OPTION_BCD_FUNCTION 2

//
// Pool options constants
//
#define POOL_KEY_CONNECTION_PARAMS "connectionParameters"
#define POOL_KEY_CLIENT_OPTIONS "clientOptions"
#define POOL_KEY_POOL_OPTIONS "poolOptions"

#define POOL_KEY_OPTION_LOW "low"
#define POOL_KEY_OPTION_HIGH "high"

#define POOL_READY_LOW 2
#define POOL_READY_HIGH 4

#define ENV_UNDEFINED node_rfc::__env.Undefined()

#endif

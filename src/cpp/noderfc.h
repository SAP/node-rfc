// SPDX-FileCopyrightText: 2014 SAP SE Srdjan Boskovic <srdjan.boskovic@sap.com>
//
// SPDX-License-Identifier: Apache-2.0

#ifndef NodeRfc_Common_H
#define NodeRfc_Common_H

#include <napi.h>
#include <sapnwrfc.h>
#include <iostream>

#define USAGE_URL ": https://github.com/SAP/node-rfc#usage"

// Unsigned and pointer types
#define uint_t uint32_t
#define pointer_t uintptr_t

//
// Logging
//
// #define LOG_RFC_CLIENT 1

#ifdef LOG_RFC_CLIENT
// Version unit test will fail, preventing the release with activated logging
#define NODERFC_VERSION "Deactivate logging: LOG_RFC_CLIENT"
#else
// client binding version
#define NODERFC_VERSION "2.7.1"
#endif

template <typename... Args>
void log(Args &&...args)
{
    (std::cerr << ... << args);
    std::cerr << std::endl;
}

// conditional logging
#ifdef LOG_RFC_CLIENT
#define DEBUG(...) \
    log(__VA_ARGS__);
#else
#define DEBUG(...) ;
#endif

// always active logging
#define EDEBUG(...) \
    log(__VA_ARGS__);

//
// Client constants
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
// Pool constants
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

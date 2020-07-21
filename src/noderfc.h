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
//#define LOG_RFC_CLIENT 1

#ifdef LOG_RFC_CLIENT
// Version unit test will fail, preventing the release with activated logging
#define NODERFC_VERSION "Deactivate logging: LOG_RFC_CLIENT"
#else
// client binding version
#define NODERFC_VERSION "2.0.2"
#endif

template <typename... Args>
void log(Args &&... args)
{
    (std::cerr << ... << args);
    std::cerr << std::endl;
}

#ifdef LOG_RFC_CLIENT
#define DEBUG(...) \
    log(__VA_ARGS__);
#else
#define DEBUG(...) ;
#endif

// always active logging

#define ERROR(...) \
    log(__VA_ARGS__);

//
// Client constants
//
#define CLIENT_OPTION_KEY_BCD "bcd"
#define CLIENT_OPTION_KEY_DATE "date"
#define CLIENT_OPTION_KEY_TIME "time"
#define CLIENT_OPTION_KEY_FILTER "filter"
#define CLIENT_OPTION_KEY_STATELESS "stateless"

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

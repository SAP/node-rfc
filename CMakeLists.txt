# SPDX-FileCopyrightText: 2014 SAP SE Srdjan Boskovic <srdjan.boskovic@sap.com>
#
# SPDX-License-Identifier: Apache-2.0

# cmake standard
cmake_minimum_required(VERSION 3.18)
cmake_policy(SET CMP0042 NEW)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# verbose
set(CMAKE_VERBOSE_MAKEFILE ON)

# project
execute_process(COMMAND node -p "require('./package.json').version"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)
project(sapnwrfc)
set(PROJECT_VERSION ${VERSION})

# node platform
execute_process(COMMAND node -p "process.platform"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE NODE_PLATFORM OUTPUT_STRIP_TRAILING_WHITESPACE)

# node version
execute_process(COMMAND node -v WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} OUTPUT_VARIABLE NODE_VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)

# node abi version
execute_process(COMMAND node -p "process.versions.modules"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE NODE_ABI_VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)

# N-API version
execute_process(COMMAND node -p "require('./package.json').binary.napi_versions[0]"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE NAPI_VERSION  OUTPUT_STRIP_TRAILING_WHITESPACE)
# execute_process(COMMAND node -p "process.versions.napi"
#     WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
#     OUTPUT_VARIABLE NAPI_VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)
add_compile_definitions(NAPI_VERSION=${NAPI_VERSION})

# N-API headers
execute_process(COMMAND node -p "require('node-addon-api').include"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE NAPI_INCLUDE_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE)
string(REPLACE "\"" "" NAPI_INCLUDE_DIR ${NAPI_INCLUDE_DIR})

# NodeJS headers
if(NOT DEFINED ENV{NODE_HEADERS_INCLUDE_DIR})
    if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
        execute_process(COMMAND cmd /c "npm -g prefix"
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            OUTPUT_VARIABLE NODE_HEADERS_INCLUDE_DIR
            OUTPUT_STRIP_TRAILING_WHITESPACE)
    else()
        execute_process(COMMAND npm -g prefix
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            OUTPUT_VARIABLE NODE_HEADERS_INCLUDE_DIR
            OUTPUT_STRIP_TRAILING_WHITESPACE)
    endif()

    string(CONCAT NODE_HEADERS_INCLUDE_DIR ${NODE_HEADERS_INCLUDE_DIR} "/include")

    if(NOT EXISTS "${NODE_HEADERS_INCLUDE_DIR}")
        set(NODE_HEADERS_FILENAME "node-${NODE_VERSION}-headers.tar.gz")
        set(NODE_HEADERS_DOWNLD_PATH ${CMAKE_CURRENT_BINARY_DIR}/${NODE_HEADERS_FILENAME})
        message("Downloading ${NODE_HEADERS_FILENAME} to ${NODE_HEADERS_DOWNLD_PATH} ...")
        file(DOWNLOAD "https://nodejs.org/download/release/${NODE_VERSION}/${NODE_HEADERS_FILENAME}" "${NODE_HEADERS_DOWNLD_PATH}") # SHOW_PROGRESS)

        file(ARCHIVE_EXTRACT INPUT ${NODE_HEADERS_DOWNLD_PATH} VERBOSE)
        file(REMOVE ${NODE_HEADERS_DOWNLD_PATH})
        file(RENAME "${CMAKE_CURRENT_BINARY_DIR}/node-${NODE_VERSION}/include" "${NODE_HEADERS_INCLUDE_DIR}")
        file(REMOVE_RECURSE "${CMAKE_CURRENT_BINARY_DIR}/node-${NODE_VERSION}")
    endif()
else()
    set(NODE_HEADERS_INCLUDE_DIR $ENV{NODE_HEADERS_INCLUDE_DIR})
endif()

# build and binding path
string(CONCAT ADDON_BINDING_PATH "node-v" ${NODE_ABI_VERSION} "-" ${NODE_PLATFORM} "-" ${NODE_ARCH})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib/binding/)

# sapnwrfc dependencies
set(SAPNWRFC_INCLUDE_DIR $ENV{SAPNWRFC_HOME}/include)
set(SAPNWRFC_LIB_DIR $ENV{SAPNWRFC_HOME}/lib)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    set(SAPNWRFC_LIB_FILES sapnwrfc.lib libsapucum.lib)
else()
    set(SAPNWRFC_LIB_FILES -lsapnwrfc -lsapucum)
endif()

# source files and target library
add_library(${PROJECT_NAME} SHARED src/cpp/Client.cc src/cpp/nwrfcsdk.cc src/cpp/Pool.cc src/cpp/Throughput.cc src/cpp/Server.cc src/cpp/addon.cc)

# build path ignored on Windows, copy after build
if(MSVC)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy
        "$<TARGET_FILE:${PROJECT_NAME}>"
        "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}\\$<TARGET_FILE_NAME:${PROJECT_NAME}>"
        COMMENT "Copying to output directory" ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
endif(MSVC)

#
# build
#
message("N-API version: ${NAPI_VERSION}   ${NODE_HEADERS_INCLUDE_DIR}")

# All platforms

# https://launchpad.support.sap.com/#/notes/2573953
target_compile_definitions(${PROJECT_NAME} PUBLIC
    _CONSOLE
    SAPwithUNICODE UNICODE _UNICODE
    _FILE_OFFSET_BITS=64
    _LARGEFILE_SOURCE
    NDEBUG
    SAPwithTHREADS
    NAPI_CPP_EXCEPTIONS)

# target_compile_options(${PROJECT_NAME} PUBLIC -fexceptions)
if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    # show SAP NWRFC SDK version
    execute_process(COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/ci/utils/nwrfcsdk-version-linux.sh OUTPUT_VARIABLE SAPNWRFCSDK_VERSION)
    message("SAP NWRFC SDK: " ${SAPNWRFCSDK_VERSION})

    target_compile_options(${PROJECT_NAME} PUBLIC -std=c++11 -Wall)

    add_compile_definitions(
        SAPonLIN
        SAPonUNIX
        __NO_MATH_INLINES)

elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    # show SAP NWRFC SDK version
    execute_process(
        COMMAND "${CMAKE_CURRENT_SOURCE_DIR}\\ci\\utils\\nwrfcsdk-version.bat"
        OUTPUT_VARIABLE SAPNWRFCSDK_VERSION)
    message("SAP NWRFC SDK: " ${SAPNWRFCSDK_VERSION})

    # https://docs.microsoft.com/en-us/cpp/build/reference/compiler-options-listed-alphabetically
    target_compile_options(${PROJECT_NAME} PUBLIC
        -EHs
        -Gy
        -J
        -MD
        -nologo
        -W3
        -Z7
        -GL -O2 -Oy-
        /we4552 /we4700 /we4789
    )

    target_compile_definitions(${PROJECT_NAME} PUBLIC
        SAP_PLATFORM_MAKENAME=ntintel
        SAPonNT
        _CRT_NON_CONFORMING_SWPRINTFS
        _CRT_SECURE_NO_DEPRECATE

        # _CRT_SECURE_NO_WARNINGS
        _CRT_NONSTDC_NO_DEPRECATE
        _AFXDLL
        WIN32
        _WIN32_WINNT=0x0502
        _ATL_ALLOW_CHAR_UNSIGNED
    )

    # https://docs.microsoft.com/en-us/cpp/build/reference/linker-options?view=vs-2019
    target_link_options(${PROJECT_NAME} PUBLIC
        /NXCOMPAT
        /SWAPRUN:NET
        /OPT:REF
        /NOLOGO
        /LTCG
    )

    if(${NODE_ARCH} STREQUAL "ia32")
        target_compile_definitions(${PROJECT_NAME} PUBLIC _X86_)

        target_link_options(${PROJECT_NAME} PUBLIC
            -STACK:0x800000
            -DEBUGTYPE:CV
            -MACHINE:x86
            -LARGEADDRESSAWARE
        )
    else()
        target_compile_definitions(${PROJECT_NAME} PUBLIC WIN64 _AMD64_)

        target_link_options(${PROJECT_NAME} PUBLIC
            -STACK:0x2000000
            -DEBUG
            -DEBUGTYPE:CV,FIXUP
            -MACHINE:amd64
        )
    endif()

elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    # show SAP NWRFC SDK version
    execute_process(COMMAND bash "${CMAKE_CURRENT_SOURCE_DIR}/ci/utils/nwrfcsdk-version-darwin.sh"
        OUTPUT_VARIABLE SAPNWRFCSDK_VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)
    message("SAP NWRFC SDK: " ${SAPNWRFCSDK_VERSION})

    target_compile_options(${PROJECT_NAME} PUBLIC -stdlib=libc++ -std=c++0x -Wall -mmacosx-version-min=10.10 -fvisibility=hidden)

    add_compile_definitions(
        SAPonDARW
        __NO_MATH_INLINES)
endif()

# set .node extension without any "lib" prefix
set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 17
    PREFIX ""
    SUFFIX ".node")

set(${PROJECT_NAME}_USE_STATIC_LIBS OFF)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake")

# build target
target_include_directories(${PROJECT_NAME} PRIVATE "${NODE_HEADERS_INCLUDE_DIR}/node" ${NAPI_INCLUDE_DIR} ${SAPNWRFC_INCLUDE_DIR})
target_link_directories(${PROJECT_NAME} PRIVATE ${SAPNWRFC_LIB_DIR})
target_link_libraries(${PROJECT_NAME} ${CMAKE_JS_LIB} ${SAPNWRFC_LIB_FILES})

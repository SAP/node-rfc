# cmake standard
cmake_minimum_required(VERSION 3.15)
cmake_policy(SET CMP0042 NEW)

# verbose
set(CMAKE_VERBOSE_MAKEFILE ON)

# C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# project
execute_process(COMMAND node -p "require('./package.json').version"
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE VERSION)
string(REPLACE "\n" "" VERSION ${VERSION})
project(sapnwrfc VERSION ${VERSION})

# node platform
execute_process(COMMAND node -p "process.platform"
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE NODE_PLATFORM)
string(REPLACE "\n" "" NODE_PLATFORM ${NODE_PLATFORM})
string(REPLACE "\"" "" NODE_PLATFORM ${NODE_PLATFORM})

# node abi version
execute_process(COMMAND node -p "process.versions.modules"
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE NODE_ABI_VERSION)
string(REPLACE "\n" "" NODE_ABI_VERSION ${NODE_ABI_VERSION})
string(REPLACE "\"" "" NODE_ABI_VERSION ${NODE_ABI_VERSION})

# N-API
set (NAPI_BUILD_VERSION 3)
add_compile_definitions(NAPI_VERSION=${NAPI_BUILD_VERSION})

execute_process(COMMAND node -p "require('node-addon-api').include"
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE NAPI_INCLUDE_DIR
    )
string(REPLACE "\n" "" NAPI_INCLUDE_DIR ${NAPI_INCLUDE_DIR})
string(REPLACE "\"" "" NAPI_INCLUDE_DIR ${NAPI_INCLUDE_DIR})

# NodeJS headers path
if (NOT DEFINED ENV{NODE_HEADERS_DIR})
    if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows") # Try defaults if not defined
        execute_process(COMMAND cmd /c npm -g root
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            OUTPUT_VARIABLE NODE_HEADERS_DIR
        )
        string(REPLACE "\\" "/" NODE_HEADERS_DIR ${NODE_HEADERS_DIR})
        string(CONCAT NODE_HEADERS_DIR ${NODE_HEADERS_DIR} "/../include/node")
    else()
        execute_process(COMMAND npm -g root
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            OUTPUT_VARIABLE NODE_HEADERS_DIR
        )
        string(CONCAT NODE_HEADERS_DIR ${NODE_HEADERS_DIR} "/../../include/node")
    endif()
    string(REPLACE "\n" "" NODE_HEADERS_DIR ${NODE_HEADERS_DIR})
endif()

# download headers
if (NOT EXISTS "${NODE_HEADERS_DIR}")
    execute_process(COMMAND node -v WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} OUTPUT_VARIABLE NODE_HEADERS_VERSION)
    string(REPLACE "\n" "" NODE_HEADERS_VERSION ${NODE_HEADERS_VERSION})
    set(NODE_HEADERS_FILENAME "node-${NODE_HEADERS_VERSION}-headers.tar.gz")
    set(NODE_HEADERS_DIR "${CMAKE_CURRENT_BINARY_DIR}")
    set(NODE_HEADERS_ARCHIVE_PATH ${NODE_HEADERS_DIR}/${NODE_HEADERS_FILENAME})
    message("Downloading ${NODE_HEADERS_FILENAME} to: ${NODE_HEADERS_DIR}")
    file(DOWNLOAD "https://nodejs.org/download/release/${NODE_HEADERS_VERSION}/${NODE_HEADERS_FILENAME}" "${NODE_HEADERS_ARCHIVE_PATH}" SHOW_PROGRESS)

    if (NOT EXISTS "${NODE_HEADERS_ARCHIVE_PATH}")
        message(FATAL_ERROR "Not found: ${NODE_HEADERS_ARCHIVE_PATH}")
    else()
        execute_process(COMMAND tar -xzf ${NODE_HEADERS_ARCHIVE_PATH})
        file(REMOVE ${NODE_HEADERS_ARCHIVE_PATH})
        set(NODE_HEADERS_DIR "${NODE_HEADERS_DIR}/node-${NODE_HEADERS_VERSION}/include/node")
        if (NOT EXISTS "${NODE_HEADERS_DIR}")
            message(FATAL_ERROR "NodeJS headers missing: ${NODE_HEADERS_DIR}")
        endif()
    endif()
endif()

# build and binding path
string(CONCAT ADDON_BINDING_PATH "node-v" ${NODE_ABI_VERSION} "-" ${NODE_PLATFORM} "-" ${NODE_ARCH} )
if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}\\lib\\binding\\)
    string(REPLACE "/" "\\" CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}) # CMAKE_SOURCE_DIR with / on Windows ??
else()
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib/binding/)
endif()

# sapnwrfc dependencies
if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    set(SAPNWRFC_INCLUDE_DIR $ENV{SAPNWRFC_HOME}\\include)
    set(SAPNWRFC_LIB_DIR $ENV{SAPNWRFC_HOME}\\lib)
    set(SAPNWRFC_LIB_FILES sapnwrfc.lib libsapucum.lib)
else()
    set(SAPNWRFC_INCLUDE_DIR $ENV{SAPNWRFC_HOME}/include)
    set(SAPNWRFC_LIB_DIR $ENV{SAPNWRFC_HOME}/lib)
    set(SAPNWRFC_LIB_FILES -lsapnwrfc -lsapucum)
endif()

# source files and target library
add_library(${PROJECT_NAME} SHARED src/node_sapnwrfc.cc src/Client.cc src/rfcio.cc src/error.cc)

# build path ignored on Windows, copy after build
if ( MSVC )
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy
            "$<TARGET_FILE:${PROJECT_NAME}>"
            "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}\\$<TARGET_FILE_NAME:${PROJECT_NAME}>"
    COMMENT "Copying to output directory" ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
endif ( MSVC )

# build

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-exceptions")

#target_compile_options(${PROJECT_NAME} PUBLIC -fno-exceptions)

# https://launchpad.support.sap.com/#/notes/2573953
target_compile_definitions(${PROJECT_NAME} PUBLIC
    _CONSOLE
    SAPwithUNICODE
    UNICODE
    _UNICODE
    SAPwithTHREADS
    _LARGEFILE_SOURCE
    _FILE_OFFSET_BITS=64
    NDEBUG
    NAPI_CPP_EXCEPTION)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")

    # display SAP NWRFC SDK version
    execute_process(COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/ci/utils/nwrfcsdk-version-linux.sh OUTPUT_VARIABLE SAPNWRFCSDK_VERSION)
    message("SAP NWRFC SDK: " ${SAPNWRFCSDK_VERSION})

    target_compile_options(${PROJECT_NAME} PUBLIC -std=c++11 -Wall)

    add_compile_definitions(
        SAPonLIN
        SAPonUNIX
        __NO_MATH_INLINES)

elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    # display SAP NWRFC SDK version
    execute_process(
        COMMAND "${CMAKE_CURRENT_SOURCE_DIR}\\ci\\utils\\nwrfcsdk-version.bat"
        OUTPUT_VARIABLE SAPNWRFCSDK_VERSION)
    message("SAP NWRFC SDK: " ${SAPNWRFCSDK_VERSION})

    # https://docs.microsoft.com/en-us/cpp/build/reference/compiler-options-listed-alphabetically
    target_compile_definitions(${PROJECT_NAME} PUBLIC
        SAP_PLATFORM_MAKENAME=ntintel
        SAPonNT
        _CRT_NON_CONFORMING_SWPRINTFS
        _CRT_SECURE_NO_DEPRECATES
        _CRT_SECURE_NO_WARNINGS
        _CRT_NONSTDC_NO_DEPRECATE
        _AFXDLL
        WIN32
        _WIN32_WINNT=0x0502
        WIN64
        _AMD64_
        _ATL_ALLOW_CHAR_UNSIGNED
    )

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


    target_link_options(${PROJECT_NAME} PUBLIC
        -NXCOMPAT -STACK:0x2000000 -SWAPRUN:NET -DEBUG
        -OPT:REF -DEBUGTYPE:CV,FIXUP -MACHINE:amd64 -nologo
        -LTCG
    )

elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")

    # display SAP NWRFC SDK version
    execute_process(COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/ci/utils/nwrfcsdk-version-darwin.sh OUTPUT_VARIABLE SAPNWRFCSDK_VERSION)
    message("SAP NWRFC SDK: " ${SAPNWRFCSDK_VERSION})

    target_compile_options(${PROJECT_NAME} PUBLIC -stdlib=libc++ -std=c++0x -Wall -mmacosx-version-min=10.10)

    add_compile_definitions(
        SAPonDARW
        __NO_MATH_INLINES)

endif()

# set .node extension without any "lib" prefix
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")

set (${PROJECT_NAME}_USE_STATIC_LIBS OFF)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake")

# build target
target_include_directories(${PROJECT_NAME} PRIVATE ${NODE_HEADERS_DIR} ${NAPI_INCLUDE_DIR} ${SAPNWRFC_INCLUDE_DIR})
target_link_directories(${PROJECT_NAME} PRIVATE ${SAPNWRFC_LIB_DIR})
target_link_libraries(${PROJECT_NAME} ${CMAKE_JS_LIB} ${SAPNWRFC_LIB_FILES})

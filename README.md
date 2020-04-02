Asynchronous, non-blocking [SAP NetWeawer RFC SDK](https://support.sap.com/en/products/connectors/nwrfcsdk.html) client bindings for [Node.js](http://nodejs.org/).

[![NPM](https://nodei.co/npm/node-rfc.png?downloads=true&downloadRank=true)](https://nodei.co/npm/node-rfc/)

[![license](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)
[![N-API v3 Badge](https://github.com/nodejs/abi-stable-node/raw/doc/assets/N-API%20v3%20Badge.svg?sanitize=true)](https://github.com/nodejs/abi-stable-node/)
[![release](https://img.shields.io/npm/v/node-rfc.svg)](https://www.npmjs.com/package/node-rfc)
[![downloads](https://img.shields.io/github/downloads/sap/node-rfc/total.svg)](https://www.npmjs.com/package/node-rfc)
[![dpw](https://img.shields.io/npm/dm/node-rfc.svg)](https://www.npmjs.com/package/node-rfc)

## Features

-   Based on [N-API](https://github.com/nodejs/node-addon-api) standard
-   Stateless and stateful connections (multiple function calls in the same ABAP session (same context))
-   Async/await, promise and callback API
-   Sequential and parallel calls, using one or more clients
-   Automatic conversion between JavaScript and ABAP datatypes
-   Buffer, Decimal and Date objects support
-   Connection pool
-   :new: Throughput monitoring: number of calls, bytes sent/received, application/total time; SAP NWRFC SDK >= 7.53 required
-   Extensive unit tests

## Supported platforms

-   [Current and active nodejs LTS releases](https://github.com/nodejs/LTS)

-   The _node-rfc_ connector can be [built from source](http://sap.github.io/node-rfc/install.html#building-from-source) on all [platforms supported by SAP NW RFC SDK](https://launchpad.support.sap.com/#/notes/2573790) and by [nodejs](https://github.com/nodejs/node/blob/master/BUILDING.md#supported-platforms-1)

-   Pre-built binaries are provided for [active nodejs LTS releases](https://github.com/nodejs/LTS), for 64 bit Windows 8.1, Ubuntu 16.04 and macOS 10.14.

## Prerequisites

### All platforms

-   SAP NW RFC SDK C++ binaries must be downloaded (SAP partner or customer account required) and locally installed ([installation instructions](http://sap.github.io/node-rfc/install.html#sap-nw-rfc-library-installation)). More information on [SAP NW RFC SDK section on SAP Support Portal](https://support.sap.com/en/product/connectors/nwrfcsdk.html). Using the latest version is reccomended as SAP NW RFC SDK is fully backwards compatible, supporting all NetWeaver systems, from today S4, down to R/3 release 4.6C.

-   Build toolchain requires [CMake](https://cmake.org/)

-   Build from source on macOS and older Linux systems, may require `uchar.h` file, attached to [SAP OSS Note 2573953](https://launchpad.support.sap.com/#/notes/2573953), to be copied to SAP NW RFC SDK include directory: [documentation](http://sap.github.io/node-rfc/install.html#macos)

### Windows

-   [Visual C++ Redistributable](https://www.microsoft.com/en-US/download/details.aspx?id=40784) is required for runtime. The version is given in [SAP Note 2573790 - Installation, Support and Availability of the SAP NetWeaver RFC Library 7.50](https://launchpad.support.sap.com/#/notes/2573790)

-   Build toolchain requires [Microsoft C++ Build Tools](https://aka.ms/buildtools), the latest version reccomended

### macOS

-   The macOS firewall stealth mode must be disabled ([Can't ping a machine - why?](https://discussions.apple.com/thread/2554739)):

```shell
sudo /usr/libexec/ApplicationFirewall/socketfilterfw --setstealthmode off
```

-   Remote paths must be set in SAP NWRFC SDK for macOS: [documentation](http://sap.github.io/node-rfc/install.html#macos)

## Installation

After the SAP NW RFC SDK is installed on your system, you can install the _node-rfc_ package from npm:

```shell
yarn add node-rfc
```

```shell
npm install node-rfc
```

Alternatively, or if the _node-rfc_ package not provided for your platform, [buld the package from the latest source](<(http://sap.github.io/node-rfc/install.html#building-from-source)>) and install:

```shell
git clone -b https://github.com/SAP/node-rfc.git
cd node-rfc
npm install
# set connection properties in test/abapSystem
npm test
```

## Getting started

**Note:** the module must be [installed](#installation) before use.

In order to call remote enabled ABAP function module, we need to create a client
with valid logon credentials, connect to SAP ABAP NetWeaver system and then invoke a
remote enabled ABAP function module from nodejs.

Connection parameters for remote ABAP systems are documented in **sapnwrfc.ini** file, located in the SAP NWRFC SDK `demo` folder

The client can be used for one or more subsequent RFC calls and for more examples check unit tests.

Callback API example below shows basic principles. See also:

-   [**Examples and API**](examples/README.md)

-   [**node-rfc documentation**](http://sap.github.io/node-rfc), complementing SAP NW RFC Library [programming guide and documentation](https://support.sap.com/en/products/connectors/nwrfcsdk.html)

```javascript
"use strict";

const Client = require("node-rfc").Client;

// ABAP system RFC connection parameters
const abapSystem = {
    user: "demo",
    passwd: "welcome",
    ashost: "10.68.104.164",
    sysnr: "00",
    client: "620",
    lang: "EN"
};

// create new client
const client = new Client(abapSystem);

// echo SAP NWRFC SDK and nodejs/RFC binding version
console.log("Client version: ", client.version);

// open connection
client.connect(function(err) {
    if (err) {
        // check for login/connection errors
        return console.error("could not connect to server", err);
    }

    // invoke ABAP function module, passing structure and table parameters

    // ABAP structure
    const structure = {
        RFCINT4: 345,
        RFCFLOAT: 1.23456789,
        // or RFCFLOAT: require('decimal.js')('1.23456789'), // as Decimal object
        RFCCHAR4: "ABCD",
        RFCDATE: "20180625" // in ABAP date format
        // or RFCDATE: new Date('2018-06-25'), // as JavaScript Date object
    };

    // ABAP table
    let table = [structure];

    client.invoke(
        "STFC_STRUCTURE",
        { IMPORTSTRUCT: structure, RFCTABLE: table },
        function(err, res) {
            if (err) {
                return console.error("Error invoking STFC_STRUCTURE:", err);
            }
            console.log("STFC_STRUCTURE call result:", res);
        }
    );
});
```

Finally, the connection is closed automatically when the instance is deleted by the garbage collector or by explicitly calling the `client.close()` method on the client instance.

## Known Issues

-   NAPI Type checks [nodejs/node-addon-api/#265](https://github.com/nodejs/node-addon-api/issues/265)

## How to obtain support

If you encounter an issue or have a feature request, you can create a [ticket](https://github.com/SAP/node-rfc/issues).

Check out the SCN Forum (search for "node-rfc") and stackoverflow (use the tag "node-rfc"), to discuss code-related problems and questions.

## License

Copyright (c) 2013 SAP SE or an SAP affiliate company. All rights reserved. This file is licensed under the Apache Software License, v. 2 except as noted otherwise in the [LICENSE](LICENSE) file.

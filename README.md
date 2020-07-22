# node-rfc v2

:exclamation: **[CHANGELOG with breaking changes](https://github.com/SAP/node-rfc/releases/tag/v2.0.0)**

Asynchronous, non-blocking [SAP NetWeawer RFC SDK](https://support.sap.com/en/product/connectors/nwrfcsdk.html) client bindings for [Node.js](http://nodejs.org/), providing convenient ABAP business logic consumption from NodeJS.

[![NPM](https://nodei.co/npm/node-rfc.png?downloads=true&downloadRank=true)](https://nodei.co/npm/node-rfc/)

[![license](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)
[![N-API v6 Badge](https://github.com/nodejs/abi-stable-node/raw/doc/assets/N-API%20v6%20Badge.svg?sanitize=true)](https://github.com/nodejs/abi-stable-node/)
[![release](https://img.shields.io/npm/v/node-rfc.svg)](https://www.npmjs.com/package/node-rfc)
[![downloads](https://img.shields.io/github/downloads/sap/node-rfc/total.svg)](https://www.npmjs.com/package/node-rfc)
[![dpw](https://img.shields.io/npm/dm/node-rfc.svg)](https://www.npmjs.com/package/node-rfc)

## Key features

-   Based on [N-API](https://github.com/nodejs/node-addon-api) standard
-   Stateless and stateful connections (multiple function calls in the same ABAP session (same context))
-   Async/await, promise and callback API
-   ECMAScript, TypeScript
-   Sequential and parallel calls, using one or more clients
-   Automatic conversion between NodeJS and ABAP datatypes
-   Direct and managed connections (connection pool)
-   Throughput monitoring: number of calls, bytes sent/received, application/total time; SAP NWRFC SDK >= 7.53 required

## Content

-   **[Supported Platforms](#supported-platforms)**
-   **[Prerequisites](#prerequisites)**
-   **[Installation](#installation)**
-   **[Getting started](#getting-started)**
-   **[Usage](doc/usage.md)**
    -   **[Authentication](doc/authentication.md)**
    -   **[Client](doc/usage.md/#client-toc)**
    -   **[Connection Pool](doc/usage.md/#connection-pool)**
    -   **[Throughput](doc/usage.md/#throughput)**
-   **[API](doc/api.md)**
    -   **[Connection Pool](doc/api.md/#connection-pool)**
    -   **[Client](doc/api.md/#client)**
    -   **[Throughput](doc/api.md/#throughput)**
-   **[Troubleshooting](doc/troubleshooting.md)**
-   **[More resource and info about SAP Connectors and RFC communication](#resources)**
-   **[Code of Conduct](CODE_OF_CONDUCT.md)**
-   **[Contributing](#contributing)**
-   **[License](#license)**

## Supported platforms

-   [Current and active nodejs LTS releases](https://github.com/nodejs/LTS)

-   The _node-rfc_ connector can be [built from source](#setup) on all platforms supported both by [SAP NW RFC SDK](https://launchpad.support.sap.com/#/notes/2573790) and by [nodejs](https://github.com/nodejs/node/blob/master/BUILDING.md#supported-platforms-1)

-   Pre-built binaries are provided for [active nodejs LTS releases](https://github.com/nodejs/LTS), for 64 bit Windows 10, Ubuntu 16.04 and macOS 10.15.

Other platforms and frameworks:

-   [Electron](doc/frameworks/electron-quick-start)
-   [NW.js](doc/frameworks/nwjs-quick-start)
-   [Node-RED](https://github.com/PaulWieland/node-red-contrib-saprfc)
-   [CloudFoundry, AWS Lambdas, heroku ...](https://github.com/SAP/node-rfc/issues/121)

## Prerequisites

### All platforms

-   SAP NW RFC SDK C++ binaries must be downloaded (SAP partner or customer account required) and locally installed ([installation instructions](doc/installation.md#sap-nwrfc-sdk-installation). More information on [SAP NW RFC SDK section on SAP Support Portal](https://support.sap.com/en/product/connectors/nwrfcsdk.html). Using the latest version is reccomended as SAP NW RFC SDK is fully backwards compatible, supporting all NetWeaver systems, from today S4, down to R/3 release 4.6C.

-   Build toolchain requires [CMake](https://cmake.org/)

-   Build from source on macOS and older Linux systems, may require `uchar.h` file, attached to [SAP OSS Note 2573953](https://launchpad.support.sap.com/#/notes/2573953), to be copied to SAP NW RFC SDK include directory: [documentation](http://sap.github

/node-rfc/install.html#macos)

### Windows

-   [Visual C++ Redistributable](https://www.microsoft.com/en-US/download/details.aspx?id=40784) is required for runtime. The version is given in [SAP Note 2573790 - Installation, Support and Availability of the SAP NetWeaver RFC Library 7.50](https://launchpad.support.sap.com/#/notes/2573790)

-   Build toolchain requires [Microsoft C++ Build Tools](https://aka.ms/buildtools), the latest version reccomended

### macOS

-   :exclamation: Stay on SAP NWRFC SDK <= 7.55 until [#143](https://github.com/SAP/node-rfc/issues/143) closed

-   Disable macOS firewall stealth mode ([Can't ping a machine - why?](https://discussions.apple.com/thread/2554739)):

```shell
sudo /usr/libexec/ApplicationFirewall/socketfilterfw --setstealthmode off
```

## Installation

More info: **[Installation](doc/installation.md)**

After the SAP NW RFC SDK is installed on your system, the `node-rfc` can be installed from npm:

```shell
npm install node-rfc
```

Alternatively, when the `node-rfc` package is not provided for your platform for example, you can build the package from source:

```shell
git clone --single-branch https://github.com/SAP/node-rfc.git
cd node-rfc
npm install
npm run addon # rebuild native addon
npm run ts    # rebuild typescript wrapper
```

## Getting started

More info: **[Usage](doc/usage.md)** and **[API](doc/api.md)**

In order to call remote enabled ABAP function module, we need to create a `node-rfc` client instance with valid logon credentials, connect to SAP ABAP NetWeaver system and then invoke a remote enabled ABAP function module from nodejs. The client instance can be used for one or more subsequent RFC calls, see unit tests for more examples. Callback API example below shows basic principles.

```javascript
"use strict";

const Client = require("node-rfc").Client;

const abapSystem = {
    user: "demo",
    passwd: "welcome",
    ashost: "10.68.104.164",
    sysnr: "00",
    client: "620",
    lang: "EN",
};

// create new client
const client = new Client(abapSystem);

// open connection
client.connect(function (err) {
    // check for login/connection errors
    if (err) return console.error("could not connect to server", err);

    // invoke ABAP function module, passing structure and table parameters

    // ABAP structure
    const structure = {
        RFCINT4: 345,
        RFCFLOAT: 1.23456789,
        RFCCHAR4: "ABCD",
        RFCDATE: "20180625", // ABAP date format
        // or RFCDATE: new Date('2018-06-25'), // as JavaScript Date object, with clientOption "date"
    };
    // ABAP table
    let table = [structure];

    client.invoke(
        "STFC_STRUCTURE",
        { IMPORTSTRUCT: structure, RFCTABLE: table },
        function (err, res) {
            if (err)
                return console.error("Error invoking STFC_STRUCTURE:", err);
            console.log("STFC_STRUCTURE call result:", res);
        }
    );
});
```

Finally, the connection is closed automatically when the instance is deleted by the garbage collector or by explicitly calling the `client.close()` method of the direct client, or `client.release()` or `pool.release()` for the managed client.

<a name="resources"></a>

## More resource and info about ABAP Connectors and RFC Communication

Highly reccomended series of three insightful articles about RFC communication and SAP NW RFC Library, published in the SAP Professional Journal (SPJ):

-   **[Part I RFC Client Programming](https://wiki.scn.sap.com/wiki/x/zz27Gg)**
-   **[Part II RFC Server Programming](https://wiki.scn.sap.com/wiki/x/9z27Gg)**
-   **[Part III Advanced Topics](https://wiki.scn.sap.com/wiki/x/FD67Gg)**

and more:

-   **[SAP NWRFC SDK 7.50 Programming Guide](https://support.sap.com/content/dam/support/en_us/library/ssp/products/connectors/nwrfcsdk/NW_RFC_750_ProgrammingGuide.pdf)**
-   **[ABAP Connectors](https://support.sap.com/en/product/connectors.html)**
-   **[SAP NWRFC SDK](https://support.sap.com/en/product/connectors/nwrfcsdk.html)**
-   **[node-addon-api](https://github.com/nodejs/node-addon-api)**

## Known Issues

-   NAPI Type checks [nodejs/node-addon-api/#265](https://github.com/nodejs/node-addon-api/issues/265)

## How to obtain support

If you encounter an issue or have a feature request, you can create a [ticket](https://github.com/SAP/node-rfc/issues).

Check out the SCN Forum (search for "node-rfc") and stackoverflow (use the tag "node-rfc"), to discuss code-related problems and questions.

## **Contributing**

We appreciate contributions from the community to **node-rfc**!
See [CONTRIBUTING.md](CONTRIBUTING.md) for more details on our philosophy around extending this module.

## License

Copyright (c) 2013 SAP SE or an SAP affiliate company. All rights reserved. This file is licensed under the Apache Software License, v. 2 except as noted otherwise in the [LICENSE](LICENSE) file.

# node-rfc

Asynchronous, non-blocking [SAP NetWeaver RFC SDK](https://support.sap.com/en/product/connectors/nwrfcsdk.html) client and server bindings for [Node.js](http://nodejs.org/). Direct consumption of ABAP business logic from Node.js and extending ABAP eco-system with Node.js capabilities, with automatic ABAP <-> Node.js data conversions.

[![node-rfc release](https://img.shields.io/npm/v/node-rfc.svg)](https://www.npmjs.com/package/node-rfc)
[![Node.js engines](https://img.shields.io/node/v/node-rfc.svg)](https://www.npmjs.com/package/node-rfc)
[![N-API version](https://img.shields.io/badge/N--API-v8-green.svg)](https://github.com/nodejs/node-addon-api)
[<img src="https://img.shields.io/badge/Electron-191970?style=for-the-badge&logo=Electron&logoColor=white" height="20"/>](https://www.electronjs.org/)
[![deno version supported](https://shield.deno.dev/deno/latest)](https://deno.land/)
[![dpm](https://img.shields.io/npm/dm/node-rfc.svg)](https://www.npmjs.com/package/node-rfc)
[![REUSE status](https://api.reuse.software/badge/github.com/SAP/node-rfc)](https://api.reuse.software/info/github.com/SAP/node-rfc)
[![CII Best Practices](https://bestpractices.coreinfrastructure.org/projects/4350/badge)](https://bestpractices.coreinfrastructure.org/projects/4350)
[![license](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)

## Key features

- Based on [N-API](https://github.com/nodejs/node-addon-api) standard
- Stateless and stateful connections (multiple function calls in the same ABAP session (same context))
- Async and callback API
- ECMAScript, TypeScript
- Sequential and parallel calls, using one or more clients
- Automatic conversion between Node.js and ABAP datatypes
- Direct and managed connections (connection pool)
- Throughput monitoring: number of calls, bytes sent/received, application/total time; SAP NWRFC SDK >= 7.53 required
- Usage examples & code-snippets: [SAP-samples/node-rfc-samples](https://github.com/SAP-samples/node-rfc-samples)

## Content

- **[Supported Platforms](#supported-platforms)**
- **[Requirements](#requirements)**
- **[Download and installation](#download-and-installation)**
- **[Getting started](#getting-started)**
- **[Usage](doc/usage.md)**
  - **[Authentication](doc/authentication.md)**
  - **[sapnwrfc.ini](doc/usage.md#addon)**
  - **[env](doc/env.md#NODE_RFC_MODULE_PATH)**
  - **[Client](doc/usage.md#client-toc)**
  - **[Server](doc/usage.md#server-toc)**
  - **[Connection Pool](doc/usage.md#connection-pool)**
  - **[Throughput](doc/usage.md#throughput)**
  - **[Logging](doc/usage.md#logging)**
- **[API](doc/api.md)**
  - **[Connection Pool](doc/api.md#connection-pool)**
  - **[Client](doc/api.md#client)**
  - **[Server](doc/api.md#server)**
  - **[Throughput](doc/api.md#throughput)**
- **[Troubleshooting](doc/troubleshooting.md)**
- **[More resource and info about SAP Connectors and RFC communication](#resources)**
- **[Code of Conduct](CODE_OF_CONDUCT.md)**
- **[Contributing](#contributing)**
- **[License](#license)**

## Supported platforms

- Kyma and BTP Node.JS buildpack: [ABAP RFC connectivity from Kyma and BTP Node.JS buildpack](https://blogs.sap.com/2023/10/26/abap-rfc-connectivity-from-btp-node.js-buildpack/)

- AWS Lambdas, heroku ...
  - Create GitHub issue to get up-to-date information
  - Create SAP feature request for `Security Services` category of the [SAP Cloud Platform – Platform Foundation](https://influence.sap.com/sap/ino/#/campaign/2277)

- Operating systems: the _node-rfc_ connector can be built from source([build instructions](#download-and-installation)) on all platforms supported by both [SAP NW RFC SDK](https://launchpad.support.sap.com/#/notes/2573790) and by [Node.js](https://github.com/nodejs/node/blob/master/BUILDING.md#supported-platforms-1)

- Node.js: [current and active LTS releases](https://github.com/nodejs/LTS)

- Docker containers: [SAP fundamental-tools/docker](https://github.com/SAP/fundamental-tools/tree/main/docker)

- Electron: current release, see [SAP-samples/node-rfc-samples/frameworks/electron-quick-start](https://github.com/SAP-samples/node-rfc-samples/tree/main/frameworks/electron-quick-start) and #144

- Deno: latest release

Other platforms and frameworks:

- NW.js
  - Feature request: #144
  - [SAP-samples/node-rfc-samples/frameworks/nwjs-quick-start](https://github.com/SAP-samples/node-rfc-samples/tree/main/frameworks/nwjs-quick-start)

- Node-RED
  - Feature requests: #161 and #148
  - Experimental work: [PaulWieland/node-red-contrib-saprfc](https://github.com/PaulWieland/node-red-contrib-saprfc)

- [Sails JS](https://github.com/dcolley/sailsjs-node-rfc)

## Requirements

### node-gyp

Build toolchain is based on `node-gyp` and Python. For further details check: [node-gyp#Installation](https://github.com/nodejs/node-gyp#installation)

### SAP NW RFC SDK 7.50 PL12

- Release notes: [SAP Note 3337381 - SAP NetWeaver RFC SDK 7.50 -- Patch Level 12](https://me.sap.com/notes/3337381)

- SAP NW RFC SDK C++ binaries must be downloaded from SAP Suport Portal and locally installed. Check [installation instructions](doc/installation.md#sap-nwrfc-sdk-installation) and [SAP NW RFC SDK section on SAP Support Portal](https://support.sap.com/en/product/connectors/nwrfcsdk.html). Using the latest version is reccomended as SAP NW RFC SDK is fully backwards compatible, supporting all NetWeaver systems, from today S4, down to R/3 release 4.6C.

### Docker

Docker container examples for Linux, Intel and ARM based Darwin: [SAP/fundamental-tools/docker](https://github.com/SAP/fundamental-tools/tree/main/docker). SAP NWRFC SDK libraries are not included.

### Linux

- Build toolchain is based on Ubuntu 20.04, for node-rfc binaries compatible with glibc >= 2.28 and libstdc++ >= 6.0.25 (GLIBCXX_3.4.25). The build toolchain follows is defined by [standard Node.js build toolchain configuration](https://github.com/nodejs/node/blob/master/BUILDING.md#official-binary-platforms-and-toolchains)

### Windows

- [Visual C++ Redistributable](https://www.microsoft.com/en-US/download/details.aspx?id=40784) is required for runtime. The version is given in [SAP Note 2573790 - Installation, Support and Availability of the SAP NetWeaver RFC Library 7.50](https://launchpad.support.sap.com/#/notes/2573790)

- Build toolchain requires [Microsoft C++ Build Tools](https://aka.ms/buildtools), the latest version reccomended

### macOS

- Remote paths must be set in SAP NWRFC SDK for macOS: [documentation](http://sap.github.io/PyRFC/install.html#macos)

- When the node-rfc is started for the first time, the popups come-up for each NWRFC SDK library, to confirm it should be opened. If SDK is installed in admin folder, the node-rfc app shall be that first time started with admin privileges, eg. `sudo -E`

## Download and Installation

More info: **[Installation](doc/installation.md)**

:exclamation: The build from source requires Node.js release with minimum N-API version given in `package.json` property "napi_versions": [Node-API version matrix](https://nodejs.org/api/n-api.html#node-api-version-matrix).

After the SAP NW RFC SDK is installed on your system, the `node-rfc` can be installed from npm:

```shell
npm install node-rfc
```

Alternatively, when the `node-rfc` package is not provided for your platform for example, the package shall be built from source. This installation method is highly recommended on Linux platforms:

```shell
git clone https://github.com/SAP/node-rfc.git
cd node-rfc
npm install
npm run addon # rebuild native addon
npm run ts    # rebuild typescript wrapper
```

## Getting started

See [Usage](doc/usage.md) and [API](doc/api.md), also [SAP NWRFC SDK 7.50 Programming Guide](https://support.sap.com/content/dam/support/en_us/library/ssp/products/connectors/nwrfcsdk/NW_RFC_750_ProgrammingGuide.pdf)

In order to call remote enabled ABAP function module, we need to create a `node-rfc` client instance with valid logon credentials, connect to SAP ABAP NetWeaver system and then invoke a remote enabled ABAP function module from nodejs. Async example below shows basic principles and you can check the documentationand unit tests for more examles.

Add your ABAP system destintion to **sapnwrfc.ini** file in your working directory:

```ini
DEST=MME
USER=demo
PASSWD=welcome
ASHOST=myhost
SYSNR=00
CLIENT=620
LANG=EN
```

Connection parameters are documented in `sapnwrfc.ini` file, located in the _SAP NWRFC SDK_ `demo` folder. Check also section `4.1.2 Using sapnwrfc.ini` of [SAP NWRFC SDK 7.50 Programming Guide](https://support.sap.com/content/dam/support/en_us/library/ssp/products/connectors/nwrfcsdk/NW_RFC_750_ProgrammingGuide.pdf)

Call the ABAP RFM. When in doubt about RFM parameters' structure try `abap call` CLI tool of [SAP/fundamental-tools](https://github.com/SAP/fundamental-tools/tree/main/abap-api-tools)

### Direct client

```javascript
const noderfc = require("node-rfc");

const client = new noderfc.Client({ dest: "MME" });

(async () => {
    try {
        // unlike the connection acquired from pool,
        // the direct client connection is initially closed
        await client.open();

        // invoke ABAP function module, passing structure and table parameters

        // ABAP structure
        const abap_structure = {
            RFCINT4: 345,
            RFCFLOAT: 1.23456789,
            RFCCHAR4: "ABCD",
            RFCDATE: "20180625", // ABAP date format
            // or RFCDATE: new Date('2018-06-25'), // as JavaScript Date object, with clientOption "date"
        };
        // ABAP table
        let abap_table = [abap_structure];

        const result = await client.call("STFC_STRUCTURE", {
            IMPORTSTRUCT: abap_structure,
            RFCTABLE: abap_table,
        });

        // check the result
        console.log(result);
    } catch (err) {
        // connection and invocation errors
        console.error(err);
    }
})();
```

### Managed client

```javascript
const noderfc = require("node-rfc");

const pool = new noderfc.Pool({ connectionParameters: { dest: "MME" } });

(async () => {
    try {
        // get a client connection instance
        const client = await pool.acquire();

        // invoke ABAP function module, passing structure and table parameters

        // ABAP structure
        const abap_structure = {
            RFCINT4: 345,
            RFCFLOAT: 1.23456789,
            RFCCHAR4: "ABCD",
            RFCDATE: "20180625", // ABAP date format
            // or RFCDATE: new Date('2018-06-25'), // as JavaScript Date object, with clientOption "date"
        };
        // ABAP table
        let abap_table = [abap_structure];

        const result = await client.call("STFC_STRUCTURE", {
            IMPORTSTRUCT: abap_structure,
            RFCTABLE: abap_table,
        });

        // check the result
        console.log(result);
    } catch (err) {
        // connection and invocation errors
        console.error(err);
    }
})();
```

Finally, the connection is closed automatically when the instance is deleted by the garbage collector or by explicitly calling the `client.close()` method of the direct client, or `client.release()` or `pool.release()` for the managed client.

<a name="resources"></a>

## More resource and info about ABAP Connectors and RFC Communication

Highly reccomended series of three insightful articles about RFC communication and SAP NW RFC Library, published in the SAP Professional Journal (SPJ):

- **[Part I RFC Client Programming](https://wiki.scn.sap.com/wiki/x/zz27Gg)**
- **[Part II RFC Server Programming](https://wiki.scn.sap.com/wiki/x/9z27Gg)**
- **[Part III Advanced Topics](https://wiki.scn.sap.com/wiki/x/FD67Gg)**

and more:

- **[SAP NWRFC SDK 7.50 Programming Guide](https://support.sap.com/content/dam/support/en_us/library/ssp/products/connectors/nwrfcsdk/NW_RFC_750_ProgrammingGuide.pdf)**
- **[ABAP Connectors](https://support.sap.com/en/product/connectors.html)**
- **[SAP NWRFC SDK](https://support.sap.com/en/product/connectors/nwrfcsdk.html)**
- **[node-addon-api](https://github.com/nodejs/node-addon-api)**

## How to obtain support

If you encounter an issue or have a feature request, you can create a [ticket](https://github.com/SAP/node-rfc/issues).

Check out the SCN Forum (search for "node-rfc") and stackoverflow (use the tag "node-rfc"), to discuss code-related problems and questions.

## Contributing

We appreciate contributions from the community to **node-rfc**!
See [CONTRIBUTING.md](CONTRIBUTING.md) for more details on our philosophy around extending this module.

## Code of Conduct

See [Code of Conduct](./CODE_OF_CONDUCT.md)

## License

Copyright (c) 2018 SAP SE or an SAP affiliate company. All rights reserved. This file is licensed under the Apache Software License, v. 2 except as noted otherwise in the [LICENSE file](LICENSES/Apache-2.0.txt).

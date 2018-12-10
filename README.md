Asynchronous, non-blocking [SAP NetWeawer RFC SDK](https://support.sap.com/en/products/connectors/nwrfcsdk.html) client bindings for [Node.js](http://nodejs.org/).

[![NPM](https://nodei.co/npm/node-rfc.png?downloads=true&downloadRank=true)](https://nodei.co/npm/node-rfc/)

[![license](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)
[![release](https://img.shields.io/npm/v/node-rfc.svg)](https://www.npmjs.com/package/node-rfc)
[![downloads](https://img.shields.io/github/downloads/sap/node-rfc/total.svg)](https://www.npmjs.com/package/node-rfc)

## Features

-   Based on [N-API](https://github.com/nodejs/node-addon-api) standard
-   Stateless and stateful connections (multiple function calls in the same ABAP session (same context))
-   Async/await, promise and callback API
-   Sequential and parallel calls, using one or more clients
-   Automatic conversion between JavaScript and ABAP datatypes
-   Decimal and Date objects support
-   Connection pool
-   Extensive unit tests

## Prerequisites

SAP NW RFC SDK C++ binaries must be downloaded (SAP partner or customer account is required) and locally installed. More information on [SAP NW RFC SDK section on SAP Support Portal](https://support.sap.com/en/product/connectors/nwrfcsdk.html) and [node-rfc documentation](http://sap.github.io/node-rfc/install.html#sap-nw-rfc-library-installation).

SAP NW RFC Library is fully backwards compatible, supporting all NetWeaver systems, from today S4, down to R/3 release 4.0B. Using the latest version is reccomended.

The macOS firewall stealth mode must be disabled ([Can't ping a machine - why?](https://discussions.apple.com/thread/2554739)):

```shell
sudo /usr/libexec/ApplicationFirewall/socketfilterfw --setstealthmode on
```

## Supported platforms

Compiled binaries are provided for [active nodejs LTS releases](https://github.com/nodejs/LTS), for 64 bit Windows 7.1 and Ubuntu 16.04 Linux platforms.

Build from source is required on other platforms, supported both by [SAP](https://launchpad.support.sap.com/#/notes/2573790) and by [nodejs](https://github.com/nodejs/node/blob/master/BUILDING.md).

## Usage and API

**Note:** the module must be [installed](#installation) before use.

In order to call remote enabled ABAP function module, we need to create a client
with valid logon credentials, connect to SAP ABAP NetWeaver system and then invoke a
remote enabled ABAP function module from nodejs. The client can be used for one or more subsequent RFC calls and for more examples check unit tests.

Callback API example below shows basic principles. See also:

-   [**Examples and API**](examples/README.md)

-   [**node-rfc documentation**](http://sap.github.io/node-rfc), complementing SAP NW RFC Library [programming guide and documentation](https://support.sap.com/en/products/connectors/nwrfcsdk.html)

```javascript
'use strict';

const rfcClient = require('node-rfc').Client;

// ABAP system RFC connection parameters
const abapSystem = {
	user: 'demo',
	passwd: 'welcome',
	ashost: '10.68.104.164',
	sysnr: '00',
	client: '620',
	lang: 'EN',
};

// create new client
const client = new rfcClient(abapSystem);

// echo SAP NW RFC SDK and nodejs/RFC binding version
console.log('Client version: ', client.version);

// open connection
client.connect(function(err) {
	if (err) {
		// check for login/connection errors
		return console.error('could not connect to server', err);
	}

	// invoke ABAP function module, passing structure and table parameters

	// ABAP structure
	const structure = {
		RFCINT4: 345,
		RFCFLOAT: 1.23456789,
		// or RFCFLOAT: require('decimal.js')('1.23456789'), // as Decimal object
		RFCCHAR4: 'ABCD',
		RFCDATE: '20180625', // in ABAP date format
		// or RFCDATE: new Date('2018-06-25'), // as JavaScript Date object
	};

	// ABAP table
	let table = [structure];

	client.invoke('STFC_STRUCTURE', { IMPORTSTRUCT: structure, RFCTABLE: table }, function(err, res) {
		if (err) {
			return console.error('Error invoking STFC_STRUCTURE:', err);
		}
		console.log('STFC_STRUCTURE call result:', res);
	});
});
```

Finally, the connection is closed automatically when the instance is deleted by the garbage collector or by explicitly calling the `client.close()` method on the client instance.

## Installation

```shell
yarn add node-rfc
```

or if you prefer npm:

```shell
npm install node-rfc
```

Build from the latest source:

```shell
git clone -b https://github.com/SAP/node-rfc.git
cd node-rfc
npm install
node-pre-gyp configure build
# set connection properties in test/abapSystem
npm run test
```

## Issues

-   [NAPI Type checks #265](https://github.com/nodejs/node-addon-api/issues/265)

## Links

-   https://support.sap.com/connectors
-   https://wiki.scn.sap.com/wiki/display/ABAPConn/ABAP+Connectivity+-+RFC
-   [SAP HANA Cloud Connector](https://help.hana.ondemand.com/help/frameset.htm?e6c7616abb5710148cfcf3e75d96d596.html)

## Developer resources

-   [Embedder's Guide](https://github.com/v8/v8/wiki/Embedder's%20Guide)
-   [v8 API docs](https://v8docs.nodesource.com/)
-   [N-API API docs](https://nodejs.github.io/node-addon-api/index.html)
-   [Node.js ES2015 Support](http://node.green/)
-   [Node.js LTS Releases](https://github.com/nodejs/LTS)

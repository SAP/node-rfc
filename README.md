## :heavy_exclamation_mark: Experimental N-API Port :heavy_exclamation_mark:

**See [open issues](#issues).**

[![license](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)
![release](https://img.shields.io/npm/v/node-rfc.svg)
![downloads](https://img.shields.io/github/downloads/sap/node-rfc/total.svg)

# The nodejs SAP NW RFC Connector

Asynchronous, non-blocking [SAP NetWeawer RFC Library](http://service.sap.com/rfc-library) client bindings for [Node.js](http://nodejs.org/):

-   Based on the latest nodejs [N-API](https://github.com/nodejs/node-addon-api) standard
-   Promise and callback interface
-   Connections pool (in progress)
-   Sequential and parallel calls, using single or multiple node-rfc cliens
-   Automatic conversion between JavaScript and ABAP datatypes
-   Decimal objects support
-   Extensive unit tests

## Prerequisites

SAP NW RFC Library must be locally installed. It can be downloaded from SAP Service Marketplace [Software Download Center](https://support.sap.com/swdc), following instrunctions in [SAP Note 2573790](https://websmp208.sap-ag.de/sap/support/notes/2573790). SAP partner or customer is required for download.

SAP NW RFC Library is fully backwards compatible, supporting all NetWeaver systems, from today S4, down to R/3 release 4.0. Using the latest version is reccomended.

## Supported platforms

Compiled binaries are provided for [active nodejs LTS releases](https://github.com/nodejs/LTS), for 64 bit Windows and Linux platforms.

OS X and ARM platforms are currently not supported, as _SAP NW RFC Library_ is not available for those platforms.

## Usage

**Note:** the module must be [installed](#install) before use.

In order to call remote enabled ABAP function module, we need to create a client
with valid logon credentials, connect to NetWeaver system and then invoke a
remote enabled ABAP function module from nodejs. The client can be used for one or more subsequent RFC calls.

```javascript
'use strict';

var rfc = require('node-rfc');

var abapSystem = {
	user: 'name',
	passwd: 'password',
	ashost: '10.11.12.13',
	sysnr: '00',
	client: '100',
};

// create new client
var client = new rfc.Client(abapSystem);

// echo the client NW RFC lib version
console.log('RFC client lib version: ', client.getVersion());

// and connect
client.connect(function(err) {
	if (err) {
		// check for login/connection errors
		return console.error('could not connect to server', err);
	}

	// invoke remote enabled ABAP function module
	client.invoke('STFC_CONNECTION', { REQUTEXT: 'H€llö SAP!' }, function(err, res) {
		if (err) {
			// check for errors (e.g. wrong parameters)
			return console.error('Error invoking STFC_CONNECTION:', err);
		}

		// work with result;  should be something like:
		// { ECHOTEXT: 'Hello SAP!',
		//   RESPTEXT: 'SAP R/3 Rel. 702   Sysid: E1Q      Date: 20140613   Time: 142530   Logon_Data: 001/DEMO/E',
		//   REQUTEXT: 'Hello SAP!' }
		console.log('Result STFC_CONNECTION:', res);
	});

	// invoke more complex ABAP function module
	var importStruct = {
		RFCFLOAT: 1.23456789,
		RFCCHAR1: 'A',
		RFCCHAR2: 'BC',
		RFCCHAR4: 'DEFG',

		RFCINT1: 1,
		RFCINT2: 2,
		RFCINT4: 345,

		RFCHEX3: 'fgh',

		RFCTIME: '121120',
		RFCDATE: '20140101',

		RFCDATA1: '1DATA1',
		RFCDATA2: 'DATA222',
	};

	var importTable = [importStruct];

	client.invoke('STFC_STRUCTURE', { IMPORTSTRUCT: importStruct, RFCTABLE: importTable }, function(err, res) {
		if (err) {
			return console.error('Error invoking STFC_STRUCTURE:', err);
		}
		console.log('Result STFC_STRUCTURE:', res);
	});
});
```

Finally, the connection is closed automatically when the instance is deleted by the garbage collector or by explicitly calling the `client.close()` method on the client instance.

For more examples check the unit tests source code.

The same example with promises:

```javascript
'use strict';

let rfc = require('node-rfc');

let abapSystem = {
	user: 'name',
	passwd: 'password',
	ashost: '10.11.12.13',
	sysnr: '00',
	client: '100',
};

// create new client
let client = rfc.Client.new(abapSystem);

// echo the client NW RFC lib version
console.log('RFC client lib version: ', client.getVersion());

// and connect
client.open

	.then(() => {
		client
			.call('STFC_CONNECTION', { REQUTEXT: 'H€llö SAP!' })

			.then(res => {
				console.log('Result STFC_CONNECTION:', res);
			})

			.catch(err => {
				return console.error('Error invoking STFC_CONNECTION:', err);
			});

		let importStruct = {
			RFCFLOAT: 1.23456789,
			RFCCHAR1: 'A',
			RFCCHAR2: 'BC',
			RFCCHAR4: 'DEFG',

			RFCINT1: 1,
			RFCINT2: 2,
			RFCINT4: 345,

			RFCHEX3: 'fgh',

			RFCTIME: '121120',
			RFCDATE: '20140101',

			RFCDATA1: '1DATA1',
			RFCDATA2: 'DATA222',
		};

		let importTable = [importStruct];

		client
			.call('STFC_STRUCTURE', { IMPORTSTRUCT: importStruct, RFCTABLE: importTable })
			.then(res => {
				console.log('Result STFC_STRUCTURE:', res);
			})
			.catch(err => {
				return console.error('Error invoking STFC_STRUCTURE:', err);
			});
	})

	.catch(err => {
		return console.error('could not connect to server', err);
	});
```

## API and documentation

For API And full documentation please refer to [_node-rfc_ documentation](http://sap.github.io/node-rfc), complementing _SAP NW RFC Library_ [programming guide and documentation](http://service.sap.com/rfc-library).

Useful links:

-   https://service.sap.com/connectors

-   https://wiki.scn.sap.com/wiki/display/ABAPConn/ABAP+Connectivity+-+RFC

-   [SAP HANA Cloud Connector](https://help.hana.ondemand.com/help/frameset.htm?e6c7616abb5710148cfcf3e75d96d596.html)

Developer resources:

-   [Embedder's Guide](https://github.com/v8/v8/wiki/Embedder's%20Guide)
-   [v8 API docs](https://v8docs.nodesource.com/)
-   [N-API API docs](https://nodejs.github.io/node-addon-api/index.html)
-   [Node.js ES2015 Support](http://node.green/)
-   [Node.js LTS Releases](https://github.com/nodejs/LTS)

## Install

To install and use:

```shell
npm install node-rfc
```

To test and develop, clone the repository, edit your backend system connection parameters, build and run tests locally:

```shell
git clone https://github.com/SAP/node-rfc.git
cd node-rfc
npm install
make test
```

Pre-compiled binaries for currently active nodejs LTS releases are provided in the [lib](https://github.com/SAP/node-rfc/tree/master/lib) folder.

## For Developers

-   [Embedder's Guide](https://github.com/v8/v8/wiki/Embedder's%20Guide)
-   [N-API enabled modules](https://github.com/nodejs/abi-stable-node)
-   [N-API API docs](https://nodejs.github.io/node-addon-api/index.html)
-   [v8 API docs](https://v8docs.nodesource.com/)
-   [Node.js ES2015 Support](http://node.green/)

## Issues

-   [Type checks](https://github.com/nodejs/node-addon-api/issues/265)

## :heavy_exclamation_mark: N-API based prerelease :heavy_exclamation_mark:

[![license](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)
[![release](https://img.shields.io/npm/v/node-rfc.svg)](https://www.npmjs.com/package/node-rfc)
[![downloads](https://img.shields.io/github/downloads/sap/node-rfc/total.svg)](https://www.npmjs.com/package/node-rfc)

## Issues

-   [NAPI Type checks #265](https://github.com/nodejs/node-addon-api/issues/265)
-   [node-pre-gyp installation #367](https://github.com/mapbox/node-pre-gyp/issues/367)

## Features

Asynchronous, non-blocking [SAP NetWeawer RFC Library](https://support.sap.com/en/products/connectors/nwrfcsdk.html) client bindings for [Node.js](http://nodejs.org/):

-   Based on the latest nodejs [N-API](https://github.com/nodejs/node-addon-api) standard
-   Promise and callback interface
-   Connection pool
-   Sequential and parallel calls, using single or multiple node-rfc clients
-   Automatic conversion between JavaScript and ABAP datatypes
-   Decimal objects support
-   Extensive unit tests

## Installation

From npm:

```shell
npm install node-rfc@next
```

Build from the latest source:

```shell
git clone -b napi https://github.com/SAP/node-rfc.git
cd node-rfc
npm install
node-pre-gyp configure build
npm run test # adapt test/connParams
```

## Prerequisites

SAP NW RFC Library must be locally installed and for download and installation information check the [Download and Documentation](https://support.sap.com/en/products/connectors/nwrfcsdk.html#section_1291717368) section of (SAP NW RFC SDK Support Portal)[https://support.sap.com/en/products/connectors/nwrfcsdk.html].SAP partner or customer is required for download.

SAP NW RFC Library is fully backwards compatible, supporting all NetWeaver systems, from today S4, down to R/3 release 4.0. Using the latest version is reccomended.

## Supported platforms

Compiled binaries are provided for [active nodejs LTS releases](https://github.com/nodejs/LTS), for 64 bit Windows and Linux platforms.

OS X and ARM platforms are currently not supported, as _SAP NW RFC Library_ is not available for those platforms.

## Usage

**Note:** the module must be [installed](#installation) before use.

In order to call remote enabled ABAP function module, we need to create a client
with valid logon credentials, connect to SAP ABAP NetWeaver system and then invoke a
remote enabled ABAP function module from nodejs. The client can be used for one or more subsequent RFC calls.

Callback, promise and async/await programming examples are provided here and for more check unit tests.

### Callback

```javascript
'use strict';

const rfcClient = require('node-rfc').Client;

const abapSystem = require('./abapSystem');

// create new client
const client = new rfcClient(abapSystem);

// echo the node-rfc client and SAP NW RFC SDK version
console.log('RFC client lib version: ', client.version);

// open connection

client.connect(function(err) {
	if (err) {
		// check for login/connection errors
		return console.error('could not connect to server', err);
	}

	//
	// invoke remote enabled ABAP function module
	//

	client.invoke('STFC_CONNECTION', { REQUTEXT: 'H€llö SAP!' }, function(err, res) {
		if (err) {
			// check for errors (e.g. wrong parameters)
			return console.error('Error invoking STFC_CONNECTION:', err);
		}
		// result should be something like:
		// { ECHOTEXT: 'Hello SAP!',
		//   RESPTEXT: 'SAP R/3 Rel. 702   Sysid: E1Q      Date: 20140613   Time: 142530   Logon_Data: 001/DEMO/E',
		//   REQUTEXT: 'Hello SAP!' }
		console.log('STFC_CONNECTION call result:', res);
	});

	//
	// invoke more complex ABAP function module
	//

	// ABAP structure
	const importStruct = {
		RFCFLOAT: 1.23456789,
		RFCCHAR4: 'DEFG',
		RFCINT4: 345,
	};

	// ABAP table
	let importTable = [importStruct];

	client.invoke('STFC_STRUCTURE', { IMPORTSTRUCT: importStruct, RFCTABLE: importTable }, function(err, res) {
		if (err) {
			return console.error('Error invoking STFC_STRUCTURE:', err);
		}
		console.log('STFC_STRUCTURE call result:', res);
	});

	//
	// invoke possibly longer running ABAP function module, returning more data
	//
	let COUNT = 50000;
	client.invoke(
		'STFC_PERFORMANCE',
		{ CHECKTAB: 'X', LGET0332: COUNT.toString(), LGET1000: COUNT.toString() },

		function(err, res) {
			if (err) {
				return err;
			}
			console.log('ETAB0332 records count:', res.ETAB0332.length);
			console.log('ETAB1000 records count:', res.ETAB1000.length);
		}
	);
});
```

Finally, the connection is closed automatically when the instance is deleted by the garbage collector or by explicitly calling the `client.close()` method on the client instance.

### Promises

```javascript
'use strict';

const rfcClient = require('node-rfc').Client;

const abapSystem = require('./abapSystem');

const client = new rfcClient(abapSystem);

client
	.open()
	.then(() => {
		client
			.call('STFC_CONNECTION', { REQUTEXT: 'H€llö SAP!' })
			.then(res => {
				// process results, reuse for the next RFC call
				res.ECHOTEXT += '#';
				return new Promise(resolve => resolve(res));
			})
			.then(res => {
				client
					.call('STFC_CONNECTION', { REQUTEXT: res.ECHOTEXT })
					.then(res => {
						console.log('STFC_CONNECTION call result:', res.ECHOTEXT);
					})
					.catch(err => {
						console.error('Error invoking STFC_CONNECTION:', err);
					});
			})
			.catch(err => {
				console.error('Error invoking STFC_CONNECTION:', err);
			});
	})
	.catch(err => {
		console.error('could not connect to server', err);
	});
```

### async/await

```javascript
'use strict';

const rfcClient = require('node-rfc').Client;

const abapSystem = require('./abapSystem');

const client = new rfcClient(abapSystem);

async function getData() {
	await client.open(); // remove await to test the catch block below

	let result = await client.call('STFC_CONNECTION', { REQUTEXT: 'H€llö SAP!' });

	result.ECHOTEXT += '#';

	result = await client.call('STFC_CONNECTION', { REQUTEXT: result.ECHOTEXT });

	return result;
}

(async () => {
	let result;
	try {
		result = await getData();
		console.log(result.ECHOTEXT); // 'H€llö SAP!#'
	} catch (ex) {
		console.log(ex);
	}
})();
```

### Connection Pool

```javascript
'use strict';

const rfcClient = require('node-rfc').Pool;

const abapSystem = require('./abapSystem');

const pool = new Pool(abapSystem);

pool.acquire()
	.then(client => {
		client
			.call('STFC_CONNECTION', { REQUTEXT: 'H€llö SAP!' })
			.then(res => {
				console.log('STFC_CONNECTION call result:', res.ECHOTEXT);
				console.log(pool.status);
				pool.release(client);
				console.log(pool.status);
			})
			.catch(err => {
				console.error('Error invoking STFC_CONNECTION:', err);
			});
	})
	.catch(err => {
		console.error('could not acquire connection', err);
	});
```

## API and documentation

For API And full documentation please refer to [_node-rfc_ documentation](http://sap.github.io/node-rfc), complementing _SAP NW RFC Library_ [programming guide and documentation](https://support.sap.com/en/products/connectors/nwrfcsdk.html).

Useful links:

-   https://support.sap.com/connectors

-   https://wiki.scn.sap.com/wiki/display/ABAPConn/ABAP+Connectivity+-+RFC

-   [SAP HANA Cloud Connector](https://help.hana.ondemand.com/help/frameset.htm?e6c7616abb5710148cfcf3e75d96d596.html)

Developer resources:

-   [Embedder's Guide](https://github.com/v8/v8/wiki/Embedder's%20Guide)
-   [v8 API docs](https://v8docs.nodesource.com/)
-   [N-API API docs](https://nodejs.github.io/node-addon-api/index.html)
-   [Node.js ES2015 Support](http://node.green/)
-   [Node.js LTS Releases](https://github.com/nodejs/LTS)

## For Developers

-   [Embedder's Guide](https://github.com/v8/v8/wiki/Embedder's%20Guide)
-   [N-API enabled modules](https://github.com/nodejs/abi-stable-node)
-   [N-API API docs](https://nodejs.github.io/node-addon-api/index.html)
-   [v8 API docs](https://v8docs.nodesource.com/)
-   [Node.js ES2015 Support](http://node.green/)

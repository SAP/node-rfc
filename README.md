The nodejs RFC Connector
========================

This node module provides bindings for SAP NetWeawer RFC Library, for a comfortable way of calling ABAP modules from nodejs, via SAP Remote Function Call (RFC) protocol.

Platforms & Prerequisites
-------------------------

Compiled binaries are provided for [active nodejs LTS releases](https://github.com/nodejs/LTS), for 64 bit Windows and Linux platforms.

OS X and ARM platforms are currently not supported, as _SAP NW RFC Library_ is not available for those platforms.

To start using _node-rfc_ you need to obtain the _SAP NW RFC Library_ from the _SAP Service Marketplace_ [Software Download Center](https://support.sap.com/swdc), 
following [these instructions](http://sap.github.io/node-rfc/install.html#sap-nw-rfc-library-installation).

A prerequisite to download is having a **customer or partner account** on _SAP Service Marketplace_ and if you are SAP employee check SAP OSS note [1037575 - Software download authorizations for SAP employees](http://service.sap.com/sap/support/notes/1037575).

_SAP NW RFC Library_ is fully backwards compatible, supporting all NetWeaver systems, from today, down to release R/3 4.0. You can use the newest version released on Service Marketplace and connect to older systems as well.

Version
-------

The latest supported nodejs version is in master branch, for older versions check other branches.

Documentation
-------------

For full documentation please refer to [_node-rfc_ documentation](http://sap.github.io/node-rfc), complementing _SAP NW RFC Library_ [programming guide and documentation](http://service.sap.com/rfc-library)
provided on SAP Service Marketplace.

Useful links:

* https://service.sap.com/connectors

* https://wiki.scn.sap.com/wiki/display/ABAPConn/ABAP+Connectivity+-+RFC

* [SAP HANA Cloud Connector](https://help.hana.ondemand.com/help/frameset.htm?e6c7616abb5710148cfcf3e75d96d596.html)

Developer resources:

* [nan](https://github.com/nodejs/nan) and [related documentation](https://github.com/nodejs/nan#api)
* libuv [documentation](http://docs.libuv.org/) and [book](http://nikhilm.github.io/uvbook/index.html)

Install
-------

To install and use:

```shell
npm install node-rfc
```

To test and develop, clone the repository, edit your backend system connection parameters, build and run tests locally:

```shell
git clone https://github.com/SAP/node-rfc.git
cd node-rfc
npm install
npm test
```

Pre-compiled binaries for currently active nodejs LTS releases are provided in the [lib](https://github.com/SAP/node-rfc/tree/master/lib) folder.

Getting started
---------------

In order to call remote enabled ABAP function module, we need to create a client
with valid logon credentials, connect to NetWeaver system and then invoke a
remote enabled ABAP function module from node. The client can be used for one or
more subsequent RFC calls.

```javascript
"use strict";

var rfc = require('node-rfc');

var abapSystem = {
  user: 'name',
  passwd: 'password',
  ashost: '10.11.12.13',
  sysnr: '00',
  client: '100',
  saprouter: '/H/111.22.33.177/S/3299/W/tdkf9d/H/132.139.17.14/H/'
};

// create new client
var client = new rfc.Client(abapSystem);

// echo the client NW RFC lib version
console.log('RFC client lib version: ', client.getVersion());

// and connect
client.connect(function(err) {
  if (err) { // check for login/connection errors
    return console.error('could not connect to server', err);
  }

  // invoke remote enabled ABAP function module
  client.invoke('STFC_CONNECTION',
    { REQUTEXT: 'H€llö SAP!' },
    function(err, res) {
      if (err) { // check for errors (e.g. wrong parameters)
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
    RFCDATA2: 'DATA222'
  };

  var importTable = [importStruct];

  client.invoke('STFC_STRUCTURE',
    { IMPORTSTRUCT: importStruct, RFCTABLE: importTable },
    function(err, res) {
      if (err) {
        return console.error('Error invoking STFC_STRUCTURE:', err);
      }
      console.log('Result STFC_STRUCTURE:', res);
  });

});
```

Finally, the connection is closed automatically when the instance is deleted by the garbage collector or by explicitly calling the `client.close()` method on the client instance.

[r3connect](https://github.com/hundeloh-consulting/r3connect) wrapper makes the node-rfc consumption even more comfortable,
offering promise-based API and connections pool capabilities.

For more examples check the unit tests source code. Maintain your NW test system parameters first in the source code, before running those examples.


Running the Unit Tests
----------------------

To run the unit tests, first ensure that you have followed the [Build from Source](http://sap.github.io/node-rfc/install.html#building-from-source) documentation, 
in order to install all dependencies and successfully build the node-rfc connector. 
Once you have done that, ensure that [mocha](https://mochajs.org) and [should](https://github.com/shouldjs/should.js) are installed, either as dependencies:

```shell
npm install
```

or globally:

```shell
npm install -g mocha should
```

Run the tests with:

```shell
mocha
```

REST API
--------

Example how to create REST APIs using node-rfc, node, express and gulp: https://github.com/Adracus/noderfc-restapi.

Links
-----

Nodejs Addons

Getting Started with Embedding https://github.com/v8/v8/wiki/Getting%20Started%20with%20Embedding

Maintainer Information
-----
Version is found/has to be changed in package.json.
There are two origins with the same repository: https://github.com/SAP/node-rfc and https://github.wdf.sap.corp/D037732/node-rfc.

### Continuous Integration

The project is built using Travis (.travis.yml) for GNU/Linux and the build.sh script for Windows.
Npm releases are managed by Travis.

The repository to publish to is specified in package.json: binary.host

#### GNU/Linux (Travis)
To publish binaries add prefix: [publish binary] to commit.

* Target node versions can be specified in .travis.yml: node_js
* The npm api key is found in .travis.yml: deploy.api_key
* The github access token is located in .travis.yml: env.global
* The build repository is specified in package.json: repository.url


#### Windows (<span>build.sh</span>)
Setup the machine like described here: http://sap.github.io/node-rfc/install.html

Node, npm and either nvm or nodist have to be installed.
To build/publish execute <span>build.sh</span>.
If test results shall be ignored use the flag -i.
Target node versions are specified in top of the file in the array "NODE_VERSIONS".


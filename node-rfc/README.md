The nodejs RFC Connector
========================

This node module provides bindings for SAP NetWeawer RFC Library, for a comfortable way of calling ABAP modules from nodejs, via SAP Remote Function Call (RFC) protocol.

[![NPM](https://nodei.co/npm/through.png?compact=true)](https://nodei.co/npm/node-rfc/)

Platforms & Prerequisites
-------------------------

The _node-rfc_ has been initially built with nodejs v0.10.26, on Linux 64 bit platform and later enhanced, mostly used and tested on Linux and Windows 64 platforms.

OS X and ARM platforms are currently not supported, as _SAP NW RFC Library_ is not available for those platforms.

To start using _node-rfc_ you need to obtain _SAP NW RFC Library_ from _SAP Service Marketplace_, following [these instructions](http://sap.github.io/PyRFC/install.html#install-c-connector).

A prerequisite to download is having a **customer or partner account** on _SAP Service Marketplace_ and if you are SAP employee please check SAP OSS note [1037575 - Software download authorizations for SAP employees](http://service.sap.com/sap/support/notes/1037575).

_SAP NW RFC Library_ is fully backwards compatible, supporting all NetWeaver systems, from today, down to release R/3 4.0.
You can therefore always use the newest version released on Service Marketplace and connect to older systems as well.

Documentation
-------------

For full documentation please refer to [_node-rfc_ documentation](http://sap.github.io/node-rfc), complementing _SAP NW RFC Library_ [programming guide and documentation](http://service.sap.com/rfc-library)
provided on SAP Service Marketplace.


Install
-------

Install from npm

```
npm install node-rfc
```

or clone from the [GitHub repository](https://github.com/SAP/node-rfc.git) to build and run tests and examples locally

```
git clone https://github.com/SAP/node-rfc.git
cd node-rfc
npm install
```

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
var client = new rfc.Client(abapSystem, true);

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

For more examples, check files in the `demo` folder. Maintain your NW test system parameters first in the source code, before running those examples.

```
node demo\demo
node demo\demo1, 2 ...
```

REST API
--------

Example how to create REST APIs using node-rfc, node, express and gulp: https://github.com/Adracus/noderfc-restapi.



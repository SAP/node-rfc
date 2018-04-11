// Copyright 2014 SAP AG.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http: //www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific
// language governing permissions and limitations under the License.

"use strict";

var should = require('should');
var binary = require('node-pre-gyp');
var path = require('path');
var rfc_path = binary.find(path.resolve(path.join(__dirname,'../package.json')));
console.log(rfc_path)
var rfc = require(rfc_path);

var connParams = {
  user: 'demo',
  passwd: 'Welcome',
  ashost:  '10.117.19.101',
  saprouter: '/H/203.13.155.17/W/xjkb3d/H/172.19.138.120/H/',
  sysnr: '00',
  lang: 'EN',
  client: '100'
}

describe("Connection", function() {
  var client;
  beforeEach(function(done) {
    client = new rfc.Client(connParams);
    client.connect(function() { setTimeout(function() { done(); }, 0); });
  });

  afterEach(function() {
    client.close();
  });

  it('connectionInfo() should return connection information', function() {
    var info = client.connectionInfo();
    info.should.have.properties('dest', 'host', 'partnerHost', 'sysNumber', 'sysId',
      'client', 'user', 'language', 'trace', 'isoLanguage', 'codepage', 'partnerCodepage',
      'rfcRole', 'type', 'partnerType', 'rel', 'partnerRel', 'kernelRel', 'cpicConvId',
      'progName', 'partnerBytesPerChar', 'reserved');
    info.should.have.properties({
      user: connParams.user.toUpperCase(),
      sysNumber: connParams.sysnr,
      client: connParams.client
    });
  });
  it('ping should return true when connected', function() {
    client.ping().should.be.true;
  });
  it('isAlive() should return true when connected', function() {
    client.isAlive().should.be.true;
  });


  it('STFC_CONNECTION should work in parallel', function(done) {
    var count = 5; //10
    var j = count;
    for (var i = 0; i < count; i++) {
      client.invoke('STFC_CONNECTION', { REQUTEXT: 'Hello SAP! ' + i },
        function(err, res) {
          should.not.exist(err);
          should.exist(res);
          res.should.be.an.Object;
          res.should.have.property('ECHOTEXT');
          res.ECHOTEXT.should.startWith('Hello SAP!');
          if (--j === 0)
						done();
        });
    }
  });

	it('STFC_CONNECTION should run sequentially', function(done) {
		var iterations = 10;
		var rec = function(depth) {
			if (depth == iterations) {
				done();
				return;
			}
      client.invoke('STFC_CONNECTION', { REQUTEXT: 'Hello SAP! ' + depth },
        function(err, res) {
          should.not.exist(err);
          should.exist(res);
          res.should.be.an.Object;
          res.should.have.property('ECHOTEXT');
          res.ECHOTEXT.should.startWith('Hello SAP! ' + depth);
					rec(depth+1);
        });
		}
		rec(0);
	});


  it('STFC_CONNECTION should return "Hello SAP!" string', function(done) {
    client.invoke('STFC_CONNECTION', { REQUTEXT: 'Hello SAP!' },
      function(err, res) {
        should.not.exist(err);
        should.exist(res);
        res.should.be.an.Object;
        res.should.have.property('ECHOTEXT');
        res.ECHOTEXT.should.startWith('Hello SAP!');
        done();
      });
  });

  it('STFC_CONNECTION should return Umlauts "H€llö SAP!" string', function(done) {
    client.invoke('STFC_CONNECTION', { REQUTEXT: 'H€llö SAP!' },
      function(err, res) {
        should.not.exist(err);
        should.exist(res);
        res.should.be.an.Object;
        res.should.have.property('ECHOTEXT');
        res.ECHOTEXT.should.startWith('H€llö SAP!');
        done();
      });
  });

  it('async test', function(done) {
    var asyncRes = undefined;
    client.invoke('STFC_CONNECTION', { REQUTEXT: 'Hello SAP!' },
      function(err, res) {
        should.not.exist(err);
        should.exist(res);
        res.should.be.an.Object;
        res.should.have.property('ECHOTEXT');
        res.ECHOTEXT.should.startWith('Hello SAP!');
        asyncRes = res;
        done();
      });
    should.not.exist(asyncRes);
  });

  it('STFC_STRUCTURE should return structure and table', function(done) {
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

      should.not.exist(err);
      should.exist(res);
      res.should.be.an.Object;
      res.should.have.properties('ECHOSTRUCT', 'RFCTABLE');

      res.ECHOSTRUCT.RFCCHAR1.should.equal(importStruct.RFCCHAR1);
      res.ECHOSTRUCT.RFCCHAR2.should.equal(importStruct.RFCCHAR2);
      res.ECHOSTRUCT.RFCCHAR4.should.equal(importStruct.RFCCHAR4);
      res.ECHOSTRUCT.RFCFLOAT.should.equal(importStruct.RFCFLOAT);
      res.ECHOSTRUCT.RFCINT1.should.equal(importStruct.RFCINT1);
      res.ECHOSTRUCT.RFCINT2.should.equal(importStruct.RFCINT2);
      res.ECHOSTRUCT.RFCINT4.should.equal(importStruct.RFCINT4);
      res.ECHOSTRUCT.RFCDATA1.should.startWith(importStruct.RFCDATA1);
      res.ECHOSTRUCT.RFCDATA2.should.startWith(importStruct.RFCDATA2);

      res.RFCTABLE.should.have.length(2);
      res.RFCTABLE[1].RFCFLOAT.should.equal(importStruct.RFCFLOAT+1);
      res.RFCTABLE[1].RFCINT1.should.equal(importStruct.RFCINT1+1);
      res.RFCTABLE[1].RFCINT2.should.equal(importStruct.RFCINT2+1);
      res.RFCTABLE[1].RFCINT4.should.equal(importStruct.RFCINT4+1);

      done();
    });
  });

  it('INT type check should detect strings', function(done) {
    var importStruct = {

      RFCINT1: "1",
      RFCINT2: 2,
      RFCINT4: 345

    };
    var importTable = [importStruct];
    client.invoke('STFC_STRUCTURE',
      { IMPORTSTRUCT: importStruct, RFCTABLE: importTable },
      function(err) {
        should.exist(err);
        err.should.be.an.Object;
        err.should.have.properties({
          message: "Number expected when filling field RFCINT1 of type 10"
        });
        done();
      });
  });
  
  it('INT type check should detect floats', function(done) {
    var importStruct = {

      RFCINT1: 1,
      RFCINT2: 2,
      RFCINT4: 3.1

    };
    var importTable = [importStruct];
    client.invoke('STFC_STRUCTURE',
      { IMPORTSTRUCT: importStruct, RFCTABLE: importTable },
      function(err) {
        should.exist(err);
        err.should.be.an.Object;
        err.should.have.properties({
          message: "Number expected when filling field RFCINT4 of type 8"
        });
        done();
      });
  });
});

describe("More complex RFCs", function() {
  var client;
  beforeEach(function(done) {
    client = new rfc.Client(connParams);
    client.connect(function() { setTimeout(function() { done(); }, 0); });
  });

  afterEach(function() {
    client.close();
  });

  it('Invoke BAPI_USER_GET_DETAIL', function(done) {
    this.timeout(15000);
    client.invoke('BAPI_USER_GET_DETAIL', { USERNAME: 'DEMO' },
      function(err, res) {
        should.not.exist(err);
        res.should.be.an.Object;
        res.should.have.properties(
          'ADDRESS', 'ACTIVITYGROUPS',
          'DEFAULTS', 'GROUPS','ISLOCKED', 'LOGONDATA',
          'PARAMETER','PROFILES', 'RETURN'
        );
        done();
      });
  });
});

describe("Error handling", function() {
  var client;
  beforeEach(function(done) {
    client = new rfc.Client(connParams);
    client.connect(function() { setTimeout(function() { done(); }, 0); });
  });

  afterEach(function() {
    client.close();
  });

  it('Invoke with wrong parameter should return err RFC_INVALID_PARAMETER', function(done) {
    client.invoke('STFC_CONNECTION', { XXX: 'wrong param' },
      function(err, res) {
        should.exist(err);
        err.should.be.an.Object;
        err.should.have.properties({
          code: 20,
          key: 'RFC_INVALID_PARAMETER',
          message: "field 'XXX' not found"
        });
        done();
      });
  });

  it('RFC_RAISE_ERROR should return error', function(done) {
    client.invoke('RFC_RAISE_ERROR', { MESSAGETYPE: 'A' },
      function(err, res) {
        should.exist(err);
        err.should.be.an.Object;
        err.should.have.properties({
          code: 4,
          key: 'Function not supported',
          abapMsgClass: 'SR',
          abapMsgType: 'A',
          abapMsgNumber: '006',
          message: "Function not supported"
        });
        done();
      });
  });

  it('Logon failure with wrong credentials', function(done) {
    var wrongParams = {};
    for (var attr in connParams) {
      if (connParams.hasOwnProperty(attr)) {
        wrongParams[attr] = connParams[attr];
      }
    }
    wrongParams.user = 'WRONGUSER';

    var wrongClient = new rfc.Client(wrongParams);
    wrongClient.connect(function(err) {
      should.exist(err);
      err.should.have.properties({
        message: "Name or password is incorrect (repeat logon)",
        code: 2,
        key: 'RFC_LOGON_FAILURE'
      });
      done();
    });
    wrongClient.close();
  });

  it('Connection parameter missing', function(done) {
    var wrongParams = {};
    for (var attr in connParams) {
      if (connParams.hasOwnProperty(attr)) {
        wrongParams[attr] = connParams[attr];
      }
    }
    delete wrongParams.ashost;

    var wrongClient = new rfc.Client(wrongParams);
    wrongClient.connect(function(err) {
      should.exist(err);
      err.should.have.properties({
        message: "Parameter ASHOST, GWHOST, MSHOST or SERVER_PORT is missing.",
        code: 20,
        key: 'RFC_INVALID_PARAMETER'
      });
      done();
    });
    wrongClient.close();
  });

  it('No connection parameters provided at all', function(done) {
    try {
      var noClient = new rfc.Client();
    } catch (err) {
      should.exist(err);
      err.should.have.properties({
        message: "Please provide connection parameters as argument"
      });
      done();
    }
  });
});

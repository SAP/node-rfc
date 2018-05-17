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

'use strict';

const rfc = require('../sapnwrfc');
const should = require('should');

const connParams = require('./connParams');

describe('Connection', function() {
    let client;

    before(function(done) {
        client = new rfc.Client(connParams);
        client.connect(function(err) {
            if (err) return done(err);
            done();
        });
    });

    after(function() {
        client.close();
    });

    it('getVersion() should return major, minor, patchLevel and node-rfc', function() {
        let version = rfc.Client.getVersion();
        version.should.have.properties('major', 'minor', 'patchLevel', 'noderfc');
    });

    it('connectionInfo() should return connection information', function() {
        let info = client.connectionInfo();
        info.should.have.properties(
            'host',
            'partnerHost',
            'sysNumber',
            'sysId',
            'client',
            'user',
            'language',
            'trace',
            'isoLanguage',
            'codepage',
            'partnerCodepage',
            'rfcRole',
            'type',
            'partnerType',
            'rel',
            'partnerRel',
            'kernelRel',
            'cpicConvId',
            'progName',
            'partnerBytesPerChar',
            'reserved'
        );
        info.should.have.properties({
            user: connParams.user.toUpperCase(),
            sysNumber: connParams.sysnr,
            client: connParams.client,
        });
    });

    it('isAlive() and ping() should return true when connected', function() {
        client.isAlive().should.be.true;
        client.ping().should.be.true;
    });

    it('isAlive() and ping() should return false after close()', function() {
        client.close();
        client.isAlive().should.be.false;
        client.ping().should.be.false;
    });

    it('reopen() should reopen the connection', function(done) {
        client.isAlive().should.be.false;
        client.reopen(function(err) {
            should.not.exist(err);
            client.isAlive().should.be.true;
            client.ping().should.be.true;
            done();
        });
    });

    it('STFC_CONNECTION should return "Hello SAP!" string', function(done) {
        client.invoke('STFC_CONNECTION', { REQUTEXT: 'Hello SAP!' }, function(err, res) {
            should.not.exist(err);
            should.exist(res);
            res.should.be.an.Object;
            res.should.have.property('ECHOTEXT');
            res.ECHOTEXT.should.startWith('Hello SAP!');
            done();
        });
    });

    it('STFC_CONNECTION should return Umlauts "H€llö SAP!" string', function(done) {
        client.invoke('STFC_CONNECTION', { REQUTEXT: 'H€llö SAP!' }, function(err, res) {
            should.not.exist(err);
            should.exist(res);
            res.should.be.an.Object;
            res.should.have.property('ECHOTEXT');
            res.ECHOTEXT.should.startWith('H€llö SAP!');
            done();
        });
    });

    it('STFC_STRUCTURE should return structure and table', function(done) {
        let importStruct = {
            RFCFLOAT: 1.23456789,
            RFCCHAR1: 'A',
            RFCCHAR2: 'BC',
            RFCCHAR4: 'DEFG',

            RFCINT1: 1,
            RFCINT2: 2,
            RFCINT4: 345,

            RFCHEX3: '\x01\x02\x03', //fgh',

            RFCTIME: '121120',
            RFCDATE: '20140101',

            RFCDATA1: '1DATA1',
            RFCDATA2: 'DATA222',
        };
        let importTable = [importStruct];

        client.invoke('STFC_STRUCTURE', { IMPORTSTRUCT: importStruct, RFCTABLE: importTable }, function(err, res) {
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
            res.ECHOSTRUCT.RFCHEX3.toString().should.equal(importStruct.RFCHEX3.toString());

            res.RFCTABLE.should.have.length(2);
            res.RFCTABLE[1].RFCFLOAT.should.equal(importStruct.RFCFLOAT + 1);
            res.RFCTABLE[1].RFCINT1.should.equal(importStruct.RFCINT1 + 1);
            res.RFCTABLE[1].RFCINT2.should.equal(importStruct.RFCINT2 + 1);
            res.RFCTABLE[1].RFCINT4.should.equal(importStruct.RFCINT4 + 1);
            done();
        });
    });
});

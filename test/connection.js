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

const rfcClient = require('../lib').Client;
const should = require('should');

const abapSystem = require('./abapSystem')('MME');

describe('Connection', function() {
    let client;

    before(function(done) {
        client = new rfcClient(abapSystem);
        client.connect(function(err) {
            if (err) return done(err);
            done();
        });
    });

    after(function() {
        client.close();
    });

    it('sapnwrfc client id check', function(done) {
        client.id.should.be.number;
        client.id.should.be.greaterThan(0);
        let n = client.id;
        try {
            client.id = n + 1;
        } catch (ex) {
            ex.should.have.properties({
                name: 'TypeError',
                message: 'Cannot set property id of #<Client> which has only a getter',
            });
            client.id.should.equal(n);
            done();
        }
    });

    it('sapnwrfc client version check', function(done) {
        client.version.should.be.an.Object();
        client.version.should.have.properties('major', 'minor', 'patchLevel', 'binding');
        try {
            client.version = { a: 1, b: 2 };
        } catch (ex) {
            ex.should.have.properties({
                name: 'TypeError',
                message: 'Cannot set property version of #<Client> which has only a getter',
            });
            done();
        }
    });

    it('VERSION should match binding version and package.json version', function(done) {
        let VERSION = require('fs')
            .readFileSync('VERSION')
            .toString()
            .trim();
        require('../package.json').version.should.be.equal(VERSION);
        client.version.binding.should.equal(VERSION);
        done();
    });

    it('connectionInfo() should return connection information', function(done) {
        client.connectionInfo.should.be.an.Object();
        client.connectionInfo.should.have.properties(
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
        client.connectionInfo.should.have.properties({
            user: abapSystem.user.toUpperCase(),
            sysNumber: abapSystem.sysnr,
            client: abapSystem.client,
        });
        client.close();
        client.connectionInfo.should.be.an.Object().and.be.empty();
        done();
    });

    it('isAlive and ping() should return true when connected', function(done) {
        client.isAlive.should.be.true;
        client.ping().should.be.true;
        done();
    });

    it('isAlive and ping() should return false after close()', function(done) {
        client.close();
        client.isAlive.should.be.false;
        client.ping().should.be.false;
        done();
    });

    it('reopen() should reopen the connection', function(done) {
        client.isAlive.should.be.false;
        client.reopen(function(err) {
            should.not.exist(err);
            client.isAlive.should.be.true;
            let convId = client.connectionInfo.cpicConvId;
            client.reopen(function(err) {
                should.not.exist(err);
                client.isAlive.should.be.true;
                convId.should.not.be.equal(client.connectionInfo.cpicConvId);
                done();
            });
        });
    });

    it('STFC_CONNECTION should return "Hello SAP!" string', function(done) {
        client.invoke('STFC_CONNECTION', { REQUTEXT: 'Hello SAP!' }, function(err, res) {
            should.not.exist(err);
            should.exist(res);
            res.should.be.an.Object();
            res.should.have.property('ECHOTEXT');
            res.ECHOTEXT.should.startWith('Hello SAP!');
            done();
        });
    });

    it('STFC_CONNECTION should return Umlauts "H€llö SAP!" string', function(done) {
        client.invoke('STFC_CONNECTION', { REQUTEXT: 'H€llö SAP!' }, function(err, res) {
            should.not.exist(err);
            should.exist(res);
            res.should.be.an.Object();
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
            res.should.be.an.Object();
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

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

const rfcClient = require('./noderfc').Client;
const abapSystem = require('./abapSystem')();

const should = require('should');

// https://github.com/SAP/node-rfc/issues/57
const UNICODETEST = 'ทดสอบสร้างลูกค้าจากภายนอกครั้งที่ 3'.repeat(7);

describe('Connection', function() {
    let client = new rfcClient(abapSystem);

    beforeEach(function(done) {
        client.reopen(err => {
            done(err);
        });
    });

    afterEach(function(done) {
        client.close(() => {
            done();
        });
    });

    it('VERSION == binding version == package.json version', function(done) {
        let VERSION = require('fs')
            .readFileSync('VERSION')
            .toString()
            .trim();
        require('../package.json').version.should.be.equal(VERSION);
        client.version.binding.should.equal(VERSION);
        done();
    });

    it('exception: Client getters', function(done) {
        client.id.should.be.number;
        client.id.should.be.greaterThan(0);
        client.version.should.be.an.Object();
        client.version.should.have.properties('major', 'minor', 'patchLevel', 'binding');
        client.options.should.be.an.Object();
        client.options.should.have.properties('rstrip', 'bcd');
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
        client.close(err => {
            if (err) return done(err);
            client.connectionInfo.should.be.an.Object().and.be.empty();
            done();
        });
    });

    it('isAlive and ping() should be true when connected', function(done) {
        client.isAlive.should.be.true;
        client.ping((err, res) => {
            if (err) return done(err);
            res.should.be.true;
            done();
        });
    });

    it('isAlive ands ping() should be false when disconnected', function(done) {
        client.close(() => {
            client.isAlive.should.be.false;
            client.ping((err, res) => {
                if (err) return done(err);
                res.should.be.false;
                client.close(err => {
                    return done(err);
                });
            });
        });
    });

    it('reopen() should reopen the connection', function(done) {
        client.isAlive.should.be.true;
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

    it('invoke() STFC_CONNECTION should return unicode string', function(done) {
        client.invoke('STFC_CONNECTION', { REQUTEXT: UNICODETEST }, function(err, res) {
            should.not.exist(err);
            should.exist(res);
            res.should.be.an.Object();
            res.should.have.property('ECHOTEXT');
            res.ECHOTEXT.should.startWith(UNICODETEST);
            done();
        });
    });

    it('invoke() STFC_STRUCTURE should return structure and table', function(done) {
        let importStruct = {
            RFCFLOAT: 1.23456789,
            RFCCHAR1: 'A',
            RFCCHAR2: 'BC',
            RFCCHAR4: 'DEFG',

            RFCINT1: 1,
            RFCINT2: 2,
            RFCINT4: 345,

            RFCHEX3: Buffer.from('\x01\x02\x03', 'ascii'),

            RFCTIME: '121120',
            RFCDATE: '20140101',

            RFCDATA1: '1DATA1',
            RFCDATA2: 'DATA222',
        };
        let INPUTROWS = 10;
        let importTable = [];
        for (let i = 0; i < INPUTROWS; i++) {
            let row = {};
            Object.assign(row, importStruct);
            row.RFCINT1 = i;
            importTable.push(row);
        }

        client.invoke('STFC_STRUCTURE', { IMPORTSTRUCT: importStruct, RFCTABLE: importTable }, function(err, res) {
            should.not.exist(err);
            should.exist(res);
            res.should.be.an.Object();
            res.should.have.properties('ECHOSTRUCT', 'RFCTABLE');

            // ECHOSTRUCT match IMPORTSTRUCT
            for (let k in importStruct) {
                if (k === 'RFCHEX3') {
                    res.ECHOSTRUCT[k].toString().should.equal(importStruct[k].toString());
                } else {
                    res.ECHOSTRUCT[k].should.equal(importStruct[k]);
                }
            }

            // added row is incremented IMPORTSTRUCT
            res.RFCTABLE.should.have.length(INPUTROWS + 1);

            // output table match import table
            for (let i = 0; i < INPUTROWS; i++) {
                let rowIn = importTable[i];
                let rowOut = res.RFCTABLE[i];
                for (let k in rowIn) {
                    if (k === 'RFCHEX3') {
                        rowIn[k].toString().should.equal(rowOut[k].toString());
                    } else {
                        rowIn[k].should.equal(rowOut[k]);
                    }
                }
            }

            // added row match incremented IMPORTSTRUCT
            res.RFCTABLE[INPUTROWS].RFCFLOAT.should.equal(importStruct.RFCFLOAT + 1);
            res.RFCTABLE[INPUTROWS].RFCINT1.should.equal(importStruct.RFCINT1 + 1);
            res.RFCTABLE[INPUTROWS].RFCINT2.should.equal(importStruct.RFCINT2 + 1);
            res.RFCTABLE[INPUTROWS].RFCINT4.should.equal(importStruct.RFCINT4 + 1);

            done();
        });
    });
});

module.exports = UNICODETEST;

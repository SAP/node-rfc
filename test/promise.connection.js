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

const abapSystem = require('./abapSystem')();

describe('[promise] Connection', function() {
    let client;

    before(function(done) {
        client = new rfcClient(abapSystem);
        client
            .open()
            .then(() => {
                done();
            })
            .catch(err => {
                return done(err);
            });
    });

    after(function() {
        client.close();
    });

    it('STFC_CONNECTION should return "Hello SAP!" string', function() {
        return client.call('STFC_CONNECTION', { REQUTEXT: 'Hello SAP!' }).then(res => {
            should.exist(res);
            res.should.be.an.Object();
            res.should.have.property('ECHOTEXT');
            res.ECHOTEXT.should.startWith('Hello SAP!');
        });
    });

    it('STFC_CONNECTION should return Umlauts "H€llö SAP!" string', function() {
        return client.call('STFC_CONNECTION', { REQUTEXT: 'H€llö SAP!' }).then(res => {
            should.exist(res);
            res.should.be.an.Object();
            res.should.have.property('ECHOTEXT');
            res.ECHOTEXT.should.startWith('H€llö SAP!');
        });
    });

    it('STFC_STRUCTURE should return structure and table', function() {
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

        return client.call('STFC_STRUCTURE', { IMPORTSTRUCT: importStruct, RFCTABLE: importTable }).then(res => {
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
        });
    });
});

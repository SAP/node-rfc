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
const should = require('should');

const abapSystem = require('./abapSystem')();

const CONNECTIONS = 50;

describe('Concurrency', function() {
    this.timeout(15000);
    let client = new rfcClient(abapSystem);

    beforeEach(function() {
        if (!client.isAlive) return client.open();
    });

    afterEach(function() {
        if (client.isAlive) return client.close();
    });

    const REQUTEXT = 'Hellö SÄP!';

    it('non-blocking invoke()', function(done) {
        let asyncRes;
        client.invoke('STFC_CONNECTION', { REQUTEXT: REQUTEXT }, function(err, res) {
            should.not.exist(err);
            should.exist(res);
            res.should.be.an.Object();
            res.should.have.property('ECHOTEXT');
            res.ECHOTEXT.should.startWith(REQUTEXT);
            asyncRes = res;
            done();
        });
        should.not.exist(asyncRes);
    });

    it(`concurrency: ${CONNECTIONS} connections invoke()`, function(done) {
        let count = CONNECTIONS;
        let CLIENTS = [];
        for (let i = 0; i < count; i++) {
            CLIENTS.push(new rfcClient(abapSystem));
        }
        for (let client of CLIENTS) {
            client.connect(err => {
                should.not.exist(err);
                client.invoke('STFC_CONNECTION', { REQUTEXT: REQUTEXT }, function(err, res) {
                    should.not.exist(err);
                    should.exist(res);
                    res.should.be.an.Object();
                    res.should.have.property('ECHOTEXT');
                    res.ECHOTEXT.should.startWith(REQUTEXT);
                    client.close();
                    if (--count === 1) done();
                });
            });
        }
    });

    it(`concurrency: 1 connection, ${CONNECTIONS} invoke() calls`, function(done) {
        let count = CONNECTIONS;
        for (let i = count; i > 0; i--) {
            client.invoke('STFC_CONNECTION', { REQUTEXT: REQUTEXT + client.id }, function(err, res) {
                should.not.exist(err);
                should.exist(res);
                res.should.be.an.Object();
                res.should.have.property('ECHOTEXT');
                res.ECHOTEXT.should.startWith(REQUTEXT + client.id);
                if (i === 1) done();
            });
        }
    });

    it(`concurrency: 1 connection ${CONNECTIONS} recursive invoke() calls`, function(done) {
        let rec = function(depth) {
            if (depth < CONNECTIONS) {
                client.invoke('STFC_CONNECTION', { REQUTEXT: REQUTEXT + depth }, function(err, res) {
                    should.not.exist(err);
                    should.exist(res);
                    res.should.be.an.Object();
                    res.should.have.property('ECHOTEXT');
                    res.ECHOTEXT.should.startWith(REQUTEXT + depth);
                    rec(depth + 1);
                });
            } else {
                done();
            }
        };
        rec(0);
    });
});

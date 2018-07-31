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

const should = require('should');

const rfcClient = require('./noderfc').Client;
const abapSystem = require('./abapSystem')();

const CONNECTIONS = 50;
describe('Concurrency callbacks', function() {
    this.timeout(15000);

    let client;

    beforeEach(function(done) {
        new rfcClient(abapSystem)
            .open()
            .then(c => {
                client = c;
                done();
            })
            .catch(err => {
                done(err);
            });
    });

    afterEach(function(done) {
        client.close(() => {
            done();
        });
    });

    const REQUTEXT = 'Hellö SÄP!';

    it('invoke() should not block', function(done) {
        let asyncRes;
        client.invoke('STFC_CONNECTION', { REQUTEXT: REQUTEXT }, function(err, res) {
            if (err) {
                return done(err);
            } else {
                should.exist(res);
                res.should.be.an.Object();
                res.should.have.property('ECHOTEXT');
                res.ECHOTEXT.should.startWith(REQUTEXT);
                asyncRes = res;
                done();
            }
        });
        should.not.exist(asyncRes);
    });

    it(`concurrency: ${CONNECTIONS} parallel connections invoke()`, function(done) {
        let callbackCount = 0;
        let CLIENTS = [];
        for (let i = 0; i < CONNECTIONS; i++) {
            CLIENTS.push(new rfcClient(abapSystem));
        }
        for (let c of CLIENTS) {
            c.connect(err => {
                if (err) return done(err);
                c.invoke('STFC_CONNECTION', { REQUTEXT: REQUTEXT }, function(err, res) {
                    if (err) {
                        return done(err);
                    } else {
                        should.exist(res);
                        res.should.be.an.Object();
                        res.should.have.property('ECHOTEXT');
                        res.ECHOTEXT.should.startWith(REQUTEXT);
                        c.close(() => {
                            if (++callbackCount === CONNECTIONS) done();
                        });
                    }
                });
            });
        }
    });

    it(`concurrency: ${CONNECTIONS} sequential invoke() calls using single connection`, function(done) {
        let callbackCount = 0;
        for (let count = 0; count < CONNECTIONS; count++) {
            client.invoke('STFC_CONNECTION', { REQUTEXT: REQUTEXT + client.id }, function(err, res) {
                if (err) return done(err);
                should.exist(res);
                res.should.be.an.Object();
                res.should.have.property('ECHOTEXT');
                res.ECHOTEXT.should.startWith(REQUTEXT + client.id);
                if (++callbackCount === CONNECTIONS) done();
            });
        }
    });

    it(`concurrency: ${CONNECTIONS} recursive invoke() calls using single connection`, function(done) {
        let callbackCount = 0;
        function call(count) {
            client.invoke('STFC_CONNECTION', { REQUTEXT: REQUTEXT + count }, function(err, res) {
                if (err) {
                    return done(err);
                } else {
                    res.should.be.an.Object();
                    res.should.have.property('ECHOTEXT');
                    res.ECHOTEXT.should.startWith(REQUTEXT + count);
                    if (++callbackCount == CONNECTIONS) {
                        done();
                    } else {
                        call(callbackCount);
                    }
                }
            });
        }
        call(callbackCount);
    });
});

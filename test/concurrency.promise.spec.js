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
const Promise = require('bluebird');

const CONNECTIONS = 50;

describe('Concurrency promises', function() {
    this.timeout(15000);

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
    const REQUTEXT = 'Hellö SÄP!';

    it('concurrency: call() should not block', function(done) {
        let asyncRes;
        let REQUTEXT = 'Hello SAP!';
        client
            .call('STFC_CONNECTION', { REQUTEXT: REQUTEXT })
            .then(res => {
                res.should.be.an.Object();
                res.should.have.property('ECHOTEXT');
                res.ECHOTEXT.should.startWith(REQUTEXT);
                asyncRes = res;
                done();
            })
            .catch(err => {
                return done(err);
            });
        should.not.exist(asyncRes);
    });

    it(`concurrency: ${CONNECTIONS} parallel connections call() promises`, function(done) {
        let callbackCount = 0;
        for (let i = 0; i < CONNECTIONS; i++) {
            let c = new rfcClient(abapSystem);
            c.open()
                .then(() => {
                    c.call('STFC_CONNECTION', { REQUTEXT: REQUTEXT + c.id })
                        .then(res => {
                            should.exist(res);
                            res.should.be.an.Object();
                            res.should.have.property('ECHOTEXT');
                            res.ECHOTEXT.should.startWith(REQUTEXT + c.id);
                            c.close(() => {
                                if (++callbackCount === CONNECTIONS) done();
                            });
                        })
                        .catch(err => {
                            return done(err);
                        });
                })
                .catch(err => {
                    return done(err);
                });
        }
    });

    it(`concurrency: ${CONNECTIONS} parallel call() promises, using single connection`, function(done) {
        let promises = [];
        for (let counter = 0; counter < CONNECTIONS; counter++) {
            promises.push(
                client
                    .call('STFC_CONNECTION', { REQUTEXT: REQUTEXT + counter })
                    .then(res => {
                        should.exist(res);
                        res.should.be.an.Object();
                        res.should.have.property('ECHOTEXT');
                        res.ECHOTEXT.should.startWith(REQUTEXT + counter);
                    })
                    .catch(err => {
                        return err;
                    })
            );
        }
        Promise.all(promises)
            .then(() => {
                done();
            })
            .catch(err => {
                done(err);
            });
    });

    it(`concurrency: ${CONNECTIONS} recursive call() promises, using single connection`, function(done) {
        let callbackCount = 0;
        function call(count) {
            client
                .call('STFC_CONNECTION', { REQUTEXT: REQUTEXT + count })
                .then(res => {
                    should.exist(res);
                    res.should.be.an.Object();
                    res.should.have.property('ECHOTEXT');
                    res.ECHOTEXT.should.startWith(REQUTEXT + count);
                    if (++callbackCount == CONNECTIONS) {
                        done();
                    } else {
                        call(callbackCount);
                    }
                })
                .catch(err => {
                    return done(err);
                });
        }
        call(callbackCount);
    });
});

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

const rfcClient = require('../noderfc').Client;
const abapSystem = require('../abapSystem')();

const should = require('should');
const Promise = require('bluebird');

const CONNECTIONS = require('./config').connections;

describe('Concurrency promises', function() {
    this.timeout(15000);

    let client = new rfcClient(abapSystem);

    beforeEach(function() {
        return client.reopen();
    });

    afterEach(function() {
        return client.close();
    });
    
    it('concurrency: call() should not block', function(done) {
        let asyncRes;
        client
            .call('BAPI_USER_GET_DETAIL', { USERNAME: 'DEMO' })
            .then(res => {
                res.should.be.an.Object();
                res.should.have.properties('RETURN');
                res.RETURN.should.be.an.Array();
                res.RETURN.length.should.equal(0);
                asyncRes = res;
                done();
            })
            .catch(err => {
                return done(err);
            });
        should.not.exist(asyncRes);
    });

    it(`concurrency: ${CONNECTIONS} parallel call() promises`, function(done) {
        let callbackCount = 0;
        for (let i = 0; i < CONNECTIONS; i++) {
            new rfcClient(abapSystem)
                .open()
                .then(c => {
                    c.call('BAPI_USER_GET_DETAIL', { USERNAME: 'DEMO' })
                        .then(res => {
                            res.should.be.an.Object();
                            res.should.have.properties('RETURN');
                            res.RETURN.should.be.an.Array();
                            res.RETURN.length.should.equal(0);
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

    xit(`concurrency: ${CONNECTIONS} concurrent call() promises, using single connection`, function(done) {
        let callbackCount = 0;
        for (let i = 0; i < CONNECTIONS; i++) {
            client
                .call('BAPI_USER_GET_DETAIL', { USERNAME: 'DEMO' })
                .then(res => {
                    res.should.be.an.Object();
                    res.should.have.properties('RETURN');
                    res.RETURN.should.be.an.Array();
                    res.RETURN.length.should.equal(0);
                    if (++callbackCount == CONNECTIONS) done();
                })
                .catch(err => {
                    return done(err);
                });
        }
    });

    it(`concurrency: ${CONNECTIONS} concurrent call() promises, using single connection and Promise.all()`, function() {
        let promises = [];
        for (let counter = 0; counter < CONNECTIONS; counter++) {
            promises.push(
                client
                    .call('BAPI_USER_GET_DETAIL', { USERNAME: 'DEMO' })
                    .then(res => {
                        res.should.be.an.Object();
                        res.should.have.properties('RETURN');
                        res.RETURN.should.be.an.Array();
                        res.RETURN.length.should.equal(0);
                    })
                    .catch(err => {
                        return err;
                    })
            );
        }
        return Promise.all(promises);
    });

    it(`concurrency: ${CONNECTIONS} recursive call() promises, using single connection`, function(done) {
        let callbackCount = 0;
        function call() {
            client
                .call('BAPI_USER_GET_DETAIL', { USERNAME: 'DEMO' })
                .then(res => {
                    res.should.be.an.Object();
                    res.should.have.properties('RETURN');
                    res.RETURN.should.be.an.Array();
                    res.RETURN.length.should.equal(0);
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
        call();
    });
});

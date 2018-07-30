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

const CONNECTIONS = 50;


describe('Concurrency promises', function() {
    this.timeout(15000);

    let client = new rfcClient(abapSystem);

    before(function() {
        if (client) return client.close();
    });

    beforeEach(function() {
        return client.open();
    });

    afterEach(function() {
        return client.close();
    });

    after(function() {
        return client.close();
    });

    const REQUTEXT = 'Hellö SÄP!';

    it('non-blocking call()', function(done) {
        let asyncRes = undefined;
        client
            .call('STFC_CONNECTION', { REQUTEXT: REQUTEXT })
            .then(res => {
                should.exist(res);
                res.should.be.an.Object();
                res.should.have.property('ECHOTEXT');
                res.ECHOTEXT.should.startWith(REQUTEXT);
                asyncRes = res;
                done();
            })
            .catch((err) => {
                return done(err);
            });
        should.not.exist(asyncRes);
    });

    it(`concurrency: ${CONNECTIONS} connections call()`, function(done) {
        let count = CONNECTIONS;
        let CLIENTS = [];
        for (let i = 0; i < count; i++) {
            CLIENTS.push(new rfcClient(abapSystem));
        }
        for (let c of CLIENTS) {
            c.open().then(client => {
                client
                    .call('STFC_CONNECTION', { REQUTEXT: REQUTEXT + client.id })
                    .then(res => {
                        should.exist(res);
                        res.should.be.an.Object();
                        res.should.have.property('ECHOTEXT');
                        res.ECHOTEXT.should.startWith(REQUTEXT + client.id);
                        client.close();
                        if (--count === 1) { 
				done();
			}
                    })
                    .catch(err => {
                        return done(err);
                    });
            });
        }
    });

    it(`concurrency: 1 connection, ${CONNECTIONS} call() calls`, function(done) {
    
        let promises = [];
        for (let counter = 0; counter < CONNECTIONS; counter++) {
            promises.push(
                client.call('STFC_CONNECTION', { REQUTEXT: REQUTEXT + counter })
		.then(res => {
                    should.exist(res);
                    res.should.be.an.Object();
                    res.should.have.property('ECHOTEXT');
                    res.ECHOTEXT.should.startWith(REQUTEXT + counter);
                })
                .catch(err => {
                    return done(new Error(JSON.stringify(err)));
                });
            );
        }
        Promise.all(promises).then(() => {done()});
    });

    it(`concurrency: 1 connection, ${CONNECTIONS} recursive call() calls`, function(done) {
        function rec(depth) {
            if (depth < CONNECTIONS) {
                client
                    .call('STFC_CONNECTION', { REQUTEXT: REQUTEXT + depth })
                    .then(res => {
                        should.exist(res);
                        res.should.be.an.Object();
                        res.should.have.property('ECHOTEXT');
                        res.ECHOTEXT.should.startWith(REQUTEXT + depth);
                        rec(depth + 1);
                    })
                    .catch(err => {
                        return done(new Error(JSON.stringify(err)));
                    });
            } else {
                
		    done();
            }
        }
        rec(1);
    });
});

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

const setup = require('../setup');
const client = setup.client;

const Promise = require('bluebird');

//this.timeout(15000);

afterAll(function (done) {
    delete setup.client;
    delete setup.rfcClient;
    delete setup.rfcPool;
    done();
});

it('concurrency: call() should not block', function () {
    let asyncRes;
    return client
        .open()
        .then(function () {
            client.call('BAPI_USER_GET_DETAIL', { USERNAME: 'DEMO' })
                .then(res => {
                    expect(res).toBeDefined();
                    expect(res).toHaveProperty('RETURN');
                    expect(res.RETURN.length).toBe(0);
                    asyncRes = res;
                    client.close();
                })
                .catch(err => {
                    console.error(err);
                });
        })

    expect(asyncRes).toBeUndefined();
});

it(`concurrency: ${setup.CONNECTIONS} parallel call() promises`, function (done) {
    let callbackCount = 0;
    for (let i = 0; i < setup.CONNECTIONS; i++) {
        new setup.rfcClient(setup.abapSystem)
            .open()
            .then(c => {
                c.call('BAPI_USER_GET_DETAIL', { USERNAME: 'DEMO' })
                    .then(res => {
                        expect(res).toBeDefined();
                        expect(res).toHaveProperty('RETURN');
                        expect(res.RETURN.length).toBe(0);
                        c.close(() => {
                            if (++callbackCount === setup.CONNECTIONS) done();
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

it(`concurrency: ${setup.CONNECTIONS} concurrent call() promises, using single connection`, function () {
    let callbackCount = 0;
    return client
        .open()
        .then(function () {
            for (let i = 0; i < setup.CONNECTIONS; i++) {
                client
                    .call('BAPI_USER_GET_DETAIL', { USERNAME: 'DEMO' })
                    .then(res => {
                        expect(res).toBeDefined();
                        expect(res).toHaveProperty('RETURN');
                        expect(res.RETURN.length).toBe(0);
                        if (++callbackCount == setup.CONNECTIONS) client.close();
                    })
            }
        })
});

it(`concurrency: ${setup.CONNECTIONS} concurrent call() promises, using single connection and Promise.all()`, function () {
    let promises = [];
    for (let counter = 0; counter < setup.CONNECTIONS; counter++) {
        promises.push(
            client
                .call('BAPI_USER_GET_DETAIL', { USERNAME: 'DEMO' })
                .then(res => {
                    expect(res).toBeDefined();
                    expect(res).toHaveProperty('RETURN');
                    expect(res.RETURN.length).toBe(0);
                })
        );
    }
    return Promise.all(promises);
});

it(`concurrency: ${setup.CONNECTIONS} recursive call() promises, using single connection`, function () {
    let callbackCount = 0;
    function call() {
        client
            .call('BAPI_USER_GET_DETAIL', { USERNAME: 'DEMO' })
            .then(res => {
                expect(res).toBeDefined();
                expect(res).toHaveProperty('RETURN');
                expect(res.RETURN.length).toBe(0);
                if (++callbackCount == setup.CONNECTIONS) {
                    client.close();
                } else {
                    call(callbackCount);
                }
            })
            .catch(err => {
                return done(err);
            });
    }

    return client
        .open()
        .then(function () {
            call();
        });

    call();
});

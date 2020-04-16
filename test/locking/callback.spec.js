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

describe('Concurrency: Callbacks', () => {
    const WAIT_SECONDS = 1;

    test('invoke() and invoke ()', function (done) {
        expect.assertions(7);

        let count = 0;
        const client = setup.client();
        client.connect((err) => {
            if (err) return done(err);

            client.invoke('RFC_PING_AND_WAIT', {
                    SECONDS: WAIT_SECONDS
                },
                function (err) {
                    if (err) return done(err);
                    count++;
                });

            // Invoke not blocking
            expect(count).toEqual(0);

            // 1 call is running
            expect(client.runningRFCCalls).toEqual(1);

            client.invoke('RFC_PING_AND_WAIT', {
                    SECONDS: WAIT_SECONDS
                },
                function (err) {
                    if (err) return done(err);
                    count++;
                });

            // Invoke not blocking
            expect(count).toEqual(0);

            // 1 calls are running
            expect(client.runningRFCCalls).toEqual(1);

            client.close(err => {
                // Close rejected because of ongoing calls
                expect(err).toEqual('Close rejected because 2 RFC calls still running');
                expect(count).toEqual(0);
                done();
            });
            // Close not blocking
            expect(count).toEqual(0);
        })

    }, 6000);

    test('invoke() and ping ()', function (done) {
        expect.assertions(6);
        let count = 0;

        const client = setup.client();
        client.connect((err) => {
            if (err) return done(err);
            client.invoke('RFC_PING_AND_WAIT', {
                    SECONDS: WAIT_SECONDS
                },
                function (err, res) {
                    count++;
                });
            // Invoke not blocking
            expect(count).toEqual(0);

            client.ping((err, res) => {
                expect(res).toBeTruthy();
                //count++;
            });

            // Ping not blocking
            expect(count).toEqual(0);

            client.close(err => {
                // Close rejected because of RFC call running
                expect(err).toEqual('Close rejected because 1 RFC calls still running');
                expect(count).toEqual(0);
                done();
            })
            // Close not blocking
            expect(count).toEqual(0);
        });
    }, 2000);

    test('ping() and ping ()', function (done) {
        const COUNT = setup.CONNECTIONS;
        expect.assertions(3 + COUNT * 3);
        let count = 0;

        const client = setup.client();
        client.connect((err) => {
            if (err) return done(err);
            for (let i = 0; i < COUNT; i++) {
                client.ping().then(res => {
                    expect(res).toBeTruthy();
                    count++;
                });
                // Ping not blocking
                expect(client.runningRFCCalls).toEqual(0);
                expect(count).toEqual(0);
            }

            client.close(err => {
                // Close scheduled after ping calls completed
                expect(err).toBeUndefined();
                expect(count).toEqual(COUNT);
                done();
            });
            expect(count).toEqual(0);
        })
    }, 2000);

    test('invoke() and close ()', function (done) {
        expect.assertions(4);
        let count = 0;

        const client = setup.client();
        client.connect((err) => {
            if (err) return done(err);;

            client.invoke('RFC_PING_AND_WAIT', {
                    SECONDS: WAIT_SECONDS
                },
                function (err) {
                    if (err) return done(err);
                    count++;
                });
            expect(count).toEqual(0);

            client.close(err => {
                // Close rejected because of RFC call running
                expect(err).toEqual('Close rejected because 1 RFC calls still running');
                expect(count).toEqual(0);
                done();
            });
            // Close not blocking
            expect(count).toEqual(0);
        });
    }, 3000);

    test('ping() and close ()', function (done) {
        expect.assertions(5);
        let count = 0;

        const client = setup.client();
        client.connect((err) => {
            if (err) return done(err);

            client.ping((err, res) => {
                expect(res).toBeTruthy();
                count++;
            });
            expect(count).toEqual(0);

            client.close(err => {
                expect(err).toBeUndefined();
                // Close scheduled after ping()
                expect(count).toEqual(1);
                done();
            });
            // Close not blocking
            expect(count).toEqual(0);
        });
    }, 3000);
})

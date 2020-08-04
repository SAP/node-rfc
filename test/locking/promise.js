// Copyright 2014 SAP AG.
// SPDX-FileCopyrightText: 2014 SAP SE Srdjan Boskovic <srdjan.boskovic@sap.com>
//
// SPDX-License-Identifier: Apache-2.0

"use strict";

module.exports = () => {
    const setup = require("../utils/setup");

    const WAIT_SECONDS = 1;

    test("call() and call ()", function (done) {
        expect.assertions(6);

        let count = 0;
        setup
            .client()
            .open()
            .then((client) => {
                client
                    .call("RFC_PING_AND_WAIT", {
                        SECONDS: WAIT_SECONDS,
                    })
                    .then(() => {
                        count++;
                    });
                // Call not blocking
                expect(client.runningRFCCalls).toEqual(1);

                client
                    .call("RFC_PING_AND_WAIT", {
                        SECONDS: WAIT_SECONDS,
                    })
                    .then(() => {
                        count++;
                    });
                expect(client.runningRFCCalls).toEqual(1);

                client
                    .call("RFC_PING_AND_WAIT", {
                        SECONDS: WAIT_SECONDS,
                    })
                    .then(() => {
                        count++;
                    });
                expect(count).toEqual(0);
                expect(client.runningRFCCalls).toEqual(1);

                client
                    .close()
                    .then(() => done("error!"))
                    .catch((ex) => {
                        expect(ex).toEqual(
                            "Close rejected because 3 RFC calls still running"
                        );
                        done();
                    });

                expect(count).toEqual(0);
            });
    }, 8000);

    test("call() and ping ()", function (done) {
        expect.assertions(5);
        let count = 0;
        setup
            .client()
            .open()
            .then((client) => {
                client
                    .call("RFC_PING_AND_WAIT", {
                        SECONDS: WAIT_SECONDS,
                    })
                    .then(() => count++);

                expect(count).toEqual(0);

                client.ping().then((res) => {
                    expect(res).toBeTruthy();
                    count++;
                });

                // Ping not blocking
                expect(count).toEqual(0);

                client.close().catch((ex) => {
                    expect(ex).toEqual(
                        "Close rejected because 1 RFC calls still running"
                    );
                    done();
                });
                expect(count).toEqual(0);
            });
    }, 2000);

    test("ping() and ping ()", function (done) {
        expect.assertions(8);
        let count = 0;
        setup
            .client()
            .open()
            .then((client) => {
                client.ping().then((res) => {
                    expect(res).toBeTruthy();
                    count++;
                });

                expect(count).toEqual(0);

                client.ping().then((res) => {
                    expect(res).toBeTruthy();
                    count++;
                });

                expect(count).toEqual(0);

                client.ping().then((res) => {
                    expect(res).toBeTruthy();
                    count++;
                });

                // Ping not blocking
                expect(count).toEqual(0);

                client.close().then(() => {
                    // Close scheduled after invoke() and ping()
                    expect(count).toEqual(3);
                    done();
                });
                expect(count).toEqual(0);
            });
    }, 2000);

    test("call() and close ()", function (done) {
        expect.assertions(3);

        let count = 0;

        setup
            .client()
            .open()
            .then((client) => {
                client
                    .call("RFC_PING_AND_WAIT", {
                        SECONDS: WAIT_SECONDS,
                    })
                    .then(() => {
                        count++;
                    });

                // Call not blocking
                expect(count).toEqual(0);

                client.close().catch((ex) => {
                    expect(ex).toEqual(
                        "Close rejected because 1 RFC calls still running"
                    );
                    done();
                });
                // Close not blocking
                expect(count).toEqual(0);
            });
    }, 2000);

    test("ping() and close ()", function (done) {
        expect.assertions(4);

        let count = 0;

        setup
            .client()
            .open()
            .then((client) => {
                client.ping().then((res) => {
                    expect(res).toBeTruthy();
                    count++;
                });

                // Call not blocking
                expect(count).toEqual(0);

                client.close().then(() => {
                    // Close scheduled after invokes completed
                    expect(count).toEqual(1);
                    done();
                });
                // Close not blocking
                expect(count).toEqual(0);
            });
    }, 2000);
};

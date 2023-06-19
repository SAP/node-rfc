// Copyright 2014 SAP AG.
// SPDX-FileCopyrightText: 2014 SAP SE Srdjan Boskovic <srdjan.boskovic@sap.com>
//
// SPDX-License-Identifier: Apache-2.0

import { RfcObject, direct_client } from "../utils/setup";

describe("Locking: Callbacks", () => {
    const WAIT_SECONDS = 1;

    test("invoke() and invoke ()", function (done) {
        expect.assertions(3);

        let count = 0;
        const client = direct_client();
        void client.connect((err: unknown) => {
            if (err) return done(err) as unknown;

            client.invoke(
                "RFC_PING_AND_WAIT",
                {
                    SECONDS: WAIT_SECONDS,
                },
                function (err) {
                    if (err) return done(err) as unknown;
                    count++;
                }
            );

            // Invoke not blocking
            expect(count).toEqual(0);

            client.invoke(
                "RFC_PING_AND_WAIT",
                {
                    SECONDS: WAIT_SECONDS,
                },
                function (err: unknown) {
                    if (err) return done(err) as unknown;
                    count++;
                }
            );

            // Invoke not blocking
            expect(count).toEqual(0);

            client.invoke(
                "RFC_PING_AND_WAIT",
                {
                    SECONDS: WAIT_SECONDS,
                },
                function (err) {
                    if (err) return done(err) as unknown;
                    count++;
                    done();
                }
            );

            // Invoke not blocking
            expect(count).toEqual(0);

            // 3 calls are running
            // expect(client.runningRFCCalls).toEqual();

            //client.close((err) => {
            //    // Close rejected because of ongoing calls
            //    expect(err).toEqual(
            //        "Close rejected because 3 RFC calls still running"
            //    );
            //    expect(count).toEqual(0);
            //    setTimeout(() => {
            //        client.close(() => done());
            //    }, 2000);
            //});
            // Close not blocking
            //expect(count).toEqual(0);
        });
    }, 6000);

    test("invoke() and ping ()", function (done) {
        expect.assertions(3);
        let count = 0;

        const client = direct_client();
        void client.connect((err: unknown) => {
            if (err) return done(err) as unknown;
            client.invoke(
                "RFC_PING_AND_WAIT",
                {
                    SECONDS: WAIT_SECONDS,
                },
                function () {
                    count++;
                }
            );
            // Invoke not blocking
            expect(count).toEqual(0);

            void client.ping((err: unknown, res: boolean) => {
                expect(res).toBe(true);
                count++;
                done();
            });

            // Ping not blocking
            expect(count).toEqual(0);

            //client.close((err) => {
            //    // Close rejected because of RFC call running
            //    expect(err).toEqual(
            //        "Close rejected because 1 RFC calls still running"
            //    );
            //    expect(count).toEqual(0);
            //    setTimeout(() => {
            //        client.close(() => done());
            //    }, 2000);
            //});
            // Close not blocking
            //expect(count).toEqual(0);
        });
    }, 4000);

    test("ping() and ping ()", function (done) {
        const COUNT = 5;
        expect.assertions(2 + COUNT * 2);
        let count = 0;

        const client = direct_client();
        void client.connect((err: unknown) => {
            if (err) return done(err) as unknown;
            for (let i = 0; i < COUNT; i++) {
                void client.ping((err: unknown, res: RfcObject) => {
                    if (err) return done(err) as unknown;
                    expect(res).toBe(true);
                    count++;
                    if (count === COUNT) {
                        void client.close((err) => {
                            // Close after all ping calls completed
                            expect(err).toBeUndefined();
                            expect(count).toEqual(COUNT);
                            done();
                        });
                    }
                });
                // Pings not blocking
                expect(count).toEqual(0);
            }
        });
    }, 2000);

    test("invoke() and close ()", function (done) {
        expect.assertions(3);
        let count = 0;

        const client = direct_client();
        void client.connect((err: unknown) => {
            if (err) return done(err) as unknown;
            client.invoke(
                "RFC_PING_AND_WAIT",
                {
                    SECONDS: WAIT_SECONDS,
                },
                function (err) {
                    // Close not blocking
                    expect(err).toMatchObject(
                        expect.objectContaining({
                            message:
                                "RFM client request over closed connection: invoke() RFC_PING_AND_WAIT",
                            name: "nodeRfcError",
                        })
                    );
                    done();
                }
            );

            // Invoke not blocking
            expect(count).toEqual(0);

            void client.close(() => {
                count++;
            });

            // Close not blocking
            expect(count).toEqual(0);
        });
    }, 3000);

    test("ping() and close ()", function (done) {
        expect.assertions(5);
        let count = 0;

        const client = direct_client();
        void client.connect((err: unknown) => {
            if (err) return done(err) as unknown;

            void client.ping((err: unknown, res: boolean) => {
                expect(res).toBeTruthy();
                count++;
            });
            expect(count).toEqual(0);

            void client.close((err: unknown) => {
                expect(err).toBeUndefined();
                // Close scheduled after ping()
                expect(count).toEqual(1);
                done();
            });
            // Close not blocking
            expect(count).toEqual(0);
        });
    }, 3000);
});

// Copyright 2014 SAP AG.
// SPDX-FileCopyrightText: 2014 SAP SE Srdjan Boskovic <srdjan.boskovic@sap.com>
//
// SPDX-License-Identifier: Apache-2.0

import { direct_client, CONNECTIONS, Client } from "../utils/setup";

describe("Concurrency: Promises", () => {
    const WAIT_SECONDS = 1;
    const TEST_TIMEOUT = 40000; // 40 sec

    test(
        `${CONNECTIONS} clients make concurrent call() requests`,
        function (done) {
            expect.assertions(CONNECTIONS);
            let callbackCount = 0;

            for (let i = 0; i < CONNECTIONS; i++) {
                (direct_client().open() as Promise<Client>)
                    .then((client) => {
                        client
                            .call(
                                i % 2 === 0
                                    ? "BAPI_USER_GET_DETAIL"
                                    : "RFC_PING_AND_WAIT",
                                i % 2 === 0
                                    ? {
                                          USERNAME: "DEMO",
                                      }
                                    : {
                                          SECONDS: WAIT_SECONDS,
                                      }
                            )
                            .then((res) => {
                                expect(res).toBeDefined();
                                void client.close(() => {
                                    if (++callbackCount === CONNECTIONS) done();
                                });
                            })
                            .catch((ex) => {
                                done(ex);
                            });
                    })
                    .catch((ex) => {
                        done(ex);
                    });
            }
        },
        TEST_TIMEOUT
    );

    test(
        `${CONNECTIONS} clients make concurrent ping() requests`,
        function (done) {
            const CLIENTS = CONNECTIONS;
            expect.assertions(CLIENTS);
            let callbackCount = 0;

            for (let i = 0; i < CLIENTS; i++) {
                (direct_client().open() as Promise<Client>)
                    .then((client) =>
                        (client.ping() as Promise<boolean>)
                            .then((res) => {
                                expect(res).toBeTruthy();
                                void client.close(() => {
                                    if (++callbackCount === CLIENTS) done();
                                });
                            })
                            .catch((ex) => {
                                done(ex);
                            })
                    )
                    .catch((ex) => {
                        done(ex);
                    });
            }
        },
        TEST_TIMEOUT
    );
});

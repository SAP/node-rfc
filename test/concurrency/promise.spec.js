// Copyright 2014 SAP AG.
// SPDX-FileCopyrightText: 2014 SAP SE Srdjan Boskovic <srdjan.boskovic@sap.com>
//
// SPDX-License-Identifier: Apache-2.0

"use strict";

describe("Concurrency: Promises", () => {
    const setup = require("../utils/setup");

    const WAIT_SECONDS = 1;

    test(`${setup.CONNECTIONS} clients make concurrent call() requests`, function (done) {
        expect.assertions(setup.CONNECTIONS);
        let callbackCount = 0;

        for (let i = 0; i < setup.CONNECTIONS; i++) {
            setup
                .direct_client()
                .open()
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
                            client.close(() => {
                                if (++callbackCount === setup.CONNECTIONS)
                                    done();
                            });
                        })
                        .catch((ex) => done(ex));
                })
                .catch((ex) => done(ex));
        }
    }, 15000);

    test(`${setup.CONNECTIONS} clients make concurrent ping() requests`, function (done) {
        const CLIENTS = setup.CONNECTIONS;
        expect.assertions(CLIENTS);
        let callbackCount = 0;

        for (let i = 0; i < CLIENTS; i++) {
            setup
                .direct_client()
                .open()
                .then((client) =>
                    client
                        .ping()
                        .then((res) => {
                            expect(res).toBeTruthy();
                            client.close(() => {
                                if (++callbackCount === CLIENTS) done();
                            });
                        })
                        .catch((ex) => done(ex))
                )
                .catch((ex) => done(ex));
        }
    }, 10000);
});

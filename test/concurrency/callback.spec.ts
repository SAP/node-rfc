// Copyright 2014 SAP AG.
// SPDX-FileCopyrightText: 2014 SAP SE Srdjan Boskovic <srdjan.boskovic@sap.com>
//
// SPDX-License-Identifier: Apache-2.0

import { direct_client, CONNECTIONS } from "../utils/setup";

describe("Concurrency: Callbacks", () => {
    const WAIT_SECONDS = 1;

    test(`${CONNECTIONS} clients make concurrent invoke() requests`, function (done) {
        expect.assertions(CONNECTIONS);
        let callbackCount = 0;

        for (let i = 0; i < CONNECTIONS; i++) {
            const c = direct_client();
            c.connect((err) => {
                if (err) return done(err);
                c.invoke(
                    i % 2 === 0 ? "BAPI_USER_GET_DETAIL" : "RFC_PING_AND_WAIT",
                    i % 2 === 0
                        ? {
                              USERNAME: "DEMO",
                          }
                        : {
                              SECONDS: WAIT_SECONDS,
                          },
                    (err, res) => {
                        if (err) return done(err);
                        expect(res).toBeDefined();
                        c.close(() => {
                            if (++callbackCount === CONNECTIONS) done();
                        });
                    }
                );
            });
        }
    }, 60000);

    test(`${CONNECTIONS} clients make concurrent ping() requests`, function (done) {
        expect.assertions(CONNECTIONS);
        let callbackCount = 0;

        for (let i = 0; i < CONNECTIONS; i++) {
            const c = direct_client();
            c.connect((err) => {
                if (err) return done(err);

                c.ping((err, res) => {
                    expect(res).toBeTruthy();
                    c.close(() => {
                        if (++callbackCount === CONNECTIONS) done();
                    });
                });
            });
        }
    }, 60000);
});

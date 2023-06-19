// Copyright 2014 SAP AG.
// SPDX-FileCopyrightText: 2014 SAP SE Srdjan Boskovic <srdjan.boskovic@sap.com>
//
// SPDX-License-Identifier: Apache-2.0

import { direct_client, CONNECTIONS, RfcObject } from "../utils/setup";

describe("Concurrency: Callbacks", () => {
    const WAIT_SECONDS = 1;

    test(`${CONNECTIONS} clients make concurrent invoke() requests`, function (done) {
        expect.assertions(CONNECTIONS);
        let callbackCount = 0;

        for (let i = 0; i < CONNECTIONS; i++) {
            const c = direct_client();
            void c.connect((err: unknown) => {
                if (err) return done(err) as unknown;
                c.invoke(
                    i % 2 === 0 ? "BAPI_USER_GET_DETAIL" : "RFC_PING_AND_WAIT",
                    i % 2 === 0
                        ? {
                              USERNAME: "DEMO",
                          }
                        : {
                              SECONDS: WAIT_SECONDS,
                          },
                    (err: unknown, res: RfcObject) => {
                        if (err) return done(err) as unknown;
                        expect(res).toBeDefined();
                        void c.close(() => {
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
            void c.connect((err: unknown) => {
                if (err) return done(err) as unknown;

                void c.ping((err: unknown, res: boolean) => {
                    expect(res).toBeTruthy();
                    void c.close(() => {
                        if (++callbackCount === CONNECTIONS) done();
                    });
                });
            });
        }
    }, 60000);
});

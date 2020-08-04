// Copyright 2014 SAP AG.
// SPDX-FileCopyrightText: 2014 SAP SE Srdjan Boskovic <srdjan.boskovic@sap.com>
//
// SPDX-License-Identifier: Apache-2.0

"use strict";

describe("Concurrency: Await", () => {
    const setup = require("../utils/setup");

    const WAIT_SECONDS = 1;

    test(`${setup.CONNECTIONS} sequential calls over single connection`, function (done) {
        expect.assertions(setup.CONNECTIONS);
        const client = setup.direct_client();
        (async () => {
            await client.open();
            for (let i = 0; i < setup.CONNECTIONS; i++) {
                try {
                    let res = await client.call("BAPI_USER_GET_DETAIL", {
                        USERNAME: "DEMO",
                    });
                    expect(res).toBeDefined();
                } catch (ex) {
                    done(ex);
                }
            }
            await client.close();
            done();
        })();
    }, 30000);

    test(`${setup.CONNECTIONS} clients make concurrent call() requests`, function (done) {
        expect.assertions(setup.CONNECTIONS);
        (async () => {
            const CLIENTS = [];
            for (let i = 0; i < setup.CONNECTIONS; i++) {
                const c = await setup.direct_client().open();
                CLIENTS.push(c);
            }
            let callbackCount = 0;
            for (const [i, c] of CLIENTS.entries()) {
                try {
                    let res = await c.call(
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
                    );
                    expect(res).toBeDefined();
                    await c.close();
                    if (++callbackCount === setup.CONNECTIONS) done();
                } catch (ex) {
                    done(ex);
                }
            }
        })();
    }, 36000);
});

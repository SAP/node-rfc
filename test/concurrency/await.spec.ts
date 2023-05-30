// Copyright 2014 SAP AG.
// SPDX-FileCopyrightText: 2014 SAP SE Srdjan Boskovic <srdjan.boskovic@sap.com>
//
// SPDX-License-Identifier: Apache-2.0

import { direct_client, CONNECTIONS, Client } from "../utils/setup";

describe("Concurrency: Await", () => {
    const WAIT_SECONDS = 1;

    test(`${CONNECTIONS} sequential calls over single connection`, async () => {
        expect.assertions(CONNECTIONS);
        const client = direct_client();
        await client.open();
        for (let i = 0; i < CONNECTIONS; i++) {
            try {
                const res = await client.call("BAPI_USER_GET_DETAIL", {
                    USERNAME: "DEMO",
                });
                expect(res).toBeDefined();
            } catch (ex) {
                // console.error(ex);
            }
        }
        await client.close();
    }, 60000);

    test(`${CONNECTIONS} clients make concurrent call() requests`, async () => {
        expect.assertions(CONNECTIONS);
        const CLIENTS = [] as Client[];
        for (let i = 0; i < CONNECTIONS; i++) {
            const c = (await direct_client().open()) as Client;
            CLIENTS.push(c);
        }
        // let callbackCount = 0;
        for (const [i, c] of CLIENTS.entries()) {
            try {
                const res = await c.call(
                    i % 2 === 0 ? "BAPI_USER_GET_DETAIL" : "RFC_PING_AND_WAIT",
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
                // if (++callbackCount === CONNECTIONS) done();
            } catch (ex) {
                // console.error(ex);
            }
        }
    }, 60000);
});

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

"use strict";

const setup = require("../setup");

describe("Concurrency: Await", () => {
    const WAIT_SECONDS = 1;

    test(`${setup.CONNECTIONS} sequential calls over single connection`, function (done) {
        expect.assertions(setup.CONNECTIONS);
        const client = setup.client();
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
                const c = await setup.client().open();
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

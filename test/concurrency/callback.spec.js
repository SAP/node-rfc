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

describe("Concurrency: Callbacks", () => {
    const setup = require("../utils/setup");

    const WAIT_SECONDS = 1;

    test(`${setup.CONNECTIONS} clients make concurrent invoke() requests`, function (done) {
        expect.assertions(setup.CONNECTIONS);
        let callbackCount = 0;

        for (let i = 0; i < setup.CONNECTIONS; i++) {
            const c = setup.direct_client();
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
                            if (++callbackCount === setup.CONNECTIONS) done();
                        });
                    }
                );
            });
        }
    }, 10000);

    test(`${setup.CONNECTIONS} clients make concurrent ping() requests`, function (done) {
        expect.assertions(setup.CONNECTIONS);
        let callbackCount = 0;

        for (let i = 0; i < setup.CONNECTIONS; i++) {
            const c = setup.direct_client();
            c.connect((err) => {
                if (err) return done(err);

                c.ping((err, res) => {
                    expect(res).toBeTruthy();
                    c.close(() => {
                        if (++callbackCount === setup.CONNECTIONS) done();
                    });
                });
            });
        }
    }, 10000);
});

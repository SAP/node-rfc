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
const client = setup.client;

beforeEach(function(done) {
    client.reopen(err => {
        done(err);
    });
});

afterEach(function(done) {
    client.close(() => {
        done();
    });
});

afterAll(function(done) {
    delete setup.client;
    delete setup.rfcClient;
    delete setup.rfcPool;
    done();
});

const REQUTEXT = "Hellö SÄP!";

it(`await: ${setup.CONNECTIONS} sequential calls using single connection`, function(done) {
    (async () => {
        for (let i = 0; i < setup.CONNECTIONS; i++) {
            try {
                let res = await client.call("BAPI_USER_GET_DETAIL", {
                    USERNAME: "DEMO"
                });
                expect(res).toBeDefined();
                expect(res).toHaveProperty("RETURN");
                expect(res.RETURN.length).toBe(0);
            } catch (ex) {
                return done(ex);
            }
        }
        done();
    })();
});

it(`await: ${setup.CONNECTIONS} parallel connections`, function(done) {
    (async () => {
        let CLIENTS = [];
        for (let i = 0; i < setup.CONNECTIONS; i++) {
            let c = await new setup.rfcClient(setup.abapSystem).open();
            CLIENTS.push(c);
        }
        for (let c of CLIENTS) {
            try {
                let res = await client.call("BAPI_USER_GET_DETAIL", {
                    USERNAME: "DEMO"
                });
                expect(res).toBeDefined();
                expect(res).toHaveProperty("RETURN");
                expect(res.RETURN.length).toBe(0);
                await c.close();
            } catch (ex) {
                return done(ex);
            }
        }
        done();
    })();
});

it(`await: ${setup.CONNECTIONS} recursive calls using single connection`, function(done) {
    let callbackCount = 0;
    async function call() {
        try {
            let res = await client.call("BAPI_USER_GET_DETAIL", {
                USERNAME: "DEMO"
            });
            expect(res).toBeDefined();
            expect(res).toHaveProperty("RETURN");
            expect(res.RETURN.length).toBe(0);
            if (++callbackCount == setup.CONNECTIONS) {
                done();
            } else {
                call();
            }
        } catch (ex) {
            done(ex);
        }
    }
    call();
});

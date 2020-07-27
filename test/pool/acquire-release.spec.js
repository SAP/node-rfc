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

describe("Pool Acquire/Release", () => {
    const setup = require("../utils/setup");
    const Pool = setup.Pool;
    const abapSystem = setup.abapSystem();
    const poolConfiguration = {
        connectionParameters: abapSystem,
        //clientOptions: { filter: 2, bcd: "number" },
        //poolOptions: { low: 2, high: 4 },
    };
    let pool;

    beforeAll((done) => {
        pool = new Pool(poolConfiguration);
        done();
    });

    afterAll((done) => {
        done();
    });

    test("pool: acquire()", function (done) {
        expect.assertions(1);
        pool.acquire().then((client) => {
            expect(client.alive).toBe(true);
            done();
        });
    });

    test("pool: acquire(multiple)", function (done) {
        const N = 3;
        expect.assertions(N + 1);
        pool.acquire(N).then((clients) => {
            expect(clients.length).toBe(N);
            clients.forEach((c) => {
                expect(c.alive).toBe(true);
            });
            done();
        });
    });

    test("pool: release(single)", function (done) {
        expect.assertions(3);
        pool.acquire((err, client) => {
            expect(err).not.toBeDefined();
            const LEASED = pool.status.leased;
            pool.release(client, (err) => {
                expect(err).not.toBeDefined();
                expect(pool.status.leased).toBe(LEASED - 1);
                done();
            });
        });
    });

    test("pool: release(multiple)", function (done) {
        expect.assertions(4);
        const N = 3;
        pool.acquire(N, (err, clients) => {
            expect(err).not.toBeDefined();
            expect(clients.length).toBe(N);
            const LEASED = pool.status.leased;
            pool.release(clients, (err) => {
                expect(err).not.toBeDefined();
                expect(pool.status.leased).toBe(LEASED - N);
                done();
            });
        });
    });

    test("pool: release already released error", function (done) {
        expect.assertions(3);
        pool.acquire((err, client) => {
            expect(err).not.toBeDefined();
            pool.release(client, (err) => {
                expect(err).not.toBeDefined();
                pool.release(client, (err) => {
                    expect(err).toMatchObject({
                        message:
                            "Client release() invoked for already closed client",
                        name: "nodeRfcError",
                    });
                    done();
                });
            });
        });
    });

    test("client: release()", function (done) {
        expect.assertions(3);
        pool.acquire((err, client) => {
            const LEASED = pool.status.leased;
            expect(err).not.toBeDefined();
            client.release((err) => {
                expect(err).not.toBeDefined();
                expect(pool.status.leased).toBe(LEASED - 1);
                done();
            });
        });
    });
});

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

const { forEach } = require("async");

describe("Pool Acquire/Release/Ready", () => {
    const setup = require("../utils/setup");
    const Pool = setup.Pool;
    const abapSystem = setup.abapSystem();
    const poolConfiguration = {
        connectionParameters: abapSystem,
        //clientOptions: { filter: 2, bcd: "number" },
        //poolOptions: { low: 2, high: 4 },
    };
    let pool;
    let acquired = [];

    beforeAll((done) => {
        pool = new Pool(poolConfiguration);
        done();
    });

    afterAll((done) => {
        pool.clearAll();
        done();
    });

    test("pool: acquire()", function () {
        expect.assertions(1);
        return pool.acquire().then((client) => {
            acquired.push(client);
            expect(client.alive).toBe(true);
        });
    });

    test("pool: acquire(multiple)", function () {
        expect.assertions(4);
        return pool.acquire(3).then((clients) => {
            acquired.push(...clients);
            expect(clients.length).toBe(3);
            clients.forEach((c) => {
                expect(c.alive).toBe(true);
            });
        });
    });

    test("pool: release(single)", function () {
        expect.assertions(1);
        const client = acquired.pop();
        const LEASED = pool.status.leased;
        return pool.release(client).then(() => {
            expect(acquired.length).toBe(LEASED - 1);
        });
    });

    test("pool: release(multiple)", function () {
        expect.assertions(1);
        const LEASED = pool.status.leased;
        const return_clients = acquired.slice(0, 2);
        return pool.release(return_clients).then(() => {
            expect(pool.status.leased).toBe(LEASED - 2);
        });
    });

    test("client: release()", function () {
        expect.assertions(1);
        const LEASED = pool.status.leased;
        const return_clients = acquired.pop();
        return return_clients.release().then(() => {
            expect(pool.status.leased).toBe(LEASED - 1);
        });
    });
});

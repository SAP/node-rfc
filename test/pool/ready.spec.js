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

    test("pool: ready()", function () {
        expect.assertions(2);
        expect(pool.status.ready).toBe(0);
        return pool.ready().then(() => {
            expect(pool.status.ready).toBe(2);
        });
    });

    test("pool: ready(5)", function () {
        expect.assertions(2);
        expect(pool.status.ready).toBe(2);
        return pool.ready(5).then(() => {
            expect(pool.status.ready).toBe(5);
        });
    });
});

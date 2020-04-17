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
const Pool = setup.rfcPool;
const abapSystem = setup.abapSystem;
const Promise = require("bluebird");

describe("Pool", () => {
    const pool = new Pool(abapSystem);

    test("Acquire single", function () {
        expect.assertions(3);
        return pool.acquire().then((client) => {
            expect(client.id).toBeGreaterThan(0);
            expect(client.isAlive).toBeTruthy();
            expect(pool.status.ready).toBe(1);
        });
    });

    test("Multiple acquire/release", function () {
        const promises = [];
        let id = new Set();
        const COUNT = 1;
        expect.assertions(COUNT + 1);
        for (let i = 0; i < COUNT; i++) {
            promises.push(
                pool.acquire().then((c) => {
                    expect(c.id).toBeGreaterThan(0);
                    id.add(c.id);
                })
            );
        }
        return Promise.all(promises).then(() => {
            expect(id.size).toEqual(COUNT);
        });
    });

    afterAll(function () {
        return pool.releaseAll();
    });
});

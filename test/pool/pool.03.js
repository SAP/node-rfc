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

module.exports = () => {
    const setup = require("../testutils/setup");
    const Pool = setup.rfcPool;
    const abapSystem = setup.abapSystem;

    const pool = new Pool(abapSystem);

    test("Acquire single", function () {
        expect.assertions(2);
        return pool.acquire().then((client) => {
            expect(client.id).toBeGreaterThan(0);
            expect(client.isAlive).toBeTruthy();
        });
    });

    test("Multiple acquire/release", function (done) {
        let ID = new Set();
        const COUNT = 10;
        expect.assertions(COUNT + 2);
        function test(client) {
            expect(client.id).toBeGreaterThan(0);
            ID.add(client.id);
            if (ID.size === COUNT) {
                expect(pool.status.active).toEqual(1);
                expect(pool.status.ready).toEqual(2);
                //console.log("pool", pool.status);
                done();
            }
        }
        for (let i = 0; i < COUNT; i++) {
            pool.acquire().then((c) => pool.release(c).then(() => test(c)));
        }
    });

    afterAll(function (done) {
        setTimeout(() => {
            pool.releaseAll().then((closed) => {
                //console.log("released", closed);
                done();
            });
        }, 2000);
    });
};

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

    test("pool: default options", function (done) {
        expect.assertions(6);
        const pool = new Pool(abapSystem);
        pool.acquire().then((client) => {
            expect(client.id).toBeGreaterThan(0);
            expect(client.isAlive).toBeTruthy();
            expect(pool.status.active).toBe(1);
            expect(client.options.bcd).toEqual("string");
            setTimeout(() => {
                pool.releaseAll().then((closed) => {
                    expect(pool.status.active).toBe(0);
                    expect(pool.status.ready).toBe(0);
                    done();
                });
            }, 2000);
        });
    });

    test("pool: bcd number", function (done) {
        expect.assertions(6);
        const pool = new Pool(abapSystem, undefined, {
            bcd: "number",
        });
        pool.acquire().then((client) => {
            expect(client.id).toBeGreaterThan(0);
            expect(client.isAlive).toBeTruthy();
            expect(pool.status.active).toBe(1);
            expect(client.options.bcd).toEqual("number");
            setTimeout(() => {
                pool.releaseAll().then((closed) => {
                    expect(pool.status.ready).toBe(0);
                    expect(pool.status.active).toBe(0);
                    done();
                });
            }, 2000);
        });
    });
};

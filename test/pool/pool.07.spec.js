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
const Promise = setup.Promise;

describe.skip("Pool", () => {
    const pool = new Pool(abapSystem);

    test("Unique client id across pools", function (done) {
        expect.assertions(4);
        let c1, c2;

        const pool2 = new Pool(abapSystem);

        pool.acquire().then((client) => {
            c1 = client;
            expect(c1.id).toBeGreaterThan(0);
            expect(c1.isAlive).toBeTruthy();
        });
        pool2.acquire().then((client) => {
            c2 = client;
            expect(c2.id).toBeGreaterThan(c1.id);
            expect(c2.isAlive).toBeTruthy();
        });
        Promise.all(promises).finally(() => done());
    });

    afterAll(function () {
        return pool.releaseAll();
    });
});

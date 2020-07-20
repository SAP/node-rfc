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

describe("Pool Options", () => {
    const setup = require("../utils/setup");
    const Pool = setup.Pool;
    const abapSystem = setup.abapSystem();

    test("pool: default pool options", function (done) {
        expect.assertions(2);
        const poolConfiguration = {
            connectionParameters: abapSystem,
            //clientOptions: { filter: 2, bcd: "number" },
            //poolOptions: { low: 2, high: 4 },
        };
        const pool = new Pool(poolConfiguration);

        expect(pool.config).toMatchObject({
            connectionParameters: abapSystem,
        });
        expect(pool.status).toMatchObject({ ready: 0, leased: 0 });
        pool.closeAll();
        done();
    });

    test("pool: low > 0", function (done) {
        expect.assertions(2);
        const poolConfiguration = {
            connectionParameters: abapSystem,
            //clientOptions: { filter: 2, bcd: "number" },
            poolOptions: { low: 3, high: 5 },
        };
        const pool = new Pool(poolConfiguration);

        expect(pool.config).toMatchObject({
            connectionParameters: abapSystem,
            poolOptions: { low: 3, high: 5 },
        });
        expect(pool.status).toMatchObject({ ready: 0, leased: 0 });
        pool.closeAll();
        done();
    });

    test("pool: ready", function () {
        expect.assertions(3);
        const poolConfiguration = {
            connectionParameters: abapSystem,
            //clientOptions: { filter: 2, bcd: "number" },
            //poolOptions: { low: 2, high: 4 },
        };
        const pool = new Pool(poolConfiguration);

        expect(pool.config).toMatchObject({
            connectionParameters: abapSystem,
        });
        expect(pool.status).toMatchObject({ ready: 0, leased: 0 });
        return pool.ready().then(() => {
            expect(pool.status).toMatchObject({ ready: 2, leased: 0 });
            pool.closeAll();
        });
        // done();
    });
});

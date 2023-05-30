// SPDX-FileCopyrightText: 2014 SAP SE Srdjan Boskovic <srdjan.boskovic@sap.com>
//
// SPDX-License-Identifier: Apache-2.0

import { Pool, abapSystem } from "../utils/setup";

describe("Pool Acquire/Release/Ready", () => {
    const poolConfiguration = {
        connectionParameters: abapSystem(),
        //clientOptions: { filter: 2, bcd: "number" },
        //poolOptions: { low: 2, high: 4 },
    };
    let pool = {} as Pool;

    beforeAll((done) => {
        pool = new Pool(poolConfiguration);
        done();
    });

    afterAll((done) => {
        done();
    });

    test("pool: ready()", function () {
        expect.assertions(2);
        expect(pool.status.ready).toBe(0);
        return (pool.ready() as Promise<void>).then(() => {
            expect(pool.status.ready).toBe(2);
        });
    });

    test("pool: ready(5)", function () {
        expect.assertions(2);
        expect(pool.status.ready).toBe(2);
        return (pool.ready(5) as Promise<void>).then(() => {
            expect(pool.status.ready).toBe(5);
        });
    });
});

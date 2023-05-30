// SPDX-FileCopyrightText: 2014 SAP SE Srdjan Boskovic <srdjan.boskovic@sap.com>
//
// SPDX-License-Identifier: Apache-2.0

import { Pool, abapSystem } from "../utils/setup";

describe("Pool Options", () => {
    test("pool: default pool options", async function () {
        expect.assertions(2);
        const poolConfiguration = {
            connectionParameters: abapSystem(),
            //clientOptions: { filter: 2, bcd: "number" },
            //poolOptions: { low: 2, high: 4 },
        };
        const pool = new Pool(poolConfiguration);

        expect(pool.config).toMatchObject({
            connectionParameters: abapSystem(),
        });
        expect(pool.status).toMatchObject({ ready: 0, leased: 0 });
        await pool.closeAll();
    });

    test("pool: low > 0", async function () {
        expect.assertions(2);
        const poolConfiguration = {
            connectionParameters: abapSystem(),
            //clientOptions: { filter: 2, bcd: "number" },
            poolOptions: { low: 3, high: 5 },
        };
        const pool = new Pool(poolConfiguration);

        expect(pool.config).toMatchObject({
            connectionParameters: abapSystem(),
            poolOptions: { low: 3, high: 5 },
        });
        expect(pool.status).toMatchObject({ ready: 0, leased: 0 });
        await pool.closeAll();
    });

    test("pool: ready", async function () {
        expect.assertions(3);
        const poolConfiguration = {
            connectionParameters: abapSystem(),
            //clientOptions: { filter: 2, bcd: "number" },
            //poolOptions: { low: 2, high: 4 },
        };
        const pool = new Pool(poolConfiguration);

        expect(pool.config).toMatchObject({
            connectionParameters: abapSystem(),
        });
        expect(pool.status).toMatchObject({ ready: 0, leased: 0 });
        await pool.ready();
        expect(pool.status).toMatchObject({ ready: 2, leased: 0 });
        await pool.closeAll();
    });
});

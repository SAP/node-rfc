// SPDX-FileCopyrightText: 2014 SAP SE Srdjan Boskovic <srdjan.boskovic@sap.com>
//
// SPDX-License-Identifier: Apache-2.0

import { Client, Pool, abapSystem } from "../utils/setup";

describe("Pool Acquire/Release", () => {
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

    test("pool: acquire()", async () => {
        expect.assertions(1);
        const client = (await pool.acquire()) as Client;
        expect(client.alive).toBe(true);
    });

    test("pool: acquire(multiple)", async () => {
        const N = 3;
        expect.assertions(N + 1);
        const clients = (await pool.acquire(N)) as Client[];
        expect(clients.length).toBe(N);
        clients.forEach((c) => {
            expect(c.alive).toBe(true);
        });
    });

    test("pool: release(single)", async () => {
        expect.assertions(1);
        const client = (await pool.acquire()) as Client;
        const LEASED = pool.status.leased;
        await pool.release(client);
        expect(pool.status.leased).toBe(LEASED - 1);
    });

    test("pool: release(multiple)", async () => {
        expect.assertions(2);
        const N = 3;
        const clients = (await pool.acquire(N)) as Client[];
        expect(clients.length).toBe(N);
        const LEASED = pool.status.leased;
        await pool.release(clients);
        expect(pool.status.leased).toBe(LEASED - N);
    });

    test("pool: release already released error", async () => {
        expect.assertions(1);
        const client = (await pool.acquire()) as Client;
        await pool.release(client);
        try {
            await pool.release(client);
        } catch (ex) {
            expect(ex).toMatchObject({
                message: "Client release() invoked for already closed client",
                name: "nodeRfcError",
            });
        }
    });

    test("client: release()", async () => {
        expect.assertions(1);
        const client = (await pool.acquire()) as Client;
        const LEASED = pool.status.leased;

        await client.release();
        expect(pool.status.leased).toBe(LEASED - 1);
    });
});

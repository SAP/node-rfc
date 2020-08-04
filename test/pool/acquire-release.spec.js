// SPDX-FileCopyrightText: 2014 SAP SE Srdjan Boskovic <srdjan.boskovic@sap.com>
//
// SPDX-License-Identifier: Apache-2.0

"use strict";

describe("Pool Acquire/Release", () => {
    const setup = require("../utils/setup");
    const Pool = setup.Pool;
    const abapSystem = setup.abapSystem();
    const poolConfiguration = {
        connectionParameters: abapSystem,
        //clientOptions: { filter: 2, bcd: "number" },
        //poolOptions: { low: 2, high: 4 },
    };
    let pool;

    beforeAll((done) => {
        pool = new Pool(poolConfiguration);
        done();
    });

    afterAll((done) => {
        done();
    });

    test("pool: acquire()", function (done) {
        expect.assertions(1);
        pool.acquire().then((client) => {
            expect(client.alive).toBe(true);
            done();
        });
    });

    test("pool: acquire(multiple)", function (done) {
        const N = 3;
        expect.assertions(N + 1);
        pool.acquire(N).then((clients) => {
            expect(clients.length).toBe(N);
            clients.forEach((c) => {
                expect(c.alive).toBe(true);
            });
            done();
        });
    });

    test("pool: release(single)", function (done) {
        expect.assertions(3);
        pool.acquire((err, client) => {
            expect(err).not.toBeDefined();
            const LEASED = pool.status.leased;
            pool.release(client, (err) => {
                expect(err).not.toBeDefined();
                expect(pool.status.leased).toBe(LEASED - 1);
                done();
            });
        });
    });

    test("pool: release(multiple)", function (done) {
        expect.assertions(4);
        const N = 3;
        pool.acquire(N, (err, clients) => {
            expect(err).not.toBeDefined();
            expect(clients.length).toBe(N);
            const LEASED = pool.status.leased;
            pool.release(clients, (err) => {
                expect(err).not.toBeDefined();
                expect(pool.status.leased).toBe(LEASED - N);
                done();
            });
        });
    });

    test("pool: release already released error", function (done) {
        expect.assertions(3);
        pool.acquire((err, client) => {
            expect(err).not.toBeDefined();
            pool.release(client, (err) => {
                expect(err).not.toBeDefined();
                pool.release(client, (err) => {
                    expect(err).toMatchObject({
                        message:
                            "Client release() invoked for already closed client",
                        name: "nodeRfcError",
                    });
                    done();
                });
            });
        });
    });

    test("client: release()", function (done) {
        expect.assertions(3);
        pool.acquire((err, client) => {
            const LEASED = pool.status.leased;
            expect(err).not.toBeDefined();
            client.release((err) => {
                expect(err).not.toBeDefined();
                expect(pool.status.leased).toBe(LEASED - 1);
                done();
            });
        });
    });
});

// SPDX-FileCopyrightText: 2014 SAP SE Srdjan Boskovic <srdjan.boskovic@sap.com>
//
// SPDX-License-Identifier: Apache-2.0

import { Pool, abapSystem } from "../utils/setup";

describe("Pool Errors", () => {
    // const poolConfiguration = {
    //     connectionParameters: abapSystem(),
    //     //clientOptions: { filter: 2, bcd: "number" },
    //     //poolOptions: { low: 2, high: 4 },
    // };

    test("pool error: connection parameters missing", function () {
        expect(() => new Pool({ connectionParameters: {} })).toThrow(
            new TypeError("Client connection parameters missing")
        );
    });

    test("pool error: user credentials w. callback", function (done) {
        //expect.assertions(1);
        const pool = new Pool({
            connectionParameters: abapSystem("MME_WRONG_USER"),
        });
        (pool.acquire as Function)(function (err) {
            expect(err).toMatchObject({
                name: "RfcLibError",
                group: 3,
                code: 2,
                codeString: "RFC_LOGON_FAILURE",
                key: "RFC_LOGON_FAILURE",
                message: "Name or password is incorrect (repeat logon)",
            });
            done();
        });
    });

    test("pool error: user credentials w. promise", function () {
        expect.assertions(1);
        const pool = new Pool({
            connectionParameters: abapSystem("MME_WRONG_USER"),
        });
        return expect(pool.acquire()).rejects.toEqual({
            name: "RfcLibError",
            group: 3,
            code: 2,
            codeString: "RFC_LOGON_FAILURE",
            key: "RFC_LOGON_FAILURE",
            message: "Name or password is incorrect (repeat logon)",
        });
    });
});

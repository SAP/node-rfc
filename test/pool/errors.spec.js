// SPDX-FileCopyrightText: 2014 SAP SE Srdjan Boskovic <srdjan.boskovic@sap.com>
//
// SPDX-License-Identifier: Apache-2.0

"use strict";

describe("Pool Errors", () => {
    const setup = require("../utils/setup");
    const Pool = setup.Pool;
    const abapSystem = setup.abapSystem();
    const poolConfiguration = {
        connectionParameters: abapSystem,
        //clientOptions: { filter: 2, bcd: "number" },
        //poolOptions: { low: 2, high: 4 },
    };

    test("pool error: connection parameters missing", function () {
        expect(() => new Pool({ connectionParameters: {} })).toThrow(
            new TypeError("Client connection parameters missing")
        );
    });

    test("pool error: user credentials w. callback", function (done) {
        //expect.assertions(1);
        const abapSystemWrongUser = Object.assign({}, abapSystem);
        abapSystemWrongUser.user += "#$!";
        const pool = new Pool({ connectionParameters: abapSystemWrongUser });
        pool.acquire((err) => {
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
        const abapSystemWrongUser = Object.assign({}, abapSystem);
        abapSystemWrongUser.user += "#$!";
        const pool = new Pool({ connectionParameters: abapSystemWrongUser });
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

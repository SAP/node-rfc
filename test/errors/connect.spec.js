// SPDX-FileCopyrightText: 2014 SAP SE Srdjan Boskovic <srdjan.boskovic@sap.com>
//
// SPDX-License-Identifier: Apache-2.0

"use strict";

describe("Errors: Connect", () => {
    const setup = require("../utils/setup");
    const client = setup.direct_client();

    test("error: new client requires connection parameters", function () {
        expect.assertions(1);
        return expect(() => new setup.Client()).toThrow(
            new TypeError("Client constructor requires an argument")
        );
    });

    test("error: connect() requires minimum of connection parameters", function (done) {
        expect.assertions(2);
        let wrongParams = { user: "X", passwd: "Y", client: "620", lang: "en" };
        let wrongClient = setup.direct_client(wrongParams);
        wrongClient.connect(function (err) {
            expect(err).toBeDefined();
            expect(err).toMatchObject({
                code: 20,
                codeString: "RFC_INVALID_PARAMETER",
                group: 5,
                key: "RFC_INVALID_PARAMETER",
                message: "Parameter ASHOST, GWHOST, MSHOST or PORT is missing.",
                name: "RfcLibError",
            });
            done();
        });
    });

    test("error: conect() rejects invalid credentials", function (done) {
        expect.assertions(2);
        let wrongParams = Object.assign({}, setup.abapSystem());
        wrongParams.user = "WRONGUSER";

        let wrongClient = setup.direct_client(wrongParams);
        wrongClient.connect(function (err) {
            expect(err).toBeDefined();
            expect(err).toMatchObject({
                message: "Name or password is incorrect (repeat logon)",
                code: 2,
                key: "RFC_LOGON_FAILURE",
            });
            done();
        });
    });

    test("error: close() over closed connection", function (done) {
        expect.assertions(2);
        client.close((err) => {
            expect(err).toMatchObject({
                name: "nodeRfcError",
                message: "RFM client request over closed connection: close()",
            });
            expect(client.alive).toBe(false);
            done();
        });
    });
});

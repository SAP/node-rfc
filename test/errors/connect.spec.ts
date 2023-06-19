// SPDX-FileCopyrightText: 2014 SAP SE Srdjan Boskovic <srdjan.boskovic@sap.com>
//
// SPDX-License-Identifier: Apache-2.0

import { direct_client, Client } from "../utils/setup";

describe("Errors: Connect", () => {
    const client = direct_client();

    test("error: new client requires connection parameters", function () {
        expect.assertions(1);
        return expect(() => new Client({})).toThrow(
            new TypeError("Client connection parameters missing")
        );
    });

    test("error: connect() requires minimum of connection parameters", function (done) {
        expect.assertions(2);

        const wrongClient = direct_client("MME_NO_ASHOST");
        void wrongClient.connect(function (err) {
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
        const wrongClient = direct_client("MME_WRONG_USER");
        void wrongClient.connect(function (err) {
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
        void client.close((err) => {
            expect(err).toMatchObject({
                name: "nodeRfcError",
                message: "RFM client request over closed connection: close()",
            });
            expect(client.alive).toBe(false);
            done();
        });
    });
});

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
            expect(err).toMatchObject(
                new Error(
                    "Client 1 is already closed; see https://github.com/SAP/node-rfc#usage"
                )
            );
            expect(client.alive).toBe(false);
            done();
        });
    });
});

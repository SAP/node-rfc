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

module.exports = () => {
    const setup = require("../testutils/setup");
    const client = setup.client();

    beforeEach(function (done) {
        client.reopen(function (err) {
            done(err);
        });
    });

    afterEach(function (done) {
        client.close(function () {
            done();
        });
    });

    test("error: invoke() requires at least three arguments", function () {
        expect(() => client.invoke("rfc", {})).toThrow(
            new Error("Callback function must be supplied")
        );
    });

    test("error: invoke() rejects non-string rfm name", function (done) {
        client.invoke(23, {}, function (err) {
            expect(err).toBeDefined();
            expect(err).toEqual(
                expect.objectContaining({
                    name: "TypeError",
                    message:
                        "First argument (remote function module name) must be an string",
                })
            );
            done();
        });
    });

    test("error: invoke() rejects non-existing parameter", function (done) {
        client.invoke(
            "STFC_CONNECTION",
            {
                XXX: "wrong param",
            },
            function (err) {
                expect(err).toBeDefined();
                expect(err).toEqual(
                    expect.objectContaining({
                        code: 20,
                        key: "RFC_INVALID_PARAMETER",
                        message: "field 'XXX' not found",
                    })
                );
                done();
            }
        );
    });

    test("error: non-existing field in input structure", function (done) {
        let importStruct = {
            XRFCCHAR1: "A",
            RFCCHAR2: "BC",
            RFCCHAR4: "DEFG",
        };

        client.invoke(
            "STFC_STRUCTURE",
            {
                IMPORTSTRUCT: importStruct,
            },
            function (err) {
                expect(err).toBeDefined();
                expect(err).toEqual(
                    expect.objectContaining({
                        name: "RfcLibError",
                        code: 20,
                        key: "RFC_INVALID_PARAMETER",
                        message: "field 'XRFCCHAR1' not found",
                    })
                );
                done();
            }
        );
    });

    test("error: non-existing field in input table", function (done) {
        let importTable = [
            {
                XRFCCHAR1: "A",
                RFCCHAR2: "BC",
                RFCCHAR4: "DEFG",
            },
        ];

        client.invoke(
            "STFC_STRUCTURE",
            {
                RFCTABLE: importTable,
            },
            function (err) {
                expect(err).toBeDefined();
                expect(err).toEqual(
                    expect.objectContaining({
                        name: "RfcLibError",
                        code: 20,
                        key: "RFC_INVALID_PARAMETER",
                        message: "field 'XRFCCHAR1' not found",
                    })
                );
                done();
            }
        );
    });

    test("error: invoke() over closed connection", function (done) {
        (async () => {
            try {
                await client.close();
                expect(client.isAlive).toEqual(false);
                await client.call("STFC_CONNECTION", {
                    REQUTEXT: setup.UNICODETEST,
                });
            } catch (ex) {
                expect(ex).toEqual(
                    new Error(
                        `Client invoked RFC call with closed connection: id=${client.id}`
                    )
                );
                done();
            }
        })();
    });
};

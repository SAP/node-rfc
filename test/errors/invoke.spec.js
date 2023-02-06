// SPDX-FileCopyrightText: 2014 SAP SE Srdjan Boskovic <srdjan.boskovic@sap.com>
//
// SPDX-License-Identifier: Apache-2.0

"use strict";

describe("Errors: Invoke", () => {
    const setup = require("../utils/setup");
    const client = setup.direct_client();

    beforeEach(function (done) {
        client.open(function (err) {
            done(err);
        });
    });

    afterEach(function (done) {
        client.close(function () {
            done();
        });
    });

    test("error: invoke() rfm parameter name must not be empty", function (done) {
        expect.assertions(1);
        client.invoke("STFC_CONNECTION", { "": "" }, function (err, res) {
            expect(err).toMatchObject(
                new TypeError(`Empty RFM parameter name when calling "STFC_CONNECTION"`)
            );
            done();
        });
    });

    test("error: invoke() rfm parameter name must be a valid string", function (done) {
        expect.assertions(1);
        client.invoke("STFC_CONNECTION", { "%$": "" }, function (err, res) {
            expect(err).toMatchObject(
                new TypeError(`RFM parameter name invalid: "%$" when calling "STFC_CONNECTION"`)
            );
            done();
        });
    });

    test("error: invoke() requires at least three arguments", function () {
        return expect(() => client.invoke("rfc", {})).toThrow(
            new Error("Callback function must be supplied")
        );
    });

    test("error: invoke() rejects non-string rfm name", function (done) {
        /*
        return expect(() => client.invoke(23, {})).toThrow(
            new TypeError(
                "Client invoke() 1st argument (remote function module name) must be an string"
            )
        );
        */
        client.invoke(23, {}, function (err) {
            expect(err).toMatchObject(
                new TypeError(
                    "Client invoke() 1st argument (remote function module name) must be an string"
                )
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
                expect(err).toMatchObject({
                    code: 20,
                    key: "RFC_INVALID_PARAMETER",
                    message: "field 'XXX' not found",
                });
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
                expect(err).toMatchObject({
                    name: "RfcLibError",
                    code: 20,
                    key: "RFC_INVALID_PARAMETER",
                    message: "field 'XRFCCHAR1' not found",
                });
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
                expect(err).toMatchObject({
                    name: "RfcLibError",
                    code: 20,
                    key: "RFC_INVALID_PARAMETER",
                    message: "field 'XRFCCHAR1' not found",
                });
                done();
            }
        );
    });

    test("error: close() closed connection", function (done) {
        expect.assertions(3);
        client.close((err) => {
            expect(err).not.toBeDefined();
            expect(client.alive).toBe(false);
            client.close((err) => {
                expect(err).toEqual(
                    expect.objectContaining({
                        message:
                            "RFM client request over closed connection: close()",
                        name: "nodeRfcError",
                    })
                );
                done();
            });
        });
    });

    test("error: invoke() over closed connection", function (done) {
        expect.assertions(2);
        client.close(() => {
            expect(client.alive).toBe(false);
            client.invoke(
                "STFC_CONNECTION",
                { REQUTEXT: setup.UNICODETEST },
                function (err, res) {
                    expect(err).toEqual(
                        expect.objectContaining({
                            message:
                                "RFM client request over closed connection: invoke() STFC_CONNECTION",
                            name: "nodeRfcError",
                        })
                    );
                    done();
                }
            );
        });
    });

    test("error: call() over closed connection", async () => {
        expect.assertions(2);
        try {
            await client.close();
            expect(client.alive).toEqual(false);
            await client.call("STFC_CONNECTION", {
                REQUTEXT: setup.UNICODETEST,
            });
        } catch (ex) {
            expect(ex).toEqual(
                expect.objectContaining({
                    message:
                        "RFM client request over closed connection: invoke() STFC_CONNECTION",
                    name: "nodeRfcError",
                })
            );
        }
    });
});

// SPDX-FileCopyrightText: 2014 SAP SE Srdjan Boskovic <srdjan.boskovic@sap.com>
//
// SPDX-License-Identifier: Apache-2.0

import { direct_client, UNICODETEST } from "../utils/setup";

describe("Errors: Invoke", () => {
    const client = direct_client();

    beforeEach(function (done) {
        void client.open((err: unknown) => {
            done(err);
        });
    });

    afterEach(function (done) {
        void client.close(() => {
            done();
        });
    });

    test("error: invoke() rfm parameter name must not be empty", function (done) {
        expect.assertions(1);
        client.invoke("STFC_CONNECTION", { "": "" }, function (err: unknown) {
            expect(err).toMatchObject(
                new TypeError(
                    `Empty RFM parameter name when calling "STFC_CONNECTION"`
                )
            );
            done();
        });
    });

    test("error: invoke() rfm parameter name must be a valid string", function (done) {
        expect.assertions(1);
        client.invoke("STFC_CONNECTION", { "%$": "" }, function (err: unknown) {
            expect(err).toMatchObject(
                new TypeError(
                    `RFM parameter name invalid: "%$" when calling "STFC_CONNECTION"`
                )
            );
            done();
        });
    });

    /*
    // error can tested with javascript only
    test.skip("error: invoke() requires at least three arguments", function () {
        return expect(() => client.invoke("rfc", {})).toThrow(
            new Error("Callback function must be supplied")
        );
    });

    // error can tested with javascript only
    test.skip("error: invoke() rejects non-string rfm name", function (done) {

        // return expect(() => client.invoke(23, {})).toThrow(
        //     new TypeError(
        //         "Client invoke() 1st argument (remote function module name) must be an string"
        //     )
        // );

        client.invoke(23, {}, function (err) {
            expect(err).toMatchObject(
                new TypeError(
                    "Client invoke() 1st argument (remote function module name) must be an string"
                )
            );
            done();
        });
    });
    */

    test("error: invoke() rejects non-existing parameter", function (done) {
        client.invoke(
            "STFC_CONNECTION",
            {
                XXX: "wrong param",
            },
            function (err: unknown) {
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
        const importStruct = {
            XRFCCHAR1: "A",
            RFCCHAR2: "BC",
            RFCCHAR4: "DEFG",
        };

        client.invoke(
            "STFC_STRUCTURE",
            {
                IMPORTSTRUCT: importStruct,
            },
            function (err: unknown) {
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
        const importTable = [
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
        void client.close((err: unknown) => {
            expect(err).not.toBeDefined();
            expect(client.alive).toBe(false);
            void client.close((err: unknown) => {
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
        void client.close(() => {
            expect(client.alive).toBe(false);
            client.invoke(
                "STFC_CONNECTION",
                { REQUTEXT: UNICODETEST },
                function (err: unknown) {
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
                REQUTEXT: UNICODETEST,
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

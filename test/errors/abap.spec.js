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

const setup = require("../setup");
const client = setup.client();

beforeAll(function (done) {
    client.connect(function (err) {
        done(err);
    });
});

afterAll(function (done) {
    client.close(function (err) {
        done(err);
    });
});

describe("Errors: ABAP", () => {
    //
    // RFC_ABAP_RUNTIME_FAILURE ///< SAP system runtime error (SYSTEM_FAILURE): Shortdump on the backend side
    //

    test("error: invoke() AbapApplicationError E3", function (done) {
        client.invoke(
            "RFC_RAISE_ERROR",
            {
                METHOD: "3",
                MESSAGETYPE: "E",
            },
            function (err) {
                expect(err).toEqual(
                    expect.objectContaining({
                        alive: true,
                        code: 3,
                        codeString: "RFC_ABAP_RUNTIME_FAILURE",
                        name: "ABAPError",
                        key: "COMPUTE_INT_ZERODIVIDE",
                        message: "Division by 0 (type I or INT8)",
                    })
                );
                done();
            }
        );
    });

    test("error: invoke() AbapApplicationError E51", function (done) {
        client.invoke(
            "RFC_RAISE_ERROR",
            {
                METHOD: "51",
                MESSAGETYPE: "E",
            },
            function (err) {
                expect(err).toEqual(
                    expect.objectContaining({
                        alive: true,
                        code: 3,
                        codeString: "RFC_ABAP_RUNTIME_FAILURE",
                        name: "ABAPError",
                        key: "BLOCKED_COMMIT",
                        message:
                            "A database commit was blocked by the application.",
                    })
                );
                done();
            }
        );
    });

    test("error: invoke() SAP GUI in background", function (done) {
        client.invoke("STFC_SAPGUI", {}, function (err) {
            expect(err).toEqual(
                expect.objectContaining({
                    alive: true,
                    code: 3,
                    codeString: "RFC_ABAP_RUNTIME_FAILURE",
                    name: "ABAPError",
                    key: "DYNPRO_SEND_IN_BACKGROUND",
                    message: "Screen output without connection to user.",
                })
            );
            done();
        });
    });

    test("error: invoke() AbapApplicationError E5", function (done) {
        client.invoke(
            "RFC_RAISE_ERROR",
            {
                METHOD: "5",
                MESSAGETYPE: "E",
            },
            function (err) {
                expect(err).toEqual(
                    expect.objectContaining({
                        alive: true,
                        name: "ABAPError",
                        code: 3,
                        codeString: "RFC_ABAP_RUNTIME_FAILURE",
                        key: "SYNTAX_ERROR",
                        message:
                            "Syntax error in program RSXERROR                                .",
                        abapMsgClass: "",
                        abapMsgType: "",
                        abapMsgNumber: "",
                        abapMsgV1: "",
                        abapMsgV2: "",
                        abapMsgV3: "",
                        abapMsgV4: "",
                    })
                );
                done();
            }
        );
    });

    //
    // RFC_ABAP_MESSAGE ///< The called function module raised an E-, A- or X-Message
    //

    test("error: invoke() AbapApplicationError E0", function (done) {
        client.invoke(
            "RFC_RAISE_ERROR",
            {
                METHOD: "0",
                MESSAGETYPE: "E",
            },
            function (err) {
                expect(err).toEqual(
                    expect.objectContaining({
                        alive: true,
                        code: 4,
                        codeString: "RFC_ABAP_MESSAGE",
                        name: "ABAPError",
                        key: "Function not supported",
                        message: "Function not supported",
                        abapMsgClass: "SR",
                        abapMsgType: "E",
                        abapMsgNumber: "006",
                    })
                );
                done();
            }
        );
    });

    test("error: invoke() AbapApplicationError E36", function (done) {
        client.invoke(
            "RFC_RAISE_ERROR",
            {
                METHOD: "36",
                MESSAGETYPE: "E",
            },
            function (err) {
                expect(err).toEqual(
                    expect.objectContaining({
                        alive: true,
                        code: 4,
                        codeString: "RFC_ABAP_MESSAGE",
                        name: "ABAPError",
                        key: "Division by 0 (type I or INT8)",
                        message: "Division by 0 (type I or INT8)",
                        abapMsgClass: "SR",
                        abapMsgType: "E",
                        abapMsgNumber: "000",
                        abapMsgV1: "Division by 0 (type I or INT8)",
                    })
                );
                done();
            }
        );
    });

    test("error: invoke() AbapRuntimeError A", function (done) {
        client.invoke(
            "RFC_RAISE_ERROR",
            {
                MESSAGETYPE: "A",
            },
            function (err, res) {
                expect(err).toEqual(
                    expect.objectContaining({
                        alive: true,
                        code: 4,
                        codeString: "RFC_ABAP_MESSAGE",
                        name: "ABAPError",
                        key: "Function not supported",
                        message: "Function not supported",
                        abapMsgClass: "SR",
                        abapMsgType: "A",
                        abapMsgNumber: "006",
                    })
                );
                done();
            }
        );
    });

    test("error: invoke() AbapRuntimeError X", function (done) {
        client.invoke(
            "RFC_RAISE_ERROR",
            {
                MESSAGETYPE: "X",
            },
            function (err) {
                expect(err).toEqual(
                    expect.objectContaining({
                        alive: true,
                        code: 4,
                        codeString: "RFC_ABAP_MESSAGE",
                        name: "ABAPError",
                        key: "MESSAGE_TYPE_X",
                        message:
                            "The current application has triggered a termination with a short dump.",
                        abapMsgClass: "00",
                        abapMsgType: "X",
                        abapMsgNumber: "341",
                        abapMsgV1: "MESSAGE_TYPE_X",
                    })
                );
                done();
            }
        );
    });

    //
    // RFC_ABAP_EXCEPTION ///< The called function module raised an Exception (RAISE or MESSAGE ... RAISING)
    //

    test("error: invoke() AbapApplicationError E1", function (done) {
        client.invoke(
            "RFC_RAISE_ERROR",
            {
                METHOD: "1",
                MESSAGETYPE: "E",
            },
            function (err) {
                expect(err).toEqual(
                    expect.objectContaining({
                        alive: true,
                        code: 5,
                        codeString: "RFC_ABAP_EXCEPTION",
                        name: "ABAPError",
                        key: "RAISE_EXCEPTION",
                        abapMsgClass: "SR",
                        abapMsgType: "E",
                        abapMsgNumber: "006",
                    })
                );
                done();
            }
        );
    });

    test("error: invoke() AbapApplicationError E2", function (done) {
        client.connect((err) => {
            client.invoke(
                "RFC_RAISE_ERROR",
                {
                    METHOD: "2",
                    MESSAGETYPE: "E",
                },
                function (err) {
                    expect(err).toEqual(
                        expect.objectContaining({
                            alive: true,
                            code: 5,
                            codeString: "RFC_ABAP_EXCEPTION",
                            name: "ABAPError",
                            key: "RAISE_EXCEPTION",
                            abapMsgNumber: "000",
                        })
                    );
                    done();
                }
            );
        });
    });
});

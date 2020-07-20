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

// Closed if
// code == RFC_COMMUNICATION_FAILURE || // Error in Network & Communication layer.
// code == RFC_ABAP_RUNTIME_FAILURE ||  // SAP system runtime error (SYSTEM_FAILURE): Shortdump on the backend side.
// code == RFC_ABAP_MESSAGE ||          // The called function module raised an E-, A- or X-Message.
// code == RFC_EXTERNAL_FAILURE
// or
// group == ABAP_RUNTIME_FAILURE ||  // ABAP Message raised in ABAP function modules or in ABAP runtime of the backend (e.g Kernel)
// group == LOGON_FAILURE ||         // Error message raised when logon fails
// group == COMMUNICATION_FAILURE || // Problems with the network connection (or backend broke down and killed the connection)
// group == EXTERNAL_RUNTIME_FAILURE // Problems in the RFC runtime of the external program (i.e "this" library)

describe("Errors: ABAP", () => {
    const setup = require("../utils/setup");
    const client = setup.direct_client();

    beforeEach(function (done) {
        client.connect(function (err) {
            done(err);
        });
    });

    afterEach(function (done) {
        if (client.alive) {
            client.close(function (err) {
                done(err);
            });
        } else {
            done();
        }
    });

    //
    // RFC_ABAP_RUNTIME_FAILURE ///< SAP system runtime error (SYSTEM_FAILURE): Shortdump on the backend side
    //

    test("error: invoke() AbapApplicationError E3", function (done) {
        expect.assertions(3);
        const old_handle = client.connectionHandle;
        client.invoke(
            "RFC_RAISE_ERROR",
            {
                METHOD: "3",
                MESSAGETYPE: "E",
            },
            function (err) {
                expect(client.alive).toBe(true);
                expect(client.connectionHandle).not.toBe(old_handle);
                expect(err).toMatchObject({
                    code: 3,
                    codeString: "RFC_ABAP_RUNTIME_FAILURE",
                    group: 2,
                    key: "COMPUTE_INT_ZERODIVIDE",
                    message: "Division by 0 (type I or INT8)",
                    name: "ABAPError",
                });
                done();
            }
        );
    });

    test("error: invoke() AbapApplicationError E51", function (done) {
        expect.assertions(3);
        const old_handle = client.connectionHandle;
        client.invoke(
            "RFC_RAISE_ERROR",
            {
                METHOD: "51",
                MESSAGETYPE: "E",
            },
            function (err) {
                expect(client.alive).toBe(true);
                expect(client.connectionHandle).not.toBe(old_handle);
                expect(err).toMatchObject({
                    code: 3,
                    codeString: "RFC_ABAP_RUNTIME_FAILURE",
                    group: 2,
                    key: "BLOCKED_COMMIT",
                    message:
                        "A database commit was blocked by the application.",
                    name: "ABAPError",
                });
                done();
            }
        );
    });

    test("error: invoke() SAP GUI in background", function (done) {
        expect.assertions(3);
        const old_handle = client.connectionHandle;
        client.invoke("STFC_SAPGUI", {}, function (err) {
            expect(client.alive).toBe(true);
            expect(client.connectionHandle).not.toBe(old_handle);
            expect(err).toMatchObject({
                code: 3,
                codeString: "RFC_ABAP_RUNTIME_FAILURE",
                group: 2,
                key: "DYNPRO_SEND_IN_BACKGROUND",
                message: "Screen output without connection to user.",
                name: "ABAPError",
            });
            done();
        });
    });

    test("error: invoke() AbapApplicationError E5", function (done) {
        expect.assertions(3);
        const old_handle = client.connectionHandle;
        client.invoke(
            "RFC_RAISE_ERROR",
            {
                METHOD: "5",
                MESSAGETYPE: "E",
            },
            function (err) {
                expect(client.alive).toBe(true);
                expect(client.connectionHandle).not.toBe(old_handle);
                expect(err).toMatchObject({
                    code: 3,
                    codeString: "RFC_ABAP_RUNTIME_FAILURE",
                    group: 2,
                    key: "SYNTAX_ERROR",
                    message:
                        "Syntax error in program RSXERROR                                .",
                    name: "ABAPError",
                });
                done();
            }
        );
    });

    //
    // RFC_ABAP_MESSAGE ///< The called function module raised an E-, A- or X-Message
    //

    test("error: invoke() AbapApplicationError E0", function (done) {
        expect.assertions(3);
        const old_handle = client.connectionHandle;
        client.invoke(
            "RFC_RAISE_ERROR",
            {
                METHOD: "0",
                MESSAGETYPE: "E",
            },
            function (err) {
                expect(client.alive).toBe(true);
                expect(client.connectionHandle).not.toBe(old_handle);
                expect(err).toMatchObject({
                    abapMsgClass: "SR",
                    abapMsgNumber: "006",
                    abapMsgType: "E",
                    //abapMsgV1: "STRING" | "Method = 0",
                    code: 4,
                    codeString: "RFC_ABAP_MESSAGE",
                    group: 2,
                    key: "Function not supported",
                    message: "Function not supported",
                    name: "ABAPError",
                });
                done();
            }
        );
    });

    test("error: invoke() AbapApplicationError E36", function (done) {
        expect.assertions(3);
        const old_handle = client.connectionHandle;
        client.invoke(
            "RFC_RAISE_ERROR",
            {
                METHOD: "36",
                MESSAGETYPE: "E",
            },
            function (err) {
                expect(client.alive).toBe(true);
                expect(client.connectionHandle).not.toBe(old_handle);
                expect(err).toMatchObject({
                    abapMsgClass: "SR",
                    abapMsgNumber: "000",
                    abapMsgType: "E",
                    abapMsgV1: "Division by 0 (type I or INT8)",
                    abapMsgV2: "",
                    abapMsgV3: "",
                    abapMsgV4: "",
                    code: 4,
                    codeString: "RFC_ABAP_MESSAGE",
                    group: 2,
                    key: "Division by 0 (type I or INT8)",
                    message: "Division by 0 (type I or INT8)",
                    name: "ABAPError",
                });
                done();
            }
        );
    });

    test("error: invoke() AbapRuntimeError A", function (done) {
        expect.assertions(3);
        const old_handle = client.connectionHandle;
        client.invoke(
            "RFC_RAISE_ERROR",
            {
                MESSAGETYPE: "A",
            },
            function (err, res) {
                expect(client.alive).toBe(true);
                expect(client.connectionHandle).not.toBe(old_handle);
                expect(err).toMatchObject({
                    abapMsgClass: "SR",
                    abapMsgNumber: "006",
                    abapMsgType: "A",
                    //abapMsgV1: "STRING",
                    code: 4,
                    codeString: "RFC_ABAP_MESSAGE",
                    group: 2,
                    key: "Function not supported",
                    message: "Function not supported",
                    name: "ABAPError",
                });
                done();
            }
        );
    });

    test("error: invoke() AbapRuntimeError X", function (done) {
        expect.assertions(3);
        const old_handle = client.connectionHandle;
        client.invoke(
            "RFC_RAISE_ERROR",
            {
                MESSAGETYPE: "X",
            },
            function (err) {
                expect(client.alive).toBe(true);
                expect(client.connectionHandle).not.toBe(old_handle);
                expect(err).toMatchObject({
                    abapMsgClass: "00",
                    abapMsgNumber: "341",
                    abapMsgType: "X",
                    abapMsgV1: "MESSAGE_TYPE_X",
                    abapMsgV2: "",
                    abapMsgV3: "",
                    abapMsgV4: "",
                    code: 4,
                    codeString: "RFC_ABAP_MESSAGE",
                    group: 2,
                    key: "MESSAGE_TYPE_X",
                    message:
                        "The current application has triggered a termination with a short dump.",
                    name: "ABAPError",
                });
                done();
            }
        );
    });

    //
    // RFC_ABAP_EXCEPTION ///< The called function module raised an Exception (RAISE or MESSAGE ... RAISING)
    //

    test("error: invoke() AbapApplicationError E1", function (done) {
        expect.assertions(2);
        client.invoke(
            "RFC_RAISE_ERROR",
            {
                METHOD: "1",
                MESSAGETYPE: "E",
            },
            function (err) {
                expect(client.alive).toBe(true);
                expect(err).toMatchObject({
                    abapMsgClass: "SR",
                    abapMsgNumber: "006",
                    abapMsgType: "E",
                    abapMsgV1: "Method = 1",
                    abapMsgV2: "",
                    abapMsgV3: "",
                    abapMsgV4: "",
                    code: 5,
                    codeString: "RFC_ABAP_EXCEPTION",
                    group: 1,
                    key: "RAISE_EXCEPTION",
                    message: "ID:SR Type:E Number:006 Method = 1",
                    name: "ABAPError",
                });
                done();
            }
        );
    });

    test("error: invoke() AbapApplicationError E2", function (done) {
        expect.assertions(2);
        client.invoke(
            "RFC_RAISE_ERROR",
            {
                METHOD: "2",
                MESSAGETYPE: "E",
            },
            function (err) {
                expect(client.alive).toBe(true);
                expect(err).toMatchObject({
                    code: 5,
                    codeString: "RFC_ABAP_EXCEPTION",
                    group: 1,
                    key: "RAISE_EXCEPTION",
                    message: " Number:000",
                    name: "ABAPError",
                });
                done();
            }
        );
    });
});

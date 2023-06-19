// SPDX-FileCopyrightText: 2014 SAP SE Srdjan Boskovic <srdjan.boskovic@sap.com>
//
// SPDX-License-Identifier: Apache-2.0

import { direct_client, abapSystem, Client, Pool } from "../utils/setup";

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
    const client = direct_client();

    beforeEach((done) => {
        void client.connect((err: unknown) => {
            done(err);
        });
    });

    afterEach((done) => {
        if (client.alive) {
            void client.close((err: unknown) => {
                done(err);
            });
        } else {
            done();
        }
    });

    //
    // RFC_ABAP_RUNTIME_FAILURE ///< SAP system runtime error (SYSTEM_FAILURE): Shortdump on the backend side
    //

    test("error: invoke() AbapApplicationError E3", (done) => {
        expect.assertions(3);
        const old_handle = client.connectionHandle;
        client.invoke(
            "RFC_RAISE_ERROR",
            {
                METHOD: "3",
                MESSAGETYPE: "E",
            },
            function (err: unknown) {
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

    test("error: invoke() AbapApplicationError E51 (direct)", (done) => {
        expect.assertions(3);
        const old_handle = client.connectionHandle;
        client.invoke(
            "RFC_RAISE_ERROR",
            {
                METHOD: "51",
                MESSAGETYPE: "E",
            },
            (err: unknown) => {
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

    test("error: invoke() AbapApplicationError E51 (pool)", async () => {
        expect.assertions(3);
        const poolConfiguration = {
            connectionParameters: abapSystem(),
        };
        const pool = new Pool(poolConfiguration);
        const client = (await pool.acquire()) as Client;
        const old_handle = client.connectionHandle;
        try {
            await client.call("RFC_RAISE_ERROR", {
                METHOD: "51",
                MESSAGETYPE: "E",
            });
        } catch (ex) {
            expect(client.alive).toBe(true);
            expect(client.connectionHandle).not.toBe(old_handle);
            expect(ex).toMatchObject({
                code: 3,
                codeString: "RFC_ABAP_RUNTIME_FAILURE",
                group: 2,
                key: "BLOCKED_COMMIT",
                message: "A database commit was blocked by the application.",
                name: "ABAPError",
            });
            await pool.release(client);
        }
    });

    test("error: invoke() SAP GUI in background", function (done) {
        expect.assertions(3);
        const old_handle = client.connectionHandle;
        client.invoke("STFC_SAPGUI", {}, function (err: unknown) {
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
            function (err: unknown) {
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
            function (err: unknown) {
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
            function (err: unknown) {
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
            function (err: unknown) {
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
            function (err: unknown) {
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
            function (err: unknown) {
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
            function (err: unknown) {
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

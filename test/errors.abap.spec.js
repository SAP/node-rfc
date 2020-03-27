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

const setup = require("./setup");
const client = setup.client;
const sync = setup.sync;

// --detectOpenHandles --runInBand

beforeAll(function (done) {
    client.connect(function (err) {
        done(err);
    });
});

afterAll(function (done) {
    client.close(function (err) {
        done();
    });
});

it("error: invoke() AbapApplicationError E0", function (done) {
    client.invoke(
        "RFC_RAISE_ERROR", {
            METHOD: "0",
            MESSAGETYPE: "E"
        },
        function (err) {
            expect(err).toBeDefined();
            expect(err).toEqual(
                expect.objectContaining({
                    code: 4,
                    name: "ABAPError",
                    key: "Function not supported",
                    message: "Function not supported",
                    abapMsgClass: "SR",
                    abapMsgType: "E",
                    abapMsgNumber: "006"
                })
            );
            // Assures that the connection handle is correctly synchronized
            sync(client, done);
        }
    );
});

it("error: invoke() AbapApplicationError E1", function (done) {
    client.invoke(
        "RFC_RAISE_ERROR", {
            METHOD: "1",
            MESSAGETYPE: "E"
        },
        function (err) {
            expect(err).toBeDefined();
            expect(err).toEqual(
                expect.objectContaining({
                    code: 5,
                    name: "ABAPError",
                    key: "RAISE_EXCEPTION",
                    abapMsgClass: "SR",
                    abapMsgType: "E",
                    abapMsgNumber: "006"
                })
            );
            // Assures that the connection handle is correctly synchronized
            sync(client, done);
        }
    );
});

it("error: invoke() AbapApplicationError E2", function (done) {
    client.connect((err, res) => {
        if (err) {
            done(err);
        } else {
            client.invoke(
                "RFC_RAISE_ERROR", {
                    METHOD: "2",
                    MESSAGETYPE: "E"
                },
                function (err) {
                    expect(err).toBeDefined();
                    expect(err).toEqual(
                        expect.objectContaining({
                            code: 5,
                            name: "ABAPError",
                            key: "RAISE_EXCEPTION",
                            abapMsgNumber: "000"
                        })
                    );
                    // Assures that the connection handle is correctly synchronized
                    sync(client, done);
                }
            );
        }
    });
});

it("error: invoke() AbapApplicationError E3", function (done) {
    client.invoke(
        "RFC_RAISE_ERROR", {
            METHOD: "3",
            MESSAGETYPE: "E"
        },
        function (err) {
            expect(err).toBeDefined();
            expect(err).toEqual(
                expect.objectContaining({
                    code: 3,
                    name: "ABAPError",
                    key: "COMPUTE_INT_ZERODIVIDE",
                    message: "Division by 0 (type I or INT8)"
                })
            );
            // Assures that the connection handle is correctly synchronized
            sync(client, done);
        }
    );
});

it("error: invoke() AbapApplicationError E11", function (done) {
    client.invoke(
        "RFC_RAISE_ERROR", {
            METHOD: "11",
            MESSAGETYPE: "E"
        },
        function (rec, err) {
            expect(err).toBeDefined();
            expect(err).toEqual(
                expect.objectContaining({
                    COUNTER: 1,
                    CSTRING: "",
                    RETURN_VALUE: "",
                    MESSAGETYPE: "E",
                    METHOD: "11",
                    PARAMETER: ""
                })
            );
            // Assures that the connection handle is correctly synchronized
            sync(client, done);
        }
    );
});

it("error: invoke() AbapApplicationError E36", function (done) {
    client.invoke(
        "RFC_RAISE_ERROR", {
            METHOD: "36",
            MESSAGETYPE: "E"
        },
        function (err) {
            expect(err).toBeDefined();
            expect(err).toEqual(
                expect.objectContaining({
                    code: 4,
                    name: "ABAPError",
                    key: "Division by 0 (type I or INT8)",
                    message: "Division by 0 (type I or INT8)",
                    abapMsgClass: "SR",
                    abapMsgType: "E",
                    abapMsgNumber: "000",
                    abapMsgV1: "Division by 0 (type I or INT8)"
                })
            );
            // Assures that the connection handle is correctly synchronized
            sync(client, done);
        }
    );
});

it("error: invoke() AbapApplicationError E51", function (done) {
    client.invoke(
        "RFC_RAISE_ERROR", {
            METHOD: "51",
            MESSAGETYPE: "E"
        },
        function (err) {
            expect(err).toBeDefined();
            expect(err).toEqual(
                expect.objectContaining({
                    code: 3,
                    name: "ABAPError",
                    key: "BLOCKED_COMMIT",
                    message: "A database commit was blocked by the application."
                })
            );
            // Assures that the connection handle is correctly synchronized
            sync(client, done);
        }
    );
});

it("error: invoke() AbapRuntimeError A", function (done) {
    client.invoke("RFC_RAISE_ERROR", {
        MESSAGETYPE: "A"
    }, function (err, res) {
        expect(err).toBeDefined();
        expect(err).toEqual(
            expect.objectContaining({
                code: 4,
                name: "ABAPError",
                key: "Function not supported",
                message: "Function not supported",
                abapMsgClass: "SR",
                abapMsgType: "A",
                abapMsgNumber: "006"
            })
        );
        // Assures that the connection handle is correctly synchronized
        sync(client, done);
    });
});

it("error: invoke() AbapRuntimeError X", function (done) {
    client.invoke("RFC_RAISE_ERROR", {
        MESSAGETYPE: "X"
    }, function (err) {
        expect(err).toBeDefined();
        expect(err).toEqual(
            expect.objectContaining({
                code: 4,
                name: "ABAPError",
                key: "MESSAGE_TYPE_X",
                message: "The current application has triggered a termination with a short dump.",
                abapMsgClass: "00",
                abapMsgType: "X",
                abapMsgNumber: "341",
                abapMsgV1: "MESSAGE_TYPE_X"
            })
        );
        // Assures that the connection handle is correctly synchronized
        sync(client, done);
    });
});

it("error: invoke() SAP GUI in background", function (done) {
    client.invoke("STFC_SAPGUI", {}, function (err) {
        expect(err).toBeDefined();
        expect(err).toEqual(
            expect.objectContaining({
                code: 3,
                name: "ABAPError",
                key: "DYNPRO_SEND_IN_BACKGROUND",
                message: "Screen output without connection to user."
            })
        );
        // Assures that the connection handle is correctly synchronized
        sync(client, done);
    });
});

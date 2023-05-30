// SPDX-FileCopyrightText: 2014 SAP SE Srdjan Boskovic <srdjan.boskovic@sap.com>
//
// SPDX-License-Identifier: Apache-2.0

import { direct_client, Client } from "../utils/setup";

describe("Errors: Promise", () => {
    const client = direct_client();

    beforeEach(function () {
        return client.open();
    });

    afterEach(function () {
        if (client.alive) {
            return client.close();
        } else {
            return new Promise((resolve) => resolve(0));
        }
    });

    test("error: call() promise rejects invalid credentials", function () {
        expect.assertions(1);
        const wrongClient = direct_client("MME_WRONG_USER");

        return (wrongClient.open() as Promise<Client>).catch((err) => {
            expect(err).toEqual(
                expect.objectContaining({
                    message: "Name or password is incorrect (repeat logon)",
                    code: 2,
                    key: "RFC_LOGON_FAILURE",
                })
            );
        });
    });

    test("error: call() promise rejects non-existing parameter", function () {
        expect.assertions(1);
        return client
            .call("STFC_CONNECTION", {
                XXX: "wrong param",
            })
            .catch((err) => {
                expect(err).toEqual(
                    expect.objectContaining({
                        code: 20,
                        key: "RFC_INVALID_PARAMETER",
                        message: "field 'XXX' not found",
                    })
                );
            });
    });

    test("error: promise call() RFC_RAISE_ERROR", function (done) {
        expect.assertions(1);
        client
            .call("RFC_RAISE_ERROR", {
                MESSAGETYPE: "A",
            })
            .catch((err) => {
                expect(err).toEqual(
                    expect.objectContaining({
                        code: 4,
                        key: "Function not supported",
                        abapMsgClass: "SR",
                        abapMsgType: "A",
                        abapMsgNumber: "006",
                        message: "Function not supported",
                    })
                );
                done();
            });
    });

    test("error: open() promise requires minimum of connection parameters", function () {
        expect.assertions(1);
        const wrongClient = direct_client("MME_NO_ASHOST");
        return (wrongClient.open() as Promise<Client>).catch((err) => {
            expect(err).toEqual(
                expect.objectContaining({
                    message:
                        "Parameter ASHOST, GWHOST, MSHOST or PORT is missing.",
                    code: 20,
                    key: "RFC_INVALID_PARAMETER",
                    name: "RfcLibError",
                })
            );
        });
    });
    /*
    // error can be tested with javascript only
    test.skip("error: promise call() requires at least two arguments", function () {
        expect.assertions(1);
        return client.call("rfc").catch((err) => {
            expect(err).toEqual(
                expect.objectContaining({
                    name: "TypeError",
                    message:
                        "Please provide remote function module name and parameters as arguments",
                })
            );
        });
    });

    // error can be tested with javascript only
    test.skip("error: promise call() rejects non-string rfm name", function () {
        expect.assertions(1);
        return client.call(23, {}, 2).catch((err) => {
            expect(err).toEqual(
                expect.objectContaining({
                    name: "TypeError",
                    message:
                        "First argument (remote function module name) must be an string",
                })
            );
        });
    });

    // error can be tested with javascript only
    test.skip("error: promise call() rejects non-object second argument (remote function module parameters)", function () {
        expect.assertions(1);
        return client.call("rfc", 41, 2).catch((err) => {
            expect(err).toEqual(
                expect.objectContaining({
                    name: "TypeError",
                    message:
                        "Second argument (remote function module parameters) must be an object",
                })
            );
        });
    });
    */
});

// SPDX-FileCopyrightText: 2014 SAP SE Srdjan Boskovic <srdjan.boskovic@sap.com>
//
// SPDX-License-Identifier: Apache-2.0

"use strict";

describe("Connection terminate timeout", () => {
    const setup = require("../../utils/setup");
    const binding = setup.binding;
    const WAIT = 3;
    const TIMEOUT = 1;
    const RfcCanceledError = {
        name: "RfcLibError",
        group: 4,
        code: 7,
        codeString: "RFC_CANCELED",
        key: "RFC_CANCELED",
        message: "Connection was canceled.",
    };

    test("Client options timeout", function (done) {
        const client = new binding.Client(
            { dest: "MME" },
            { timeout: TIMEOUT }
        );
        expect.assertions(3);
        client.open(() => {
            // call function
            const handle = client.connectionHandle;
            client.invoke(
                "RFC_PING_AND_WAIT",
                {
                    SECONDS: WAIT,
                },
                function (err) {
                    expect(client.alive).toEqual(true);
                    expect(err).toMatchObject(RfcCanceledError);
                    expect(client.connectionHandle).notjest.toEqual(handle);
                    client.close(() => done());
                }
            );
        });
    });
});

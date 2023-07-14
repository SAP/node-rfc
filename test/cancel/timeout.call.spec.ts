// SPDX-FileCopyrightText: 2014 SAP SE Srdjan Boskovic <srdjan.boskovic@sap.com>
//
// SPDX-License-Identifier: Apache-2.0

import { Client } from "../utils/setup";

describe("Connection terminate timeout", () => {
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

    test("Call options timeout", function (done) {
        const client = new Client({ dest: "MME" });
        expect.assertions(3);
        void client.open(() => {
            // call function
            const handle = client.connectionHandle;
            client.invoke(
                "RFC_PING_AND_WAIT",
                {
                    SECONDS: WAIT,
                },
                function (err: unknown) {
                    expect(client.alive).toEqual(true);
                    expect(err).toMatchObject(RfcCanceledError);
                    expect(client.connectionHandle).not.toEqual(handle);
                    void client.close(() => {
                        done();
                    });
                },
                { timeout: TIMEOUT }
            );
        });
    });
});

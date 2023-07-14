// SPDX-FileCopyrightText: 2014 SAP SE Srdjan Boskovic <srdjan.boskovic@sap.com>
//
// SPDX-License-Identifier: Apache-2.0

import { direct_client } from "../utils/setup";

describe("Connection terminate by client", () => {
    const DURATION = 3;
    const CANCEL = 1;
    const RfcCanceledError = {
        name: "RfcLibError",
        group: 4,
        code: 7,
        codeString: "RFC_CANCELED",
        key: "RFC_CANCELED",
        message: "Connection was canceled.",
    };

    test("Non-managed, client.cancel() callback", function (done) {
        const client = direct_client();
        expect.assertions(2);
        void client.open(() => {
            // call function
            client.invoke(
                "RFC_PING_AND_WAIT",
                {
                    SECONDS: DURATION,
                },
                function (err) {
                    expect(err).toMatchObject(RfcCanceledError);
                    done();
                }
            );
            // cancel
            setTimeout(() => {
                void client.cancel((err: unknown) => {
                    expect(err).toBeUndefined();
                });
            }, CANCEL * 1000);
        });
    });
});

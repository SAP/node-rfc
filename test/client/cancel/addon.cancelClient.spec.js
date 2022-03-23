// SPDX-FileCopyrightText: 2014 SAP SE Srdjan Boskovic <srdjan.boskovic@sap.com>
//
// SPDX-License-Identifier: Apache-2.0

"use strict";

describe.skip("Connection terminate by addon", () => {
    const setup = require("../../utils/setup");
    const binding = setup.binding;
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

    test("Non-managed, addon.cancelClient() callback", function (done) {
        const client = setup.direct_client();
        expect.assertions(3);
        client.open(() => {
            // call function
            const handle = client.connectionHandle;
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
                binding.cancelClient(client, (err, res) => {
                    expect(err).toBeUndefined();
                    expect(res).toMatchObject({
                        connectionHandle: handle,
                        result: "cancelled",
                    });
                });
            }, CANCEL * 1000);
        });
    });
});

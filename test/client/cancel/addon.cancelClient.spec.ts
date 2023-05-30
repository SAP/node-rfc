// SPDX-FileCopyrightText: 2014 SAP SE Srdjan Boskovic <srdjan.boskovic@sap.com>
//
// SPDX-License-Identifier: Apache-2.0

import { direct_client, cancelClient } from "../../utils/setup";

describe.skip("Connection terminate by addon", () => {
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
        const client = direct_client();
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
                cancelClient(client, (err, res) => {
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

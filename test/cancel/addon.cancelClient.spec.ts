// SPDX-FileCopyrightText: 2014 SAP SE Srdjan Boskovic <srdjan.boskovic@sap.com>
//
// SPDX-License-Identifier: Apache-2.0

import { direct_client, cancelClient } from "../utils/setup";

describe("Connection terminate by addon", () => {
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

    test(
        "Non-managed, addon.cancelClient() callback",
        function (done) {
            const client = direct_client();
            expect.assertions(1);
            void client.open(() => {
                // 3 seconds long call
                client.invoke(
                    "RFC_PING_AND_WAIT",
                    {
                        SECONDS: DURATION,
                    },
                    function (err: unknown) {
                        expect(err).toMatchObject(RfcCanceledError);
                        done();
                    }
                );
                // cancel after 1 sec
                setTimeout(() => {
                    <void>cancelClient(client);
                }, CANCEL * 1000);
            });
        },
        DURATION * 1000
    );
});

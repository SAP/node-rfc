// SPDX-FileCopyrightText: 2014 SAP SE Srdjan Boskovic <srdjan.boskovic@sap.com>
//
// SPDX-License-Identifier: Apache-2.0

"use strict";

describe("Connection terminate by addon", () => {
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

    test("Non-managed, addon.cancelClient() promise", function (done) {
        const client = setup.direct_client();
        expect.assertions(2);
        client.open().then((client) => {
            // call function
            const handle = client.connectionHandle;
            client
                .call("RFC_PING_AND_WAIT", {
                    SECONDS: DURATION,
                })
                .catch((err) => {
                    expect(err).toMatchObject(RfcCanceledError);
                    done();
                });
            // cancel
            setTimeout(() => {
                binding.cancelClient(client).then((res) => {
                    expect(res).toMatchObject({
                        connectionHandle: handle,
                        result: "cancelled",
                    });
                });
            }, CANCEL * 1000);
        });
    });
});

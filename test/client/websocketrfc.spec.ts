// SPDX-FileCopyrightText: 2014 SAP SE Srdjan Boskovic <srdjan.boskovic@sap.com>
//
// SPDX-License-Identifier: Apache-2.0

import { Client, loadCryptoLibrary, CryptoLibPath } from "../utils/setup";

describe.skip("Client: WebSocket RFC", () => {
    test("loadCryptoLib: success", () => {
        return expect(loadCryptoLibrary(CryptoLibPath)).toBeUndefined();
    });

    test("loadCryptoLib: error", () => {
        const invalidLibPath = "_" + CryptoLibPath;
        return expect(() => loadCryptoLibrary(invalidLibPath)).toThrow(
            new Error(`Crypto library not found: ${invalidLibPath}`)
        );
    });

    test("wsrfc call error: no client pse ", async () => {
        const client = new Client({ dest: "WS_ALX_NOCC" });
        try {
            await client.open();
        } catch (ex) {
            expect(ex).toMatchObject({
                code: 20,
                codeString: "RFC_INVALID_PARAMETER",
                group: 5,
                key: "RFC_INVALID_PARAMETER",
                message: "Unable to use TLS with client PSE missing",
                name: "RfcLibError",
            });
        }
    });

    test.skip("wsrfc call: basic auth", async () => {
        expect.assertions(1);
        try {
            const client = new Client({ dest: "WS_ALX" });

            await client.open();
            expect(client.alive).toBe(true);
            await client.close();
        } catch (ex) {
            expect(ex).toBeUndefined();
        }
    });

    test("wsrfc call: client certificate", async () => {
        expect.assertions(1);

        try {
            const client = new Client({ dest: "WS_ALX_CC" });

            await client.open();
            expect(client.alive).toBe(true);
            await client.close();
        } catch (ex) {
            expect(ex).toBeUndefined();
        }
    });
});

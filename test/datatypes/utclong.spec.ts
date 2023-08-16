// SPDX-FileCopyrightText: 2014 SAP SE Srdjan Boskovic <srdjan.boskovic@sap.com>
//
// SPDX-License-Identifier: Apache-2.0

import { direct_client } from "../utils/setup";
import { RFC_MATH } from "../utils/config";

describe("Datatypes: UTCLONG", () => {
    const UTCLONG = RFC_MATH.UTCLONG;
    const client = direct_client("QM7");

    beforeAll(() => {
        return client.open();
    });

    afterAll(() => {
        return client.close();
    });

    test("UTCLONG accepts min, max, initial", async () => {
        expect.assertions(3);
        let res = await client.call("ZDATATYPES", {
            IV_UTCLONG: UTCLONG.MIN,
        });
        expect(res.EV_UTCLONG).toEqual(UTCLONG.MIN);

        res = await client.call("ZDATATYPES", {
            IV_UTCLONG: UTCLONG.MAX,
        });
        expect(res.EV_UTCLONG).toEqual(UTCLONG.MAX);

        res = await client.call("ZDATATYPES", {
            IV_UTCLONG: UTCLONG.INITIAL,
        });
        expect(res.EV_UTCLONG).toEqual(UTCLONG.INITIAL);
    });

    test("UTCLONG rejects non string", () => {
        expect.assertions(1);
        return client
            .call("ZDATATYPES", {
                IV_UTCLONG: 1,
            })
            .catch((ex) => {
                expect(ex).toEqual(
                    expect.objectContaining({
                        message:
                            "UTCLONG string expected from NodeJS for ABAP field of type 32",
                        name: "nodeRfcError",
                        rfmPath: {
                            field: "IV_UTCLONG",
                            parameter: "IV_UTCLONG",
                            rfm: "ZDATATYPES",
                        },
                    })
                );
            });
    });

    test("UTCLONG rejects invalid format", () => {
        expect.assertions(1);
        return client
            .call("ZDATATYPES", {
                IV_UTCLONG: "1",
            })
            .catch((ex) => {
                expect(ex).toEqual(
                    expect.objectContaining({
                        name: "RfcLibError",
                        code: 22,
                        key: "RFC_CONVERSION_FAILURE",
                        message:
                            "Cannot convert 1 to RFCTYPE_UTCLONG : illegal format",
                    })
                );
            });
    });
});

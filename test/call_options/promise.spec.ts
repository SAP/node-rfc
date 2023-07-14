// SPDX-FileCopyrightText: 2014 SAP SE Srdjan Boskovic <srdjan.boskovic@sap.com>
//
// SPDX-License-Identifier: Apache-2.0

import { direct_client, RfcTable } from "../utils/setup";

describe("RFC Call options - promise", () => {
    const client = direct_client();

    beforeEach(function (done) {
        void client.open(function (err:unknown) {
            done(err);
        });
    });

    afterEach(function (done) {
        void client.close(function (err:unknown) {
            done(err);
        });
    });

    const TIMEOUT = 10000;

    test(
        "options: pass when some parameters skipped",
        async () => {
            const notRequested = [
                "ET_COMPONENTS",
                "ET_HDR_HIERARCHY",
                "ET_MPACKAGES",
                "ET_OPERATIONS",
                "ET_OPR_HIERARCHY",
                "ET_PRTS",
                "ET_RELATIONS",
            ];
            const res = await client.call(
                "EAM_TASKLIST_GET_DETAIL",
                {
                    IV_PLNTY: "A",
                    IV_PLNNR: "00100000",
                },
                {
                    notRequested: notRequested,
                }
            );

            expect(res).toBeDefined();
            expect(res).toHaveProperty("ET_RETURN");
            expect((res.ET_RETURN as RfcTable).length).toBe(0);
        },
        TIMEOUT
    );

    test("options: error when all requested", function () {
        return client
            .call("EAM_TASKLIST_GET_DETAIL", {
                IV_PLNTY: "A",
                IV_PLNNR: "00100000",
            })
            .then((res) => {
                // ET_RETURN error if all params requested
                expect(res).toBeDefined();
                expect(res).toHaveProperty("ET_RETURN");
                expect((res.ET_RETURN as RfcTable).length).toBe(1);
                expect(res.ET_RETURN[0]).toEqual(
                    expect.objectContaining({
                        TYPE: "E",
                        ID: "DIWP1",
                        NUMBER: "212",
                        MESSAGE: "Task list A 00100000  is not hierarchical",
                        LOG_NO: "",
                        LOG_MSG_NO: "000000",
                        MESSAGE_V1: "A",
                        MESSAGE_V2: "00100000",
                        MESSAGE_V3: "",
                        MESSAGE_V4: "",
                        PARAMETER: "HIERARCHY",
                        ROW: 0,
                        FIELD: "",
                    })
                );
            });
    });
});

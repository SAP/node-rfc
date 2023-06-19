// SPDX-FileCopyrightText: 2014 SAP SE Srdjan Boskovic <srdjan.boskovic@sap.com>
//
// SPDX-License-Identifier: Apache-2.0

import { RfcObject, RfcTable, direct_client } from "../utils/setup";

describe("Performance: ABAP", () => {
    const client = direct_client();

    beforeEach(function (done) {
        void client.open((err: unknown) => {
            done(err);
        });
    });

    afterEach(function (done) {
        void client.close(() => {
            done();
        });
    });

    const TIMEOUT = 20000;
    test(
        "performance: invoke() BAPI_USER_GET_DETAIL",
        function (done) {
            client.invoke(
                "BAPI_USER_GET_DETAIL",
                {
                    USERNAME: "DEMO",
                },
                function (err: unknown, res: RfcObject) {
                    if (err) return done(err) as unknown;
                    expect(res).toBeDefined();
                    expect(Object.keys(res).sort()).toEqual(
                        expect.arrayContaining([
                            "ADDRESS",
                            "ACTIVITYGROUPS",
                            "DEFAULTS",
                            "GROUPS",
                            "ISLOCKED",
                            "LOGONDATA",
                            "PARAMETER",
                            "PROFILES",
                            "RETURN",
                        ])
                    );
                    void client.close(() => {
                        done();
                    });
                }
            );
        },
        TIMEOUT
    );

    test(
        "performance: invoke() STFC_PERFORMANCE",
        function (done) {
            const COUNT = 10000;
            client.invoke(
                "STFC_PERFORMANCE",
                {
                    CHECKTAB: "X",
                    LGET0332: COUNT.toString(),
                    LGET1000: COUNT.toString(),
                },

                function (err: unknown, res: RfcObject) {
                    if (err) return done(err) as unknown;
                    expect((res.ETAB0332 as RfcTable).length).toBe(COUNT);
                    expect((res.ETAB1000 as RfcTable).length).toBe(COUNT);
                    void client.close(() => {
                        done();
                    });
                }
            );
        },
        TIMEOUT
    );

    /*
    test('performance: invoke() SWNC_READ_SNAPSHOT', function (done) {
        function toABAPdate(date) {
            let mm = date.getMonth() + 1;
            let dd = date.getDate();
            return [date.getFullYear(), mm > 9 ? mm : '0' + mm, dd > 9 ? dd : '0' + dd].join('');
        }
        let endDate = new Date();
        let startDate = new Date(endDate);
        startDate.setDate(startDate.getDate() - 1);
        client.invoke(
            'SWNC_READ_SNAPSHOT', {
                READ_TIMEZONE: 'UTC',
                READ_START_DATE: toABAPdate(startDate),
                READ_START_TIME: '000000',
                READ_END_DATE: toABAPdate(endDate),
                READ_END_TIME: '235959',
                TIME_RESOLUTION: 60,
            },

            function (err, res) {
                if (err) return done(err);
                expect(res.HITLIST_DATABASE.length).toBe(0);
                expect(res.HITLIST_RESPTIME.length).toBe(0);
                client.close(() => done());
            }
        );
    }, TIMEOUT);
    */
});

// SPDX-FileCopyrightText: 2014 SAP SE Srdjan Boskovic <srdjan.boskovic@sap.com>
//
// SPDX-License-Identifier: Apache-2.0

import {
    addon,
    dependencies,
    direct_client,
    refEnvironment,
    Promise,
    UNICODETEST,
    UNICODETEST2,
    Client,
    RfcStructure,
    RfcTable,
    RfcObject,
} from "../utils/setup";

describe("Client: direct callback", () => {
    let client: Client;

    beforeAll((done) => {
        client = direct_client();
        void client.open((err: unknown) => {
            return done(err) as unknown;
        });
        done();
    });

    afterAll((done) => {
        done();
    });

    test("environment of addon and Client", function () {
        expect.assertions(3);
        expect(addon.environment).toMatchObject(refEnvironment);
        expect(addon.Client.environment).toMatchObject(refEnvironment);
        expect(client.environment).toMatchObject(refEnvironment);
    });

    test("Typescript Promises replaced by Bluebird", () => {
        expect.assertions(1);
        const promise = Promise;
        expect(dependencies.bluebird.indexOf(promise.version)).toBeGreaterThan(
            -1
        );
    });

    test("Getters", function () {
        expect.assertions(6);

        expect(client.id).toBeGreaterThan(0);
        expect(client.config).toHaveProperty("connectionParameters");
        expect(client.alive).toBe(false);
        expect(client.connectionHandle).toBe(0);
        expect(client.pool_id).toBe(0);
        expect(client._id.indexOf("[d]")).toBeGreaterThan(-1);
    });

    test("alive and ping() should return false when disconnected", function (done) {
        expect.assertions(3);
        const clnt = direct_client();
        expect(clnt.alive).toBe(false);
        void clnt.ping((err: unknown, res: boolean) => {
            expect(res).toBe(false);
            expect(err).toMatchObject({
                name: "nodeRfcError",
                message: "RFM client request over closed connection: ping()",
            });
            done();
        });
    });

    test("alive and ping() should return true when connected", function (done) {
        expect.assertions(3);
        void client.connect((err: unknown) => {
            if (err) return done(err) as unknown;
            expect(client.alive).toBe(true);
            void client.ping((err: unknown, res: RfcObject) => {
                expect(err).toBeUndefined();
                expect(res).toBe(true);
                void client.close(() => {
                    done();
                });
            });
        });
    });

    test("connectionInfo should return connection information when connected", function (done) {
        expect.assertions(1);
        void client.connect((err) => {
            if (err) return done(err) as unknown;
            const connectionInfo = client.connectionInfo;
            expect(Object.keys(connectionInfo).sort()).toEqual(
                [
                    "dest",
                    "host",
                    "partnerHost",
                    "sysNumber",
                    "sysId",
                    "client",
                    "user",
                    "language",
                    "trace",
                    "isoLanguage",
                    "codepage",
                    "partnerCodepage",
                    "rfcRole",
                    "type",
                    "partnerType",
                    "rel",
                    "partnerRel",
                    "kernelRel",
                    "cpicConvId",
                    "progName",
                    "partnerBytesPerChar",
                    "partnerSystemCodepage",
                    "partnerIP",
                    "partnerIPv6",
                    //'reserved'
                ].sort()
            );

            void client.close(() => {
                done();
            });
        });
    });

    test("connectionInfo() should return Error when disconnected", function (done) {
        expect.assertions(2);
        expect(client.alive).toBeFalsy();
        expect(client.connectionInfo).toMatchObject({
            name: "nodeRfcError",
            message:
                "RFM client request over closed connection: connectionInfo",
        });
        done();
    });

    test("invoke() STFC_CONNECTION should return unicode string (1)", function (done) {
        void client.connect((err) => {
            if (err) return done(err) as unknown;
            client.invoke(
                "STFC_CONNECTION",
                {
                    REQUTEXT: UNICODETEST,
                },
                function (err: unknown, res: RfcObject) {
                    if (err) return done(err) as unknown;
                    expect(res).toHaveProperty("ECHOTEXT");
                    const echotext = (res as RfcStructure).ECHOTEXT as string;
                    expect(echotext.indexOf(UNICODETEST)).toBe(0);
                    void client.close(() => {
                        done();
                    });
                }
            );
        });
    });

    test("invoke() STFC_CONNECTION should return unicode string (2)", function (done) {
        void client.connect((err: unknown) => {
            if (err) return done(err) as unknown;
            client.invoke(
                "STFC_CONNECTION",
                {
                    REQUTEXT: UNICODETEST2,
                },
                function (err: unknown, res: RfcObject) {
                    if (err) return done(err) as unknown;
                    expect(res).toHaveProperty("ECHOTEXT");
                    const echotext = (res as RfcStructure).ECHOTEXT as string;
                    expect(echotext.indexOf(UNICODETEST2)).toBe(0);
                    void client.close(() => {
                        done();
                    });
                }
            );
        });
    });

    test("invoke() STFC_STRUCTURE should return structure and table", function (done) {
        const importStruct = {
            RFCFLOAT: 1.23456789,
            RFCCHAR1: "A",
            RFCCHAR2: "",
            RFCCHAR4: "DFG",

            RFCINT1: 1,
            RFCINT2: 2,
            RFCINT4: 345,

            RFCHEX3: Buffer.from("\x01\x02\x03", "ascii"),

            RFCTIME: "121120",
            RFCDATE: "20140101",

            RFCDATA1: "1DATA1",
            RFCDATA2: "DATA222",
        };
        const INPUTROWS = 10;
        const importTable = [] as Array<typeof importStruct>;
        for (let i = 0; i < INPUTROWS; i++) {
            const row = {} as typeof importStruct;
            Object.assign(row, importStruct);
            row.RFCINT1 = i;
            importTable.push(row);
        }
        void client.connect(() => {
            client.invoke(
                "STFC_STRUCTURE",
                {
                    IMPORTSTRUCT: importStruct,
                    RFCTABLE: importTable,
                },
                function (err: unknown, res: RfcObject) {
                    if (err) return done(err) as unknown;
                    expect(Object.keys(res)).toEqual([
                        "ECHOSTRUCT",
                        "RESPTEXT",
                        "IMPORTSTRUCT",
                        "RFCTABLE",
                    ]);

                    // ECHOSTRUCT match IMPORTSTRUCT
                    for (const k in importStruct) {
                        if (k === "RFCHEX3") {
                            //console.log(importStruct[k].length, res.ECHOSTRUCT[k].length);
                            //for (let u = 0; u < importStruct[k].length; u++) {
                            //    console.log(importStruct[k][u], res.ECHOSTRUCT[k][u])
                            //}
                            expect(
                                Buffer.compare(
                                    importStruct[k],
                                    (res["ECHOSTRUCT"] as RfcStructure)[
                                        k
                                    ] as Buffer
                                )
                            ).toEqual(0);
                        } else {
                            expect(
                                (res["ECHOSTRUCT"] as RfcStructure)[k]
                            ).toEqual(importStruct[k]);
                        }
                    }

                    // added row is incremented IMPORTSTRUCT
                    expect((res["RFCTABLE"] as RfcTable).length).toEqual(
                        INPUTROWS + 1
                    );

                    // output table match import table
                    for (let i = 0; i < INPUTROWS; i++) {
                        const rowIn = importTable[i];
                        const rowOut = (res["RFCTABLE"] as RfcTable)[
                            i
                        ] as RfcStructure;
                        for (const k in rowIn) {
                            if (k === "RFCHEX3") {
                                expect(
                                    Buffer.compare(
                                        rowIn[k],
                                        rowOut[k] as Buffer
                                    )
                                ).toEqual(0);
                            } else {
                                expect(rowIn[k]).toEqual(rowOut[k]);
                            }
                        }
                    }

                    // added row match incremented IMPORTSTRUCT
                    expect((res["RFCTABLE"] as RfcTable)[INPUTROWS]).toEqual(
                        expect.objectContaining({
                            RFCFLOAT: importStruct.RFCFLOAT + 1,
                            RFCINT1: importStruct.RFCINT1 + 1,
                            RFCINT2: importStruct.RFCINT2 + 1,
                            RFCINT4: importStruct.RFCINT4 + 1,
                        })
                    );

                    void client.close(() => {
                        done();
                    });
                }
            );
        });
    });
});

// SPDX-FileCopyrightText: 2014 SAP SE Srdjan Boskovic <srdjan.boskovic@sap.com>
//
// SPDX-License-Identifier: Apache-2.0

"use strict";

describe("Client: direct callback", () => {
    const setup = require("../utils/setup");
    const binding = setup.binding;
    let client;

    beforeAll((done) => {
        client = setup.direct_client();
        done();
    });

    afterAll((done) => {
        done();
    });

    test("environment @ binding and Client", function () {
        expect.assertions(3);
        expect(binding.environment).toMatchObject(setup.RefEnvironment);
        expect(binding.Client.environment).toMatchObject(setup.RefEnvironment);
        expect(client.environment).toMatchObject(setup.RefEnvironment);
        expect;
    });

    test("Typescript Promises replaced by Bluebird", () => {
        expect.assertions(1);
        const promise = setup.Promise;
        expect(
            require("../../package.json").dependencies.bluebird.indexOf(
                promise.version
            )
        ).toBeGreaterThan(-1);
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
        expect(client.alive).toBe(false);
        client.ping((err, res) => {
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
        client.connect((err) => {
            if (err) return done(err);
            expect(client.alive).toBe(true);
            client.ping((err, res) => {
                expect(err).toBeUndefined();
                expect(res).toBe(true);
                client.close(() => done());
            });
        });
    });

    test("connectionInfo should return connection information when connected", function (done) {
        expect.assertions(1);
        client.connect((err) => {
            if (err) return done(err);
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

            client.close(() => done());
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
        client.connect((err) => {
            if (err) return done(err);
            client.invoke(
                "STFC_CONNECTION",
                {
                    REQUTEXT: setup.UNICODETEST,
                },
                function (err, res) {
                    if (err) return done(err);
                    expect(res).toHaveProperty("ECHOTEXT");
                    expect(res.ECHOTEXT.indexOf(setup.UNICODETEST)).toBe(0);
                    client.close(() => done());
                }
            );
        });
    });

    test("invoke() STFC_CONNECTION should return unicode string (2)", function (done) {
        client.connect((err) => {
            if (err) return done(err);
            client.invoke(
                "STFC_CONNECTION",
                {
                    REQUTEXT: setup.UNICODETEST2,
                },
                function (err, res) {
                    if (err) return done(err);
                    expect(res).toHaveProperty("ECHOTEXT");
                    expect(res.ECHOTEXT.indexOf(setup.UNICODETEST2)).toBe(0);
                    client.close(() => done());
                }
            );
        });
    });

    test.only("invoke() STFC_STRUCTURE should return structure and table", function (done) {
        let importStruct = {
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
        let INPUTROWS = 10;
        let importTable = [];
        for (let i = 0; i < INPUTROWS; i++) {
            let row = {};
            Object.assign(row, importStruct);
            row.RFCINT1 = i;
            importTable.push(row);
        }
        client.connect(() => {
            client.invoke(
                "STFC_STRUCTURE",
                {
                    IMPORTSTRUCT: importStruct,
                    RFCTABLE: importTable,
                },
                function (err, res) {
                    if (err) return done(err);
                    expect(Object.keys(res)).toEqual([
                        "ECHOSTRUCT",
                        "RESPTEXT",
                        "IMPORTSTRUCT",
                        "RFCTABLE",
                    ]);

                    // ECHOSTRUCT match IMPORTSTRUCT
                    for (let k in importStruct) {
                        if (k === "RFCHEX3") {
                            //console.log(importStruct[k].length, res.ECHOSTRUCT[k].length);
                            //for (let u = 0; u < importStruct[k].length; u++) {
                            //    console.log(importStruct[k][u], res.ECHOSTRUCT[k][u])
                            //}
                            expect(
                                Buffer.compare(
                                    importStruct[k],
                                    res.ECHOSTRUCT[k]
                                )
                            ).toEqual(0);
                        } else {
                            expect(res.ECHOSTRUCT[k]).toEqual(importStruct[k]);
                        }
                    }

                    // added row is incremented IMPORTSTRUCT
                    expect(res.RFCTABLE.length).toEqual(INPUTROWS + 1);

                    // output table match import table
                    for (let i = 0; i < INPUTROWS; i++) {
                        let rowIn = importTable[i];
                        let rowOut = res.RFCTABLE[i];
                        for (let k in rowIn) {
                            if (k === "RFCHEX3") {
                                expect(
                                    Buffer.compare(rowIn[k], rowOut[k])
                                ).toEqual(0);
                            } else {
                                expect(rowIn[k]).toEqual(rowOut[k]);
                            }
                        }
                    }

                    // added row match incremented IMPORTSTRUCT
                    expect(res.RFCTABLE[INPUTROWS]).toEqual(
                        expect.objectContaining({
                            RFCFLOAT: importStruct.RFCFLOAT + 1,
                            RFCINT1: importStruct.RFCINT1 + 1,
                            RFCINT2: importStruct.RFCINT2 + 1,
                            RFCINT4: importStruct.RFCINT4 + 1,
                        })
                    );

                    client.close(() => done());
                }
            );
        });
    });
});

// Copyright 2014 SAP AG.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http: //www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific
// language governing permissions and limitations under the License.

"use strict";

const setup = require("../setup");
const client = setup.client();
const packageJSON = require("../../package.json");

describe("Connection: Callbacks", () => {
    test("Client and package versions", function (done) {
        expect(packageJSON.version).toBe(
            client.version.binding
        );
        done();
    });

    test("Typescript Promises replaced by Bluebird", () => {
        const promise = setup.Promise;
        expect(promise).toEqual(expect.anything());
        expect(packageJSON.dependencies.bluebird.indexOf(promise.version)).toBeGreaterThan(-1);
    });

    test("Client getters", function () {
        expect.assertions(9);

        expect(client.id).toBeGreaterThan(0);

        expect(client.version).toHaveProperty("major");
        expect(client.version).toHaveProperty("minor");
        expect(client.version).toHaveProperty("patchLevel");
        expect(client.version).toHaveProperty(
            "binding",
            packageJSON.version
        );

        expect(client.options).toHaveProperty("bcd");
        expect(client.options).toHaveProperty("date");
        expect(client.options).toHaveProperty("time");

        return expect(
            () =>
            (client.version = {
                a: 1,
                b: 2,
            })
        ).toThrow(
            new TypeError(
                "Cannot set property version of #<Client> which has only a getter"
            )
        );
    });

    test("isAlive ands ping() should be false when disconnected", function (done) {
        expect(client.isAlive).toBeFalsy();
        client.ping((err, res) => {
            expect(res).toBeFalsy();
            done();
        });
    });

    test("isAlive and ping() should be true when connected", function (done) {
        client.connect(err => {
            if (err) return done(err);
            expect(client.isAlive).toBeTruthy();
            client.ping((err, res) => {
                expect(res).toBeTruthy();
                client.close(() => done());
            });
        });
    });

    test("connectionInfo should return connection information when connected", function (done) {
        client.connect(err => {
            if (err) return done(err);
            let connectionInfo = client.connectionInfo;
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

            expect(connectionInfo).toEqual(
                expect.objectContaining({
                    user: setup.abapSystem.user.toUpperCase(),
                    sysNumber: setup.abapSystem.sysnr,
                    client: setup.abapSystem.client,
                })
            );

            client.close(() => done());
        })
    });

    test("connectionInfo() should return {} when disconnected", function (done) {
        expect(client.isAlive).toBeFalsy();
        expect(client.connectionInfo).toEqual({});
        done();
    });

    test("reopen() should reopen the connection", function (done) {
        expect(client.isAlive).toBeFalsy();
        client.connect(err => {
            if (err) return done(err);
            expect(client.isAlive).toBeTruthy();
            client.reopen(function (err) {
                if (err) return done(err);
                expect(client.isAlive).toBeTruthy();
                let convId = client.connectionInfo.cpicConvId;
                client.reopen(function (err) {
                    if (err) return done(err);
                    expect(client.isAlive).toBeTruthy();
                    expect(
                        convId === client.connectionInfo.cpicConvId
                    ).toBeFalsy();
                    done();
                });
            });
        });
    });

    test("invoke() STFC_CONNECTION should return unicode string", function (done) {
        client.connect(err => {
            if (err) return done(err);
            client.invoke(
                "STFC_CONNECTION", {
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

    test("invoke() STFC_STRUCTURE should return structure and table", function (done) {
        let importStruct = {
            RFCFLOAT: 1.23456789,
            RFCCHAR1: "A",
            RFCCHAR2: "BC",
            RFCCHAR4: "DEFG",

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
                "STFC_STRUCTURE", {
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
                                Buffer.compare(importStruct[k], res.ECHOSTRUCT[k])
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
                                expect(Buffer.compare(rowIn[k], rowOut[k])).toEqual(
                                    0
                                );
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
                })
        });
    });
});

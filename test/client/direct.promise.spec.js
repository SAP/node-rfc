// SPDX-FileCopyrightText: 2014 SAP SE Srdjan Boskovic <srdjan.boskovic@sap.com>
//
// SPDX-License-Identifier: Apache-2.0

"use strict";

describe("Client: direct promise", () => {
    const setup = require("../utils/setup");
    let client;

    beforeAll((done) => {
        client = setup.direct_client();
        done();
    });

    test("call() STFC_CONNECTION should return unicode string", function (done) {
        client.open().then((clnt) => {
            clnt.call("STFC_CONNECTION", {
                REQUTEXT: setup.UNICODETEST,
            }).then((res) => {
                expect(res).toBeDefined();
                expect(res).toHaveProperty("ECHOTEXT");
                expect(res.ECHOTEXT.indexOf(setup.UNICODETEST)).toEqual(0);
                clnt.close(() => done());
            });
        });
    });

    test("call() STFC_STRUCTURE should return structure and table", function (done) {
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
        let importTable = [importStruct];

        client.open().then((clnt) => {
            clnt.call("STFC_STRUCTURE", {
                IMPORTSTRUCT: importStruct,
                RFCTABLE: importTable,
            }).then((res) => {
                expect(res).toBeDefined();
                expect(res).toHaveProperty("ECHOSTRUCT");
                expect(res).toHaveProperty("RFCTABLE");

                expect(res.ECHOSTRUCT.RFCCHAR1).toEqual(importStruct.RFCCHAR1);
                expect(res.ECHOSTRUCT.RFCCHAR2).toEqual(importStruct.RFCCHAR2);
                expect(res.ECHOSTRUCT.RFCCHAR4).toEqual(importStruct.RFCCHAR4);
                expect(res.ECHOSTRUCT.RFCFLOAT).toEqual(importStruct.RFCFLOAT);
                expect(res.ECHOSTRUCT.RFCINT1).toEqual(importStruct.RFCINT1);
                expect(res.ECHOSTRUCT.RFCINT2).toEqual(importStruct.RFCINT2);
                expect(res.ECHOSTRUCT.RFCINT4).toEqual(importStruct.RFCINT4);
                expect(res.ECHOSTRUCT.RFCDATA1).toContain(
                    importStruct.RFCDATA1
                );
                expect(res.ECHOSTRUCT.RFCDATA2).toContain(
                    importStruct.RFCDATA2
                );
                expect(res.ECHOSTRUCT.RFCHEX3.toString()).toEqual(
                    importStruct.RFCHEX3.toString()
                );

                expect(res.RFCTABLE.length).toBe(2);
                expect(res.RFCTABLE[1].RFCFLOAT).toEqual(
                    importStruct.RFCFLOAT + 1
                );
                expect(res.RFCTABLE[1].RFCINT1).toEqual(
                    importStruct.RFCINT1 + 1
                );
                expect(res.RFCTABLE[1].RFCINT2).toEqual(
                    importStruct.RFCINT2 + 1
                );
                expect(res.RFCTABLE[1].RFCINT4).toEqual(
                    importStruct.RFCINT4 + 1
                );

                clnt.close(() => done());
            });
        });
    });

    test("alive and ping() should be true when connected", function (done) {
        expect.assertions(2);
        client.open().then((clnt) => {
            expect(clnt.alive).toBe(true);
            clnt.ping().then((res) => {
                expect(res).toBe(true);
                clnt.close().then(() => {
                    done();
                });
            });
        });
    });

    test("alive and ping() should be false when disconnected", function () {
        expect.assertions(2);
        expect(client.alive).toBe(false);
        return client.ping().catch((ex) => {
            expect(ex).toMatchObject({
                name: "nodeRfcError",
                message: "RFM client request over closed connection: ping()",
            });
        });
    });
});

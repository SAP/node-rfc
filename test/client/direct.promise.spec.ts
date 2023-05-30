// SPDX-FileCopyrightText: 2014 SAP SE Srdjan Boskovic <srdjan.boskovic@sap.com>
//
// SPDX-License-Identifier: Apache-2.0

import { direct_client, UNICODETEST2, Client } from "../utils/setup";

describe("Client: direct promise", () => {
    let client;

    beforeAll((done) => {
        client = direct_client();
        done();
    });

    test("call() STFC_CONNECTION should return unicode string", function (done) {
        client.open().then((clnt: Client) => {
            clnt.call("STFC_CONNECTION", {
                REQUTEXT: UNICODETEST2,
            }).then((res) => {
                expect(res).toBeDefined();
                expect(res).toHaveProperty("ECHOTEXT");
                expect((res.ECHOTEXT as string).indexOf(UNICODETEST2)).toEqual(
                    0
                );
                clnt.close(() => done());
            });
        });
    });

    test("call() STFC_STRUCTURE should return structure and table", function (done) {
        const importStruct = {
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
        const importTable = [importStruct];

        client.open().then((clnt: Client) => {
            clnt.call("STFC_STRUCTURE", {
                IMPORTSTRUCT: importStruct,
                RFCTABLE: importTable,
            }).then((res) => {
                expect(res).toBeDefined();
                expect(res).toHaveProperty("ECHOSTRUCT");
                expect(res).toHaveProperty("RFCTABLE");
                const ECHOSTRUCT = res.ECHOSTRUCT as typeof importStruct;

                expect(ECHOSTRUCT.RFCCHAR1).toEqual(importStruct.RFCCHAR1);
                expect(ECHOSTRUCT.RFCCHAR2).toEqual(importStruct.RFCCHAR2);
                expect(ECHOSTRUCT.RFCCHAR4).toEqual(importStruct.RFCCHAR4);
                expect(ECHOSTRUCT.RFCFLOAT).toEqual(importStruct.RFCFLOAT);
                expect(ECHOSTRUCT.RFCINT1).toEqual(importStruct.RFCINT1);
                expect(ECHOSTRUCT.RFCINT2).toEqual(importStruct.RFCINT2);
                expect(ECHOSTRUCT.RFCINT4).toEqual(importStruct.RFCINT4);
                expect(ECHOSTRUCT.RFCDATA1).toContain(importStruct.RFCDATA1);
                expect(ECHOSTRUCT.RFCDATA2).toContain(importStruct.RFCDATA2);
                expect(ECHOSTRUCT.RFCHEX3.toString()).toEqual(
                    importStruct.RFCHEX3.toString()
                );

                const RFCTABLE = res.RFCTABLE as typeof importTable;

                expect(RFCTABLE.length).toBe(2);
                expect(RFCTABLE[1].RFCFLOAT).toEqual(importStruct.RFCFLOAT + 1);
                expect(RFCTABLE[1].RFCINT1).toEqual(importStruct.RFCINT1 + 1);
                expect(RFCTABLE[1].RFCINT2).toEqual(importStruct.RFCINT2 + 1);
                expect(RFCTABLE[1].RFCINT4).toEqual(importStruct.RFCINT4 + 1);

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

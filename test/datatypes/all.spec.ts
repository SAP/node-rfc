// SPDX-FileCopyrightText: 2014 SAP SE Srdjan Boskovic <srdjan.boskovic@sap.com>
//
// SPDX-License-Identifier: Apache-2.0

import { randomBytes } from "crypto";
import Decimal from "decimal.js";
import {
    direct_client,
    RfcVariable,
    RfcStructure,
    RfcTable,
    RfcObject,
} from "../utils/setup";
import { RFC_MATH } from "../utils/config";
import { toABAPdate, fromABAPdate, XBYTES_TEST } from "../utils/utils";
import { RfcClientOptions } from "../../lib";

describe("Datatypes: all", () => {
    const client = direct_client();

    beforeAll(function (done) {
        void client.open((err) => {
            return done(err) as unknown;
        });
    });

    afterAll(function (done) {
        void client.close((err: unknown) => {
            return done(err) as unknown;
        });
    });

    test("Min/Max positive", function (done) {
        const isInput = {
            // Float
            ZFLTP_MIN: RFC_MATH.FLOAT.POS.MIN,
            ZFLTP_MAX: RFC_MATH.FLOAT.POS.MAX,

            // Decimal
            ZDECF16_MIN: RFC_MATH.DECF16.POS.MIN,
            ZDECF16_MAX: RFC_MATH.DECF16.POS.MAX,

            ZDECF34_MIN: RFC_MATH.DECF34.POS.MIN,
            ZDECF34_MAX: RFC_MATH.DECF34.POS.MAX,
        };
        client.invoke(
            "/COE/RBP_FE_DATATYPES",
            {
                IS_INPUT: isInput,
            },
            function (err: unknown, res: RfcObject) {
                expect(res).toHaveProperty("ES_OUTPUT");
                const output = res.ES_OUTPUT as RfcStructure;

                expect(typeof output.ZFLTP_MIN).toEqual("number");
                expect(output.ZFLTP_MIN).toEqual(parseFloat(isInput.ZFLTP_MIN));

                expect(typeof output.ZFLTP_MAX).toEqual("number");
                expect(output.ZFLTP_MAX).toEqual(parseFloat(isInput.ZFLTP_MAX));

                expect(typeof output.ZDECF16_MIN).toEqual("string");
                expect(output.ZDECF16_MIN).toEqual(isInput.ZDECF16_MIN);

                expect(typeof output.ZDECF16_MAX).toEqual("string");
                expect(output.ZDECF16_MAX).toEqual(isInput.ZDECF16_MAX);

                expect(typeof output.ZDECF34_MIN).toEqual("string");
                expect(output.ZDECF34_MIN).toEqual(isInput.ZDECF34_MIN);

                expect(typeof output.ZDECF34_MAX).toEqual("string");
                expect(output.ZDECF34_MAX).toEqual(isInput.ZDECF34_MAX);
                done(err);
            }
        );
    });

    test("Min/Max negative", function (done) {
        const isInput = {
            // Float
            ZFLTP_MIN: RFC_MATH.FLOAT.NEG.MIN,
            ZFLTP_MAX: RFC_MATH.FLOAT.NEG.MAX,

            // Decimal
            ZDECF16_MIN: RFC_MATH.DECF16.NEG.MIN,
            ZDECF16_MAX: RFC_MATH.DECF16.NEG.MAX,

            ZDECF34_MIN: RFC_MATH.DECF34.NEG.MIN,
            ZDECF34_MAX: RFC_MATH.DECF34.NEG.MAX,
        };

        client.invoke(
            "/COE/RBP_FE_DATATYPES",
            {
                IS_INPUT: isInput,
            },
            function (err: unknown, res: RfcObject) {
                expect(res).toHaveProperty("ES_OUTPUT");
                const output = res.ES_OUTPUT as RfcStructure;

                expect(typeof output.ZFLTP_MIN).toEqual("number");
                expect(output.ZFLTP_MIN).toEqual(parseFloat(isInput.ZFLTP_MIN));

                expect(typeof output.ZFLTP_MAX).toEqual("number");
                expect(output.ZFLTP_MAX).toEqual(parseFloat(isInput.ZFLTP_MAX));

                expect(typeof output.ZDECF16_MIN).toEqual("string");
                expect(output.ZDECF16_MIN).toEqual(isInput.ZDECF16_MIN);

                expect(typeof output.ZDECF16_MAX).toEqual("string");
                expect(output.ZDECF16_MAX).toEqual(isInput.ZDECF16_MAX);

                expect(typeof output.ZDECF34_MIN).toEqual("string");
                expect(output.ZDECF34_MIN).toEqual(isInput.ZDECF34_MIN);

                expect(typeof output.ZDECF34_MAX).toEqual("string");
                expect(output.ZDECF34_MAX).toEqual(isInput.ZDECF34_MAX);
                done(err);
            }
        );
    });

    test("error: ARRAY rejects non array", function (done) {
        client.invoke(
            "RFC_READ_TABLE",
            {
                QUERY_TABLE: "MARA",
                OPTIONS: "A string instead of an array",
            },
            function (err: unknown) {
                expect(err).toEqual(
                    expect.objectContaining({
                        message:
                            "Array expected from NodeJS, for ABAP RFM table of type 5",
                        name: "nodeRfcError",
                        rfmPath: {
                            parameter: "OPTIONS",
                            rfm: "RFC_READ_TABLE",
                            table: "OPTIONS",
                            table_line: -1,
                        },
                    })
                );
                done();
            }
        );
    });

    test("BCD and FLOAT accept numbers", function (done) {
        const isInput = {
            // Float
            ZFLTP: 0.123456789,
            // Decimal
            ZDEC: 12345.67,
            ZDECF16_MIN: 12345.67,
            ZDECF34_MIN: 12345.67,
            // Currency, Quantity
            ZCURR: 1234.56,
            ZQUAN: 12.3456,
            ZQUAN_SIGN: -12.345,
        };
        const EXPECTED_TYPES = {
            ZFLTP: "number",
            ZDEC: "number",
            ZCURR: "number",
            ZQUAN: "number",
            ZQUAN_SIGN: "number",
            ZDECF16_MIN: "number",
            ZDECF34_MIN: "number",
        };
        const xclient = direct_client("MME", { bcd: "number" });
        expect(xclient.config.clientOptions?.bcd).toEqual("number");
        void xclient.connect((err) => {
            expect(err).not.toBeDefined();
            xclient.invoke(
                "/COE/RBP_FE_DATATYPES",
                {
                    IS_INPUT: isInput,
                },
                function (err: unknown, res: RfcObject) {
                    expect(res).toHaveProperty("ES_OUTPUT");
                    const es_output = res.ES_OUTPUT as RfcStructure;
                    for (const [k, inVal] of Object.entries(isInput)) {
                        const outVal = es_output[k];
                        const outTyp = typeof outVal;
                        expect(outTyp).toEqual(EXPECTED_TYPES[k]);
                        expect(outVal).toEqual(inVal);
                    }
                    void xclient.close(() => {
                        done(err);
                    });
                }
            );
        });
    });

    test("BCD and FLOAT accept strings", function (done) {
        const isInput = {
            // Float
            ZFLTP: "0.123456789",
            // Decimal
            ZDEC: "12345.67",
            ZDECF16_MIN: "12345.67",
            ZDECF34_MIN: "12345.67",
            // Currency, Quantity
            ZCURR: "1234.56",
            ZQUAN: "12.3456",
            ZQUAN_SIGN: "-12.345",
        };
        const EXPECTED_TYPES = {
            ZFLTP: "number",
            ZDEC: "string",
            ZCURR: "string",
            ZQUAN: "string",
            ZQUAN_SIGN: "string",
            ZDECF16_MIN: "string",
            ZDECF34_MIN: "string",
        };
        expect(client.config.clientOptions?.bcd).toEqual("string");
        client.invoke(
            "/COE/RBP_FE_DATATYPES",
            {
                IS_INPUT: isInput,
            },
            function (err: unknown, res: RfcObject) {
                expect(res).toHaveProperty("ES_OUTPUT");
                const es_output = res.ES_OUTPUT as RfcStructure;
                for (const k in isInput) {
                    const outVal = es_output[k];
                    const outTyp = typeof outVal;
                    expect(outTyp).toEqual(EXPECTED_TYPES[k]);
                }
                done(err);
            }
        );
    });

    test("BCD and FLOAT accept Decimals", function (done) {
        const isInput = {
            ZFLTP: new Decimal("0.123456789"),

            // Decimal
            ZDEC: new Decimal("12345.67"),
            ZDECF16_MIN: new Decimal("12345.67"),
            ZDECF34_MIN: new Decimal("54321.76"),
            // Currency, Quantity
            ZCURR: new Decimal("1234.56"),
            ZQUAN: new Decimal("12.3456"),
            ZQUAN_SIGN: new Decimal("-12.345"),
        };
        const EXPECTED_TYPES = {
            ZFLTP: "number",
            ZDEC: "object",
            ZCURR: "object",
            ZQUAN: "object",
            ZQUAN_SIGN: "object",
            ZDECF16_MIN: "object",
            ZDECF34_MIN: "object",
        };
        const xclient = direct_client("MME", { bcd: Decimal });
        expect(xclient.config.clientOptions?.bcd).toEqual(Decimal);
        void xclient.connect(() => {
            xclient.invoke(
                "/COE/RBP_FE_DATATYPES",
                {
                    IS_INPUT: isInput,
                },
                function (err: unknown, res: RfcObject) {
                    expect(res).toHaveProperty("ES_OUTPUT");
                    const es_output = res.ES_OUTPUT as RfcStructure;
                    for (const [k, inVal] of Object.entries(isInput)) {
                        const outVal = es_output[k];
                        const outTyp = typeof outVal;
                        expect(outTyp).toEqual(EXPECTED_TYPES[k]);
                        if (k == "ZFLTP")
                            expect(inVal.toString()).toEqual(outVal.toString());
                        else expect(inVal).toEqual(outVal);
                    }
                    void xclient.close(() => {
                        done(err);
                    });
                }
            );
        });
    });

    test("RAW/BYTE accepts Buffer", function (done) {
        expect.assertions(2);
        const isInput = {
            ZRAW: XBYTES_TEST,
        };
        client.invoke(
            "/COE/RBP_FE_DATATYPES",
            {
                IS_INPUT: isInput,
            },
            function (err: unknown, res: RfcObject) {
                expect(
                    Buffer.compare(
                        isInput.ZRAW,
                        (res.ES_OUTPUT as RfcStructure).ZRAW as Buffer
                    )
                ).toBe(
                    -1 // because the ZRAW is 17 bytes long and padded with zeroes
                );
                expect(
                    Buffer.compare(
                        Buffer.concat([
                            isInput.ZRAW,
                            Buffer.from([0, 0, 0, 0]),
                        ]),
                        (res.ES_OUTPUT as RfcStructure).ZRAW as Buffer
                    )
                ).toBe(0); // ok
                done(err);
            }
        );
    });

    test("XSTRING accepts Buffer", function (done) {
        expect.assertions(1);
        const isInput = {
            ZRAWSTRING: XBYTES_TEST,
        };
        client.invoke(
            "/COE/RBP_FE_DATATYPES",
            {
                IS_INPUT: isInput,
            },
            function (err: unknown, res: RfcObject) {
                expect(
                    Buffer.compare(
                        isInput.ZRAWSTRING,
                        (res.ES_OUTPUT as RfcStructure).ZRAWSTRING as Buffer
                    )
                ).toBe(0);
                done(err);
            }
        );
    });

    test("CHAR with zero bytes in the middle", function (done) {
        client.invoke("ZLONG_STRING", {}, (err: unknown, res: RfcObject) => {
            const expres = "TESTUSER3 \x00\x00\x00\x0c 1000329";
            expect(res.EV_LONGCHAR).toEqual(expres);
            done(err);
        });
    });

    test.skip("BYTE and XSTRING tables", function (done) {
        const IT_SXMSMGUIDT = [] as RfcTable;
        const IT_SDOKCNTBINS = [] as RfcTable;

        const COUNT = 50;

        for (let i = 0; i < COUNT; i++) {
            // array -> unnamed structure
            IT_SXMSMGUIDT.push(XBYTES_TEST);
            IT_SXMSMGUIDT.push(Buffer.from(randomBytes(16)));
            IT_SXMSMGUIDT.push(Buffer.from(new Uint8Array(randomBytes(16))));

            // structure -> unnamed structure
            IT_SXMSMGUIDT.push({
                "": XBYTES_TEST,
            });
            IT_SXMSMGUIDT.push({
                "": Buffer.from(randomBytes(16)),
            });
            IT_SXMSMGUIDT.push({
                "": new Uint8Array(randomBytes(16)),
            });

            // named structure
            IT_SDOKCNTBINS.push({
                LINE: XBYTES_TEST,
            });
            IT_SDOKCNTBINS.push({
                LINE: Buffer.from(randomBytes(1022)),
            });
            IT_SDOKCNTBINS.push({
                LINE: new Uint8Array(randomBytes(1022)),
            });
        }

        const inp = {
            IT_SXMSMGUIDT: IT_SXMSMGUIDT,
            IT_SDOKCNTBINS: IT_SDOKCNTBINS,
        };
        client.invoke(
            "/COE/RBP_FE_DATATYPES",
            inp,
            function (err: unknown, result: RfcObject) {
                expect(result).toHaveProperty("ES_OUTPUT");

                expect(IT_SXMSMGUIDT.length).toBe(
                    (result.ET_SXMSMGUIDT as RfcTable).length
                );
                expect(IT_SDOKCNTBINS.length).toBe(
                    (result.ET_SDOKCNTBINS as RfcTable).length
                );

                for (let i = 0; i < IT_SXMSMGUIDT.length; i++) {
                    let lineIn = IT_SXMSMGUIDT[i];
                    if ("" in (lineIn as RfcStructure)) {
                        lineIn = lineIn[""] as RfcVariable;
                    }
                    const lineOut = (result.ET_SXMSMGUIDT as RfcTable)[
                        i
                    ] as Buffer;
                    expect(Buffer.compare(lineIn as Buffer, lineOut)).toBe(0);
                }

                for (let i = 0; i < IT_SDOKCNTBINS.length; i++) {
                    const lineIn = IT_SDOKCNTBINS[i]["LINE"] as Buffer;
                    const lineOut = (result.ET_SDOKCNTBINS as RfcTable)[i][
                        "LINE"
                    ] as Buffer;
                    expect(Buffer.compare(lineIn, lineOut)).toBe(0);
                }
                done(err);
            }
        );
    });

    test("DATE accepts string", function (done) {
        const testDate = "20180625";
        const importStruct = {
            RFCDATE: testDate,
        };
        const importTable = [importStruct];
        client.invoke(
            "STFC_STRUCTURE",
            {
                IMPORTSTRUCT: importStruct,
                RFCTABLE: importTable,
            },
            function (err: unknown, res: RfcObject) {
                expect(res).toHaveProperty("ECHOSTRUCT");
                expect((res.ECHOSTRUCT as RfcStructure).RFCDATE).toEqual(
                    testDate
                );
                expect(
                    ((res.RFCTABLE as RfcTable)[0] as RfcStructure).RFCDATE
                ).toEqual(testDate);
                done(err);
            }
        );
    });

    test.skip("DATE accepts Date", function (done) {
        const Months = [
            "01",
            "02",
            "03",
            "04",
            "05",
            "06",
            "07",
            "08",
            "09",
            "10",
            "11",
            "12",
        ];
        const xClientOptions = {
            date: {
                toABAP: toABAPdate,
                fromABAP: fromABAPdate,
            },
        } as RfcClientOptions;

        const xclient = direct_client("MME", xClientOptions);
        expect(xclient.config.clientOptions).toHaveProperty("date");
        expect(xclient.config.clientOptions?.date).toMatchObject(
            xClientOptions
        );

        void xclient.connect(() => {
            const abapDate = "20180725";
            const jsDate = fromABAPdate(abapDate);
            const importStruct = {
                RFCDATE: jsDate,
            };
            const importTable = [] as Array<RfcStructure>; // RfcTable;
            let count = 1;
            for (const month of Months) {
                importTable.push({
                    RFCDATE: fromABAPdate(`2018${month}${12 + count++}`),
                });
            }
            importTable.push({
                RFCDATE: fromABAPdate("20180101"),
            });
            importTable.push({
                RFCDATE: fromABAPdate("20181230"),
            });

            xclient.invoke(
                "STFC_STRUCTURE",
                {
                    IMPORTSTRUCT: importStruct,
                    RFCTABLE: importTable,
                },
                (err: unknown, res: RfcObject) => {
                    if (err) {
                        return done(err) as unknown;
                    }
                    expect(res).toHaveProperty("ECHOSTRUCT");
                    expect(res).toHaveProperty("RFCTABLE");
                    expect(
                        (res.ECHOSTRUCT as RfcStructure).RFCDATE.toString()
                    ).toEqual(jsDate.toString());

                    const rfc_table = res.RFCTABLE as Array<RfcStructure>;
                    for (let i = 0; i < rfc_table.length - 1; i++) {
                        expect(rfc_table[i].RFCDATE.toString()).toEqual(
                            importTable[i].RFCDATE.toString()
                        );
                    }
                    done();
                }
            );
        });
    });

    test("error: INT rejects string", function (done) {
        const importStruct = {
            RFCINT1: "1",
        };
        const importTable = [importStruct];
        client.invoke(
            "STFC_STRUCTURE",
            {
                IMPORTSTRUCT: importStruct,
                RFCTABLE: importTable,
            },
            function (err: unknown) {
                expect(err).toEqual(
                    expect.objectContaining({
                        message:
                            "Integer number expected from NodeJS for ABAP field of type 10",
                        name: "nodeRfcError",
                        rfmPath: {
                            field: "RFCINT1",
                            parameter: "IMPORTSTRUCT",
                            rfm: "STFC_STRUCTURE",
                            structure: "IMPORTSTRUCT",
                        },
                    })
                );
                done();
            }
        );
    });

    test("error: CHAR rejects int", function (done) {
        const importStruct = {
            RFCCHAR4: 65,
        };
        const importTable = [importStruct];
        client.invoke(
            "STFC_STRUCTURE",
            {
                IMPORTSTRUCT: importStruct,
                RFCTABLE: importTable,
            },
            function (err: unknown) {
                expect(err).toEqual(
                    expect.objectContaining({
                        message:
                            "String expected from NodeJS for ABAP field of type 0",
                        name: "nodeRfcError",
                        rfmPath: {
                            field: "RFCCHAR4",
                            parameter: "IMPORTSTRUCT",
                            rfm: "STFC_STRUCTURE",
                            structure: "IMPORTSTRUCT",
                        },
                    })
                );
                done();
            }
        );
    });

    test("error: BCD and FLOAT reject not a number string", function (done) {
        const importStruct = {
            RFCFLOAT: "A",
        };
        const importTable = [importStruct];
        client.invoke(
            "STFC_STRUCTURE",
            {
                IMPORTSTRUCT: importStruct,
                RFCTABLE: importTable,
            },
            function (err: unknown) {
                expect(err).toEqual(
                    expect.objectContaining({
                        code: 22,
                        key: "RFC_CONVERSION_FAILURE",
                        message:
                            "Cannot convert string value A at position 0 for the field RFCFLOAT to type RFCTYPE_FLOAT",
                    })
                );
                done();
            }
        );
    });

    test("error: DATE rejects number", function (done) {
        const testDate = 41;
        const importStruct = {
            RFCDATE: testDate,
        };
        const importTable = [importStruct];
        client.invoke(
            "STFC_STRUCTURE",
            {
                IMPORTSTRUCT: importStruct,
                RFCTABLE: importTable,
            },
            function (err: unknown) {
                expect(err).toEqual(
                    expect.objectContaining({
                        message:
                            "Date format YYYYMMDD expected from NodeJS for ABAP field of type 1",
                        name: "nodeRfcError",
                        rfmPath: {
                            field: "RFCDATE",
                            parameter: "IMPORTSTRUCT",
                            rfm: "STFC_STRUCTURE",
                            structure: "IMPORTSTRUCT",
                        },
                    })
                );
                done();
            }
        );
    });

    test("error: INT1 rejects float with fractional part", function (done) {
        const importStruct = {
            RFCINT1: 1 + Number.EPSILON,
            RFCINT2: 1.0,
        };
        const importTable = [importStruct];
        client.invoke(
            "STFC_STRUCTURE",
            {
                IMPORTSTRUCT: importStruct,
                RFCTABLE: importTable,
            },
            function (err: unknown) {
                expect(err).toEqual(
                    expect.objectContaining({
                        message:
                            "Integer number expected from NodeJS for ABAP field of type 10, got 1.0000000000000002",
                        name: "nodeRfcError",
                        rfmPath: {
                            field: "RFCINT1",
                            parameter: "IMPORTSTRUCT",
                            rfm: "STFC_STRUCTURE",
                            structure: "IMPORTSTRUCT",
                        },
                    })
                );
                done();
            }
        );
    });

    test("error: INT2 rejects float", function (done) {
        const importStruct = {
            RFCINT2: 1 + Number.EPSILON,
            RFCINT1: 1.0,
        };
        const importTable = [importStruct];
        client.invoke(
            "STFC_STRUCTURE",
            {
                IMPORTSTRUCT: importStruct,
                RFCTABLE: importTable,
            },
            function (err: unknown) {
                expect(err).toEqual(
                    expect.objectContaining({
                        message:
                            "Integer number expected from NodeJS for ABAP field of type 9, got 1.0000000000000002",
                        name: "nodeRfcError",
                        rfmPath: {
                            field: "RFCINT2",
                            parameter: "IMPORTSTRUCT",
                            rfm: "STFC_STRUCTURE",
                            structure: "IMPORTSTRUCT",
                        },
                    })
                );
                done();
            }
        );
    });

    test("error: INT4 rejects float", function (done) {
        const importStruct = {
            RFCINT4: 1 + Number.EPSILON,
        };
        const importTable = [importStruct];
        client.invoke(
            "STFC_STRUCTURE",
            {
                IMPORTSTRUCT: importStruct,
                RFCTABLE: importTable,
            },
            function (err: unknown) {
                expect(err).toEqual(
                    expect.objectContaining({
                        message:
                            "Integer number expected from NodeJS for ABAP field of type 8, got 1.0000000000000002",
                        name: "nodeRfcError",
                        rfmPath: {
                            field: "RFCINT4",
                            parameter: "IMPORTSTRUCT",
                            rfm: "STFC_STRUCTURE",
                            structure: "IMPORTSTRUCT",
                        },
                    })
                );
                done();
            }
        );
    });

    test("error: INT1 positive infinity", function (done) {
        const importStruct = {
            RFCINT1: Number.POSITIVE_INFINITY,
        };
        const importTable = [importStruct];
        client.invoke(
            "STFC_STRUCTURE",
            {
                IMPORTSTRUCT: importStruct,
                RFCTABLE: importTable,
            },
            function (err: unknown) {
                expect(err).toEqual(
                    expect.objectContaining({
                        message:
                            "Integer number expected from NodeJS for ABAP field of type 10, got Infinity",
                        name: "nodeRfcError",
                        rfmPath: {
                            field: "RFCINT1",
                            parameter: "IMPORTSTRUCT",
                            rfm: "STFC_STRUCTURE",
                            structure: "IMPORTSTRUCT",
                        },
                    })
                );
                done();
            }
        );
    });

    test("error: INT1 negative infinity", function (done) {
        const importStruct = {
            RFCINT1: Number.POSITIVE_INFINITY,
        };
        const importTable = [importStruct];
        client.invoke(
            "STFC_STRUCTURE",
            {
                IMPORTSTRUCT: importStruct,
                RFCTABLE: importTable,
            },
            function (err: unknown) {
                expect(err).toEqual(
                    expect.objectContaining({
                        message:
                            "Integer number expected from NodeJS for ABAP field of type 10, got Infinity",
                        name: "nodeRfcError",
                        rfmPath: {
                            field: "RFCINT1",
                            parameter: "IMPORTSTRUCT",
                            rfm: "STFC_STRUCTURE",
                            structure: "IMPORTSTRUCT",
                        },
                    })
                );
                done();
            }
        );
    });

    test("INT max positive", function (done) {
        const importStruct = {
            RFCINT1: 254,
            RFCINT2: 32766,
            RFCINT4: 2147483646,
        };
        const importTable = [importStruct];
        client.invoke(
            "STFC_STRUCTURE",
            {
                IMPORTSTRUCT: importStruct,
                RFCTABLE: importTable,
            },
            function (err: unknown, res: RfcObject) {
                expect(res).toBeDefined();
                expect(res).toHaveProperty("ECHOSTRUCT");
                expect(res).toHaveProperty("RFCTABLE");
                expect((res.ECHOSTRUCT as RfcStructure).RFCINT1).toBe(254);
                expect((res.RFCTABLE as RfcStructure[])[0].RFCINT1).toBe(254);
                expect((res.RFCTABLE as RfcStructure[])[1].RFCINT1).toBe(255);

                expect((res.ECHOSTRUCT as RfcStructure).RFCINT2).toBe(32766);
                expect((res.RFCTABLE as RfcStructure[])[0].RFCINT2).toBe(32766);
                expect((res.RFCTABLE as RfcStructure[])[1].RFCINT2).toBe(32767);

                expect((res.ECHOSTRUCT as RfcStructure).RFCINT4).toBe(
                    2147483646
                );
                expect((res.RFCTABLE[0] as RfcStructure).RFCINT4).toBe(
                    2147483646
                );
                expect((res.RFCTABLE[1] as RfcStructure).RFCINT4).toBe(
                    2147483647
                );
                done(err);
            }
        );
    });

    test("INT max negative", function (done) {
        const importStruct = {
            RFCINT1: 0,
            RFCINT2: -32768,
            RFCINT4: -2147483648,
        };
        const importTable = [importStruct];
        client.invoke(
            "STFC_STRUCTURE",
            {
                IMPORTSTRUCT: importStruct,
                RFCTABLE: importTable,
            },
            function (err: unknown, res: RfcObject) {
                expect(res).toBeDefined();
                expect(res).toHaveProperty("ECHOSTRUCT");
                expect(res).toHaveProperty("RFCTABLE");
                expect((res.ECHOSTRUCT as RfcStructure).RFCINT1).toBe(0);
                expect((res.RFCTABLE as RfcStructure[])[0].RFCINT1).toBe(0);
                expect((res.RFCTABLE as RfcStructure[])[1].RFCINT1).toBe(1);
                expect((res.ECHOSTRUCT as RfcStructure).RFCINT2).toBe(-32768);
                expect((res.RFCTABLE as RfcStructure[])[0].RFCINT2).toBe(
                    -32768
                );
                expect((res.RFCTABLE as RfcStructure[])[1].RFCINT2).toBe(
                    -32767
                );

                expect((res.ECHOSTRUCT as RfcStructure).RFCINT4).toBe(
                    -2147483648
                );
                expect((res.RFCTABLE as RfcStructure[])[0].RFCINT4).toBe(
                    -2147483648
                );
                expect((res.RFCTABLE as RfcStructure[])[1].RFCINT4).toBe(
                    -2147483647
                );
                done(err);
            }
        );
    });

    test("INT1 positive overflow", function (done) {
        const importStruct = {
            RFCINT1: 256,
            //RFCINT2: 32766,
            //RFCINT4: 2147483646
        };
        const importTable = [importStruct];
        client.invoke(
            "STFC_STRUCTURE",
            {
                IMPORTSTRUCT: importStruct,
                RFCTABLE: importTable,
            },
            function (err: unknown) {
                expect(err).toEqual(
                    expect.objectContaining({
                        message:
                            "Overflow or other error when putting NodeJS value 256 into ABAP integer field of type 10",
                        name: "nodeRfcError",
                        rfmPath: {
                            field: "RFCINT1",
                            parameter: "IMPORTSTRUCT",
                            rfm: "STFC_STRUCTURE",
                            structure: "IMPORTSTRUCT",
                        },
                    })
                );
                done();
            }
        );
    });

    test("INT2 positive overflow", function (done) {
        const importStruct = {
            RFCINT2: RFC_MATH.RFC_INT2.POS + 1,
        };
        const importTable = [importStruct];
        client.invoke(
            "STFC_STRUCTURE",
            {
                IMPORTSTRUCT: importStruct,
                RFCTABLE: importTable,
            },
            function (err: unknown) {
                expect(err).toEqual(
                    expect.objectContaining({
                        message:
                            "Overflow or other error when putting NodeJS value 32768 into ABAP integer field of type 9",
                        name: "nodeRfcError",
                        rfmPath: {
                            field: "RFCINT2",
                            parameter: "IMPORTSTRUCT",
                            rfm: "STFC_STRUCTURE",
                            structure: "IMPORTSTRUCT",
                        },
                    })
                );
                done();
            }
        );
    });

    test("INT2 negative overflow", function (done) {
        const importStruct = {
            RFCINT2: RFC_MATH.RFC_INT2.NEG - 1,
        };
        const importTable = [importStruct];
        client.invoke(
            "STFC_STRUCTURE",
            {
                IMPORTSTRUCT: importStruct,
                RFCTABLE: importTable,
            },
            function (err: unknown) {
                expect(err).toEqual(
                    expect.objectContaining({
                        message:
                            "Overflow or other error when putting NodeJS value -32769 into ABAP integer field of type 9",
                        name: "nodeRfcError",
                        rfmPath: {
                            field: "RFCINT2",
                            parameter: "IMPORTSTRUCT",
                            rfm: "STFC_STRUCTURE",
                            structure: "IMPORTSTRUCT",
                        },
                    })
                );
                done();
            }
        );
    });
});

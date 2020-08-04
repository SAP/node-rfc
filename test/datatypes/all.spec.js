// SPDX-FileCopyrightText: 2014 SAP SE Srdjan Boskovic <srdjan.boskovic@sap.com>
//
// SPDX-License-Identifier: Apache-2.0

"use strict";

describe("Datatypes: all", () => {
    const setup = require("../utils/setup");
    const client = setup.direct_client();

    const Decimal = require("decimal.js");
    const randomBytes = require("random-bytes");
    const Utils = require("../utils/utils");
    const RFC_MATH = require("../utils/config").RFC_MATH;

    beforeAll(function (done) {
        client.open((err) => {
            return done(err);
        });
    });

    afterAll(function (done) {
        client.close((err) => {
            return done(err);
        });
    });

    test("Min/Max positive", function (done) {
        let isInput = {
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
            function (err, res) {
                expect(err).toBeUndefined();
                expect(res).toHaveProperty("ES_OUTPUT");
                const output = res.ES_OUTPUT;

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
                done();
            }
        );
    });

    test("Min/Max negative", function (done) {
        let isInput = {
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
            function (err, res) {
                expect(err).toBeUndefined();
                expect(res).toHaveProperty("ES_OUTPUT");
                const output = res.ES_OUTPUT;

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
                done();
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
            function (err) {
                expect(err).toBeDefined();
                expect(err).toEqual(
                    expect.objectContaining({
                        message:
                            "Array expected from NodeJS, for RFM table of type 5",
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
        let isInput = {
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
        let xclient = setup.direct_client(setup.abapSystem(), {
            bcd: "number",
        });
        expect(xclient.config.clientOptions.bcd).toEqual("number");
        xclient.connect((err) => {
            expect(err).not.toBeDefined();
            xclient.invoke(
                "/COE/RBP_FE_DATATYPES",
                {
                    IS_INPUT: isInput,
                },
                function (err, res) {
                    expect(err).toBeUndefined();
                    expect(res).toHaveProperty("ES_OUTPUT");
                    for (let k in isInput) {
                        let inVal = isInput[k];
                        let outVal = res.ES_OUTPUT[k];
                        let outTyp = typeof outVal;
                        expect(outTyp).toEqual(EXPECTED_TYPES[k]);
                        expect(outVal).toEqual(inVal);
                    }
                    xclient.close(() => {
                        done();
                    });
                }
            );
        });
    });

    test("BCD and FLOAT accept strings", function (done) {
        let isInput = {
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
        expect(client.config.clientOptions.bcd).toEqual("string");
        client.invoke(
            "/COE/RBP_FE_DATATYPES",
            {
                IS_INPUT: isInput,
            },
            function (err, res) {
                expect(err).toBeUndefined();
                expect(res).toHaveProperty("ES_OUTPUT");
                for (let k in isInput) {
                    let inVal = isInput[k];
                    let outVal = res.ES_OUTPUT[k];
                    let outTyp = typeof outVal;
                    expect(outTyp).toEqual(EXPECTED_TYPES[k]);
                }
                done();
            }
        );
    });

    test("BCD and FLOAT accept Decimals", function (done) {
        let isInput = {
            ZFLTP: Decimal("0.123456789"),

            // Decimal
            ZDEC: Decimal("12345.67"),
            ZDECF16_MIN: Decimal("12345.67"),
            ZDECF34_MIN: Decimal("54321.76"),
            // Currency, Quantity
            ZCURR: Decimal("1234.56"),
            ZQUAN: Decimal("12.3456"),
            ZQUAN_SIGN: Decimal("-12.345"),
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
        let xclient = setup.direct_client(setup.abapSystem(), {
            bcd: Decimal,
        });
        expect(xclient.config.clientOptions.bcd).toEqual(Decimal);
        xclient.connect(() => {
            xclient.invoke(
                "/COE/RBP_FE_DATATYPES",
                {
                    IS_INPUT: isInput,
                },
                function (err, res) {
                    expect(err).toBeUndefined();
                    expect(res).toHaveProperty("ES_OUTPUT");
                    for (let k in isInput) {
                        let inVal = isInput[k];
                        let outVal = res.ES_OUTPUT[k];
                        let outTyp = typeof outVal;
                        expect(outTyp).toEqual(EXPECTED_TYPES[k]);
                        if (k == "ZFLTP")
                            expect(inVal.toString()).toEqual(outVal.toString());
                        else expect(inVal).toEqual(outVal);
                    }
                    xclient.close(() => {
                        done();
                    });
                }
            );
        });
    });

    test("RAW/BYTE accepts Buffer", function (done) {
        expect.assertions(3);
        let isInput = {
            ZRAW: Utils.XBYTES_TEST,
        };
        client.invoke(
            "/COE/RBP_FE_DATATYPES",
            {
                IS_INPUT: isInput,
            },
            function (err, res) {
                expect(err).toBeUndefined();
                expect(Buffer.compare(isInput.ZRAW, res.ES_OUTPUT.ZRAW)).toBe(
                    -1 // because the ZRAW is 17 bytes long and padded with zeroes
                );
                expect(
                    Buffer.compare(
                        Buffer.concat([
                            isInput.ZRAW,
                            Buffer.from([0, 0, 0, 0]),
                        ]),
                        res.ES_OUTPUT.ZRAW
                    )
                ).toBe(0); // ok
                done();
            }
        );
    });

    test("XSTRING accepts Buffer", function (done) {
        expect.assertions(2);
        let isInput = {
            ZRAWSTRING: Utils.XBYTES_TEST,
        };
        client.invoke(
            "/COE/RBP_FE_DATATYPES",
            {
                IS_INPUT: isInput,
            },
            function (err, res) {
                expect(err).toBeUndefined();
                expect(
                    Buffer.compare(isInput.ZRAWSTRING, res.ES_OUTPUT.ZRAWSTRING)
                ).toBe(0);
                done();
            }
        );
    });

    test.skip("BYTE and XSTRING tables", function (done) {
        let IT_SXMSMGUIDT = [];
        let IT_SDOKCNTBINS = [];

        const COUNT = 50;

        for (let i = 0; i < COUNT; i++) {
            // array -> unnamed structure
            IT_SXMSMGUIDT.push(Utils.XBYTES_TEST);
            IT_SXMSMGUIDT.push(new Buffer.from(randomBytes.sync(16)));
            IT_SXMSMGUIDT.push(new Uint8Array(randomBytes.sync(16)));

            // structure -> unnaamed structure
            IT_SXMSMGUIDT.push({
                "": Utils.XBYTES_TEST,
            });
            IT_SXMSMGUIDT.push({
                "": new Buffer.from(randomBytes.sync(16)),
            });
            IT_SXMSMGUIDT.push({
                "": new Uint8Array(randomBytes.sync(16)),
            });

            // named structure
            IT_SDOKCNTBINS.push({
                LINE: Utils.XBYTES_TEST,
            });
            IT_SDOKCNTBINS.push({
                LINE: new Buffer.from(randomBytes.sync(1022)),
            });
            IT_SDOKCNTBINS.push({
                LINE: new Uint8Array(randomBytes.sync(1022)),
            });
        }

        let inp = {
            IT_SXMSMGUIDT: IT_SXMSMGUIDT,
            IT_SDOKCNTBINS: IT_SDOKCNTBINS,
        };
        client.invoke("/COE/RBP_FE_DATATYPES", inp, function (err, result) {
            expect(err).toBeUndefined();
            expect(res).toHaveProperty("ES_OUTPUT");

            expect(IT_SXMSMGUIDT.length).toBe(result.ET_SXMSMGUIDT.length);
            expect(IT_SDOKCNTBINS.length).toBe(result.ET_SDOKCNTBINS.length);

            for (let i = 0; i < IT_SXMSMGUIDT.length; i++) {
                let lineIn = IT_SXMSMGUIDT[i];
                if ("" in lineIn) lineIn = lineIn[""];
                let lineOut = result.ET_SXMSMGUIDT[i];
                expect(Buffer.compare(lineIn, lineOut)).ToBe(0);
            }

            for (let i = 0; i < IT_SDOKCNTBINS.length; i++) {
                let lineIn = IT_SDOKCNTBINS[i].LINE;
                let lineOut = result.ET_SDOKCNTBINS[i].LINE;
                expect(Buffer.compare(lineIn, lineOut)).toBe(0);
            }
            done();
        });
    });

    test("DATE accepts string", function (done) {
        const testDate = "20180625";
        let importStruct = {
            RFCDATE: testDate,
        };
        let importTable = [importStruct];
        client.invoke(
            "STFC_STRUCTURE",
            {
                IMPORTSTRUCT: importStruct,
                RFCTABLE: importTable,
            },
            function (err, res) {
                expect(err).toBeUndefined();
                expect(res).toHaveProperty("ECHOSTRUCT");
                expect(res.ECHOSTRUCT.RFCDATE).toEqual(testDate);
                expect(res.RFCTABLE[0].RFCDATE).toEqual(testDate);
                done();
            }
        );
    });

    test("DATE accepts Date", function (done) {
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
        let xclient = setup.direct_client(setup.abapSystem(), {
            date: {
                toABAP: Utils.toABAPdate,
                fromABAP: Utils.fromABAPdate,
            },
        });
        expect(xclient.config.clientOptions.date).toHaveProperty("toABAP");
        expect(xclient.config.clientOptions.date).toHaveProperty("fromABAP");
        expect(xclient.config.clientOptions.date.toABAP).toBeInstanceOf(
            Function
        );
        expect(xclient.config.clientOptions.date.fromABAP).toBeInstanceOf(
            Function
        );
        xclient.connect(() => {
            const abapDate = "20180725";
            const jsDate = Utils.fromABAPdate(abapDate);
            let importStruct = {
                RFCDATE: jsDate,
            };
            let importTable = [];
            let count = 1;
            for (let month of Months) {
                importTable.push({
                    RFCDATE: Utils.fromABAPdate(`2018${month}${12 + count++}`),
                });
            }
            importTable.push({
                RFCDATE: Utils.fromABAPdate("20180101"),
            });
            importTable.push({
                RFCDATE: Utils.fromABAPdate("20181230"),
            });

            xclient.invoke(
                "STFC_STRUCTURE",
                {
                    IMPORTSTRUCT: importStruct,
                    RFCTABLE: importTable,
                },
                (err, res) => {
                    if (err) {
                        done(err);
                    }
                    expect(res).toHaveProperty("ECHOSTRUCT");
                    expect(res).toHaveProperty("RFCTABLE");
                    expect(res.ECHOSTRUCT.RFCDATE.toString()).toEqual(
                        jsDate.toString()
                    );

                    for (let i = 0; i < res.RFCTABLE.length - 1; i++) {
                        expect(res.RFCTABLE[i].RFCDATE.toString()).toEqual(
                            importTable[i].RFCDATE.toString()
                        );
                    }
                    done();
                }
            );
        });
    });

    test("error: INT rejects string", function (done) {
        let importStruct = {
            RFCINT1: "1",
        };
        let importTable = [importStruct];
        client.invoke(
            "STFC_STRUCTURE",
            {
                IMPORTSTRUCT: importStruct,
                RFCTABLE: importTable,
            },
            function (err) {
                expect(err).toBeDefined();
                expect(err).toEqual(
                    expect.objectContaining({
                        message:
                            "Integer number expected from NodeJS for the field of type 10",
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
        let importStruct = {
            RFCCHAR4: 65,
        };
        let importTable = [importStruct];
        client.invoke(
            "STFC_STRUCTURE",
            {
                IMPORTSTRUCT: importStruct,
                RFCTABLE: importTable,
            },
            function (err) {
                expect(err).toBeDefined();
                expect(err).toEqual(
                    expect.objectContaining({
                        message:
                            "Char expected from NodeJS for the field of type 0",
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
        let importStruct = {
            RFCFLOAT: "A",
        };
        let importTable = [importStruct];
        client.invoke(
            "STFC_STRUCTURE",
            {
                IMPORTSTRUCT: importStruct,
                RFCTABLE: importTable,
            },
            function (err) {
                expect(err).toBeDefined();
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
        let importStruct = {
            RFCDATE: testDate,
        };
        let importTable = [importStruct];
        client.invoke(
            "STFC_STRUCTURE",
            {
                IMPORTSTRUCT: importStruct,
                RFCTABLE: importTable,
            },
            function (err) {
                expect(err).toBeDefined();
                expect(err).toEqual(
                    expect.objectContaining({
                        message:
                            "ABAP date format YYYYMMDD expected from NodeJS for the field of type 1",
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
        let importStruct = {
            RFCINT1: 1 + Number.EPSILON,
            RFCINT2: 1.0,
        };
        let importTable = [importStruct];
        client.invoke(
            "STFC_STRUCTURE",
            {
                IMPORTSTRUCT: importStruct,
                RFCTABLE: importTable,
            },
            function (err) {
                expect(err).toBeDefined();
                expect(err).toEqual(
                    expect.objectContaining({
                        message:
                            "Integer number expected from NodeJS for the field of type 10, got 1.0000000000000002",
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
        let importStruct = {
            RFCINT2: 1 + Number.EPSILON,
            RFCINT1: 1.0,
        };
        let importTable = [importStruct];
        client.invoke(
            "STFC_STRUCTURE",
            {
                IMPORTSTRUCT: importStruct,
                RFCTABLE: importTable,
            },
            function (err) {
                expect(err).toBeDefined();
                expect(err).toEqual(
                    expect.objectContaining({
                        message:
                            "Integer number expected from NodeJS for the field of type 9, got 1.0000000000000002",
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
        let importStruct = {
            RFCINT4: 1 + Number.EPSILON,
        };
        let importTable = [importStruct];
        client.invoke(
            "STFC_STRUCTURE",
            {
                IMPORTSTRUCT: importStruct,
                RFCTABLE: importTable,
            },
            function (err) {
                expect(err).toBeDefined();
                expect(err).toEqual(
                    expect.objectContaining({
                        message:
                            "Integer number expected from NodeJS for the field of type 8, got 1.0000000000000002",
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
        let importStruct = {
            RFCINT1: Number.POSITIVE_INFINITY,
        };
        let importTable = [importStruct];
        client.invoke(
            "STFC_STRUCTURE",
            {
                IMPORTSTRUCT: importStruct,
                RFCTABLE: importTable,
            },
            function (err) {
                expect(err).toBeDefined();
                expect(err).toEqual(
                    expect.objectContaining({
                        message:
                            "Integer number expected from NodeJS for the field of type 10, got Infinity",
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
        let importStruct = {
            RFCINT1: Number.POSITIVE_INFINITY,
        };
        let importTable = [importStruct];
        client.invoke(
            "STFC_STRUCTURE",
            {
                IMPORTSTRUCT: importStruct,
                RFCTABLE: importTable,
            },
            function (err) {
                expect(err).toBeDefined();
                expect(err).toEqual(
                    expect.objectContaining({
                        message:
                            "Integer number expected from NodeJS for the field of type 10, got Infinity",
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
        let importStruct = {
            RFCINT1: 254,
            RFCINT2: 32766,
            RFCINT4: 2147483646,
        };
        let importTable = [importStruct];
        client.invoke(
            "STFC_STRUCTURE",
            {
                IMPORTSTRUCT: importStruct,
                RFCTABLE: importTable,
            },
            function (err, res) {
                expect(err).toBeUndefined();
                expect(res).toBeDefined();
                expect(res).toHaveProperty("ECHOSTRUCT");
                expect(res).toHaveProperty("RFCTABLE");
                expect(res.ECHOSTRUCT.RFCINT1).toBe(254);
                expect(res.RFCTABLE[0].RFCINT1).toBe(254);
                expect(res.RFCTABLE[1].RFCINT1).toBe(255);

                expect(res.ECHOSTRUCT.RFCINT2).toBe(32766);
                expect(res.RFCTABLE[0].RFCINT2).toBe(32766);
                expect(res.RFCTABLE[1].RFCINT2).toBe(32767);

                expect(res.ECHOSTRUCT.RFCINT4).toBe(2147483646);
                expect(res.RFCTABLE[0].RFCINT4).toBe(2147483646);
                expect(res.RFCTABLE[1].RFCINT4).toBe(2147483647);
                done();
            }
        );
    });

    test("INT max negative", function (done) {
        let importStruct = {
            RFCINT1: 0,
            RFCINT2: -32768,
            RFCINT4: -2147483648,
        };
        let importTable = [importStruct];
        client.invoke(
            "STFC_STRUCTURE",
            {
                IMPORTSTRUCT: importStruct,
                RFCTABLE: importTable,
            },
            function (err, res) {
                expect(err).toBeUndefined();
                expect(res).toBeDefined();
                expect(res).toHaveProperty("ECHOSTRUCT");
                expect(res).toHaveProperty("RFCTABLE");
                expect(res.ECHOSTRUCT.RFCINT1).toBe(0);
                expect(res.RFCTABLE[0].RFCINT1).toBe(0);
                expect(res.RFCTABLE[1].RFCINT1).toBe(1);

                expect(res.ECHOSTRUCT.RFCINT2).toBe(-32768);
                expect(res.RFCTABLE[0].RFCINT2).toBe(-32768);
                expect(res.RFCTABLE[1].RFCINT2).toBe(-32767);

                expect(res.ECHOSTRUCT.RFCINT4).toBe(-2147483648);
                expect(res.RFCTABLE[0].RFCINT4).toBe(-2147483648);
                expect(res.RFCTABLE[1].RFCINT4).toBe(-2147483647);
                done();
            }
        );
    });

    test("INT1 positive overflow", function (done) {
        let importStruct = {
            RFCINT1: 256,
            //RFCINT2: 32766,
            //RFCINT4: 2147483646
        };
        let importTable = [importStruct];
        client.invoke(
            "STFC_STRUCTURE",
            {
                IMPORTSTRUCT: importStruct,
                RFCTABLE: importTable,
            },
            function (err, res) {
                expect(err).toBeDefined();
                expect(err).toEqual(
                    expect.objectContaining({
                        message:
                            "Overflow or other error when putting NodeJS value 256 into integer field of type 10",
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
        let importStruct = {
            RFCINT2: RFC_MATH.RFC_INT2.POS + 1,
        };
        let importTable = [importStruct];
        client.invoke(
            "STFC_STRUCTURE",
            {
                IMPORTSTRUCT: importStruct,
                RFCTABLE: importTable,
            },
            function (err, res) {
                expect(err).toBeDefined();
                expect(err).toEqual(
                    expect.objectContaining({
                        message:
                            "Overflow or other error when putting NodeJS value 32768 into integer field of type 9",
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
        let importStruct = {
            RFCINT2: RFC_MATH.RFC_INT2.NEG - 1,
        };
        let importTable = [importStruct];
        client.invoke(
            "STFC_STRUCTURE",
            {
                IMPORTSTRUCT: importStruct,
                RFCTABLE: importTable,
            },
            function (err, res) {
                expect(err).toBeDefined();
                expect(err).toEqual(
                    expect.objectContaining({
                        message:
                            "Overflow or other error when putting NodeJS value -32769 into integer field of type 9",
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

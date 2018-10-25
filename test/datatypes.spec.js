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

'use strict';

const rfcClient = require('./noderfc').Client;
const abapSystem = require('./abapSystem')();

const should = require('should');
const Decimal = require('decimal.js');
const randomBytes = require('random-bytes');

const Utils = require('./utils');

describe('Datatypes', function() {
    let client = new rfcClient(abapSystem);

    beforeEach(function(done) {
        client.reopen(err => {
            done(err);
        });
    });

    afterEach(function(done) {
        client.close(() => {
            done();
        });
    });

    it('BCD and FLOAT accept numbers', function(done) {
        let isInput = {
            // Float
            ZFLTP: 0.123456789,

            // Decimal
            ZDEC: 12345.67,

            // Currency, Quantity
            ZCURR: 1234.56,
            ZQUAN: 12.3456,
            ZQUAN_SIGN: -12.345,
        };
        client.invoke('/COE/RBP_FE_DATATYPES', { IS_INPUT: isInput }, function(err, res) {
            should.not.exist(err);
            for (let k in isInput) {
                let inVal = isInput[k];
                let outVal = res.ES_OUTPUT[k];
                let outTyp = typeof outVal;
                if (k === 'ZFLTP') {
                    outTyp.should.equal('number');
                    outVal.should.equal(inVal);
                } else {
                    outTyp.should.equal('string');
                    outVal.should.equal(inVal.toString());
                }
            }
            done();
        });
    });

    it('BCD and FLOAT accept strings', function(done) {
        let isInput = {
            // Float
            ZFLTP: '0.123456789',
            // Decimal
            ZDEC: '12345.67',
            // Currency, Quantity
            ZCURR: '1234.56',
            ZQUAN: '12.3456',
            ZQUAN_SIGN: '-12.345',
        };
        client.invoke('/COE/RBP_FE_DATATYPES', { IS_INPUT: isInput }, function(err, res) {
            should.not.exist(err);
            for (let k in isInput) {
                let inVal = isInput[k];
                let outVal = res.ES_OUTPUT[k];
                let outTyp = typeof outVal;
                if (k === 'ZFLTP') {
                    outTyp.should.equal('number');
                    inVal.should.equal(outVal.toString());
                } else {
                    outTyp.should.equal('string');
                    outVal.should.equal(inVal);
                }
            }
            done();
        });
    });

    it('BCD and FLOAT accept Decimals, return strings', function(done) {
        let isInput = {
            ZFLTP: Decimal('0.123456789'),

            // Decimal
            ZDEC: Decimal('12345.67'),

            // Currency, Quantity
            ZCURR: Decimal('1234.56'),
            ZQUAN: Decimal('12.3456'),
            ZQUAN_SIGN: Decimal('-12.345'),
        };
        client.invoke('/COE/RBP_FE_DATATYPES', { IS_INPUT: isInput }, function(err, res) {
            should.not.exist(err);
            for (let k in isInput) {
                let inVal = isInput[k];
                let outVal = res.ES_OUTPUT[k];
                inVal.equals(outVal).should.equal(true);
            }
            done();
        });
    });

    it('RAW/BYTE accepts binary string', function(done) {
        let isInput = {
            ZRAW: Utils.XBYTES_TEST,
        };
        client.invoke('/COE/RBP_FE_DATATYPES', { IS_INPUT: isInput }, function(err, res) {
            should.not.exist(err);
            let test = Utils.compareBuffers(isInput.ZRAW, res.ES_OUTPUT.ZRAW);
            test.content.should.equal(true);
            done();
        });
    });

    it('RAW/BYTE accepts string', function(done) {
        let isInput = {
            ZRAW: Utils.XBYTES_TEST,
        };
        client.invoke('/COE/RBP_FE_DATATYPES', { IS_INPUT: isInput }, function(err, res) {
            should.not.exist(err);
            let test = Utils.compareBuffers(isInput.ZRAW, res.ES_OUTPUT.ZRAW);
            test.content.should.equal(true);
            done();
        });
    });

    it('RAW/BYTE accepts Buffer', function(done) {
        let isInput = {
            ZRAW: Buffer.alloc(17, '01234567890123456'),
        };
        client.invoke('/COE/RBP_FE_DATATYPES', { IS_INPUT: isInput }, function(err, res) {
            should.not.exist(err);
            let test = Utils.compareBuffers(isInput.ZRAW, res.ES_OUTPUT.ZRAW);
            test.content.should.equal(true);
            done();
        });
    });

    it('XSTRING accepts binary string', function(done) {
        let isInput = {
            ZRAWSTRING: Buffer.from('\xd8\x42\x43\x44\x45\x46\x47\x48\x49\x50\x51\x52\x53\x54\x55\x56\x57', 'hex'),
        };
        client.invoke('/COE/RBP_FE_DATATYPES', { IS_INPUT: isInput }, function(err, res) {
            should.not.exist(err);
            let test = Utils.compareBuffers(isInput.ZRAWSTRING, res.ES_OUTPUT.ZRAWSTRING);
            test.content.should.equal(true);
            done();
        });
    });

    it('XSTRING accepts string', function(done) {
        let isInput = {
            ZRAWSTRING: Buffer.from('abcdefghijklmnopq', 'hex'),
        };
        client.invoke('/COE/RBP_FE_DATATYPES', { IS_INPUT: isInput }, function(err, res) {
            should.not.exist(err);
            Buffer.alloc(isInput.ZRAWSTRING.length, isInput.ZRAWSTRING)
                .equals(res.ES_OUTPUT.ZRAWSTRING)
                .should.equal(true);
            done();
        });
    });

    it('XSTRING accepts Buffer', function(done) {
        let isInput = {
            ZRAWSTRING: Buffer.from('01234567890123456', 'hex'),
        };
        client.invoke('/COE/RBP_FE_DATATYPES', { IS_INPUT: isInput }, function(err, res) {
            should.not.exist(err);
            isInput.ZRAWSTRING.equals(res.ES_OUTPUT.ZRAWSTRING).should.equal(true);
            done();
        });
    });

    it('BYTE and XSTRING tables', function(done) {
        let IT_SXMSMGUIDT = [];
        let IT_SDOKCNTBINS = [];

        const COUNT = 50;

        for (let i = 0; i < COUNT; i++) {
            // array -> unnamed structure
            IT_SXMSMGUIDT.push(Utils.XBYTES_TEST);
            IT_SXMSMGUIDT.push(new Buffer.from(randomBytes.sync(16)));
            IT_SXMSMGUIDT.push(new Uint8Array(randomBytes.sync(16)));

            // structure -> unnaamed structure
            IT_SXMSMGUIDT.push({ '': Utils.XBYTES_TEST });
            IT_SXMSMGUIDT.push({ '': new Buffer.from(randomBytes.sync(16)) });
            IT_SXMSMGUIDT.push({ '': new Uint8Array(randomBytes.sync(16)) });

            // named structure
            IT_SDOKCNTBINS.push({ LINE: Utils.XBYTES_TEST });
            IT_SDOKCNTBINS.push({ LINE: new Buffer.from(randomBytes.sync(1022)) });
            IT_SDOKCNTBINS.push({ LINE: new Uint8Array(randomBytes.sync(1022)) });
        }

        let inp = {
            IT_SXMSMGUIDT: IT_SXMSMGUIDT,
            IT_SDOKCNTBINS: IT_SDOKCNTBINS,
        };
        client.invoke('/COE/RBP_FE_DATATYPES', inp, function(err, result) {
            should.not.exist(err);

            IT_SXMSMGUIDT.length.should.equal(result.ET_SXMSMGUIDT.length);
            IT_SDOKCNTBINS.length.should.equal(result.ET_SDOKCNTBINS.length);

            for (let i = 0; i < IT_SXMSMGUIDT.length; i++) {
                let lineIn = IT_SXMSMGUIDT[i];
                if ('' in lineIn) lineIn = lineIn[''];
                let lineOut = result.ET_SXMSMGUIDT[i];
                let test = Utils.compareBuffers(lineIn, lineOut);
                test.content.should.equal(true);
            }

            for (let i = 0; i < IT_SDOKCNTBINS.length; i++) {
                let lineIn = IT_SDOKCNTBINS[i].LINE;
                let lineOut = result.ET_SDOKCNTBINS[i].LINE;

                let test = Utils.compareBuffers(lineIn, lineOut);
                test.content.should.equal(true);
            }
            done();
        });
    });

    it('DATE accepts string', function(done) {
        const testDate = '20180625';
        let importStruct = {
            RFCDATE: testDate,
        };
        let importTable = [importStruct];
        client.invoke('STFC_STRUCTURE', { IMPORTSTRUCT: importStruct, RFCTABLE: importTable }, function(err, res) {
            should.not.exist(err);
            res.should.be.an.Object();
            res.ECHOSTRUCT.RFCDATE.should.equal(testDate);
            res.RFCTABLE[0].RFCDATE.should.equal(testDate);
            done();
        });
    });

    xit('DATE accepts Date', function(done) {
        const Months = ['01', '02', '03', '04', '05', '06', '07', '08', '09', '10', '11', '12'];
        let count = 0;
        for (let month of Months) {
            const testDateIn = new Date(`2018-${month}-25`);
            const testDateOut = Utils.toABAPdate(testDateIn);
            let importStruct = {
                RFCDATE: testDateIn,
            };
            let importTable = [importStruct];
            client
                .call('STFC_STRUCTURE', { IMPORTSTRUCT: importStruct, RFCTABLE: importTable })
                .then(res => {
                    res.should.be.an.Object();
                    res.ECHOSTRUCT.RFCDATE.should.equal(testDateOut);
                    res.RFCTABLE[0].RFCDATE.should.equal(testDateOut);
                    if (++count === Months.length) done();
                })
                .catch(err => {
                    done(err);
                });
        }
    });

    it('error: INT rejects string', function(done) {
        let importStruct = {
            RFCINT1: '1',
        };
        let importTable = [importStruct];
        client.invoke('STFC_STRUCTURE', { IMPORTSTRUCT: importStruct, RFCTABLE: importTable }, function(err) {
            should.exist(err);
            err.should.be.an.Object();
            err.should.have.properties({
                name: 'TypeError',
                message: 'Integer number expected when filling field RFCINT1 of type 10',
            });
            done();
        });
    });

    it('error: CHAR rejects string', function(done) {
        let importStruct = {
            RFCCHAR4: 65,
        };
        let importTable = [importStruct];
        client.invoke('STFC_STRUCTURE', { IMPORTSTRUCT: importStruct, RFCTABLE: importTable }, function(err) {
            should.exist(err);
            err.should.be.an.Object();
            err.should.have.properties({
                message: 'Char expected when filling field RFCCHAR4 of type 0',
            });
            done();
        });
    });

    it('error: BCD and FLOAT reject not a number string', function(done) {
        let importStruct = {
            RFCFLOAT: 'A',
        };
        let importTable = [importStruct];
        client.invoke('STFC_STRUCTURE', { IMPORTSTRUCT: importStruct, RFCTABLE: importTable }, function(err) {
            should.exist(err);
            err.should.be.an.Object();
            err.should.have.properties({
                code: 22,
                key: 'RFC_CONVERSION_FAILURE',
                message: 'Cannot convert string value A at position 0 for the field RFCFLOAT to type RFCTYPE_FLOAT',
            });
            done();
        });
    });

    it('error: DATE rejects number', function(done) {
        const testDate = 41;
        let importStruct = {
            RFCDATE: testDate,
        };
        let importTable = [importStruct];
        client.invoke('STFC_STRUCTURE', { IMPORTSTRUCT: importStruct, RFCTABLE: importTable }, function(err) {
            should.exist(err);
            err.should.be.an.Object();
            err.should.have.properties({
                name: 'TypeError',
                message: 'Date string YYYYMMDD expected when filling field RFCDATE of type 1',
            });
            done();
        });
    });

    it('error: INT1 rejects float with fractional part', function(done) {
        let importStruct = {
            RFCINT1: 1 + Number.EPSILON,
            RFCINT2: 1.0,
        };
        let importTable = [importStruct];
        client.invoke('STFC_STRUCTURE', { IMPORTSTRUCT: importStruct, RFCTABLE: importTable }, function(err) {
            should.exist(err);
            err.should.be.an.Object();
            err.should.have.properties({
                name: 'TypeError',
                message: 'Integer number expected when filling field RFCINT1 of type 10, got 0x1.0000000000001p+0',
            });
            done();
        });
    });

    it('error: INT2 rejects float', function(done) {
        let importStruct = {
            RFCINT2: 1 + Number.EPSILON,
            RFCINT1: 1.0,
        };
        let importTable = [importStruct];
        client.invoke('STFC_STRUCTURE', { IMPORTSTRUCT: importStruct, RFCTABLE: importTable }, function(err) {
            should.exist(err);
            err.should.be.an.Object();
            err.should.have.properties({
                name: 'TypeError',
                message: 'Integer number expected when filling field RFCINT2 of type 9, got 0x1.0000000000001p+0',
            });
            done();
        });
    });

    it('error: INT4 rejects float', function(done) {
        let importStruct = {
            RFCINT4: 1 + Number.EPSILON,
        };
        let importTable = [importStruct];
        client.invoke('STFC_STRUCTURE', { IMPORTSTRUCT: importStruct, RFCTABLE: importTable }, function(err) {
            should.exist(err);
            err.should.be.an.Object();
            err.should.have.properties({
                name: 'TypeError',
                message: 'Integer number expected when filling field RFCINT4 of type 8, got 0x1.0000000000001p+0',
            });
            done();
        });
    });

    it('error: INT1 positive infinity', function(done) {
        let importStruct = {
            RFCINT1: Number.POSITIVE_INFINITY,
        };
        let importTable = [importStruct];
        client.invoke('STFC_STRUCTURE', { IMPORTSTRUCT: importStruct, RFCTABLE: importTable }, function(err) {
            should.exist(err);
            err.should.be.an.Object();
            err.should.have.properties({
                name: 'TypeError',
                message: 'Integer number expected when filling field RFCINT1 of type 10, got inf',
            });
            done();
        });
    });

    it('error: INT1 negative infinity', function(done) {
        let importStruct = {
            RFCINT1: Number.POSITIVE_INFINITY,
        };
        let importTable = [importStruct];
        client.invoke('STFC_STRUCTURE', { IMPORTSTRUCT: importStruct, RFCTABLE: importTable }, function(err) {
            should.exist(err);
            err.should.be.an.Object();
            err.should.have.properties({
                name: 'TypeError',
                message: 'Integer number expected when filling field RFCINT1 of type 10, got inf',
            });
            done();
        });
    });

    // ABAP integers range https://help.sap.com/http.svc/rc/abapdocu_752_index_htm/7.52/en-US/index.htm?file=abenddic_builtin_types_intro.htm
    // RFCINT1: 0 to 255
    // RFCINT2: -32,768 to 32,767
    // RFCINT4: -2,147,483,648 to +2,147,483,647
    // RFCINT8: -9,223,372,036,854,775,808 to +9,223,372,036,854,775,807

    // JavaScript safe integers range https://www.ecma-international.org/ecma-262/8.0/#sec-number.max_safe_integer
    // MAX: 9,007,199,254,740,991 (2 ** 53 - 1)
    // MIN: -9,007,199,254,740,991

    it('INT max positive', function(done) {
        let importStruct = {
            RFCINT1: 254,
            RFCINT2: 32766,
            RFCINT4: 2147483646,
        };
        let importTable = [importStruct];
        client.invoke('STFC_STRUCTURE', { IMPORTSTRUCT: importStruct, RFCTABLE: importTable }, function(err, res) {
            should.not.exist(err);
            res.ECHOSTRUCT.RFCINT1.should.equal(254);
            res.RFCTABLE[0].RFCINT1.should.equal(254);
            res.RFCTABLE[1].RFCINT1.should.equal(255);

            res.ECHOSTRUCT.RFCINT2.should.equal(32766);
            res.RFCTABLE[0].RFCINT2.should.equal(32766);
            res.RFCTABLE[1].RFCINT2.should.equal(32767);

            res.ECHOSTRUCT.RFCINT4.should.equal(2147483646);
            res.RFCTABLE[0].RFCINT4.should.equal(2147483646);
            res.RFCTABLE[1].RFCINT4.should.equal(2147483647);
            done();
        });
    });

    it('INT max negative', function(done) {
        let importStruct = {
            RFCINT1: -2,
            RFCINT2: -32768,
            RFCINT4: -2147483648,
        };
        let importTable = [importStruct];
        client.invoke('STFC_STRUCTURE', { IMPORTSTRUCT: importStruct, RFCTABLE: importTable }, function(err, res) {
            should.not.exist(err);
            res.ECHOSTRUCT.RFCINT1.should.equal(254);
            res.RFCTABLE[0].RFCINT1.should.equal(254);
            res.RFCTABLE[1].RFCINT1.should.equal(255);

            res.ECHOSTRUCT.RFCINT2.should.equal(-32768);
            res.RFCTABLE[0].RFCINT2.should.equal(-32768);
            res.RFCTABLE[1].RFCINT2.should.equal(-32767);

            res.ECHOSTRUCT.RFCINT4.should.equal(-2147483648);
            res.RFCTABLE[0].RFCINT4.should.equal(-2147483648);
            res.RFCTABLE[1].RFCINT4.should.equal(-2147483647);
            done();
        });
    });
});

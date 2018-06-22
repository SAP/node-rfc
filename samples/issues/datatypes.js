const rfcClient = require('../lib').Client;
const Decimal = require('decimal.js');

const connParams = require('../test/connParams');

let client = new rfcClient(connParams);

let IS_INPUT = {
    ZACCP: '', // todo: no field text
    ZCHAR: '', // todo: no field text
    ZCHECKBOX: '', // Checkbox
    ZCLNT: '', // Client
    ZCUKY_DTEL: '', // Currency Key
    ZCURR: 0, // todo: no field text
    ZDATS: '', // todo: no field text
    ZDEC: 0, // todo: no field text
    ZFLTP: 0, // todo: no field text
    ZFVAL_LEIN_STATU: '', // Status of Storage Unit
    ZINT1: 0, // todo: no field text
    ZINT2: 0, // todo: no field text
    ZINT4: 0, // todo: no field text
    ZLANG: '', // Language Key
    ZLCHR: '', // todo: no field text
    ZLRAW: '', // todo: no field text
    ZNUMC: '', // todo: no field text
    ZPREC: 0, // todo: no field text
    ZQUAN: 0, // todo: no field text
    ZQUAN_SIGN: 0, // Difference quantity in alternate unit of measure
    ZRAW: '', // todo: no field text
    ZRAWSTRING: '', // todo: no field text
    ZSHLP_DEBI: '', // Customer Number
    ZSHLP_MAT1: '', // Material Number
    ZSHLP_PRODH: '', // Product hierarchy
    ZSSTRING: '', // todo: no field text
    ZSTRING: '', // todo: no field text
    ZTIMS: '', // todo: no field text
    ZUNIT_DTEL: '', // Condition unit
};
let bytes = new Uint32Array(20);
bytes[0] = 'H';
bytes[1] = 'e';
bytes[2] = 'l';
bytes[3] = 'l';
bytes[4] = 'o';

IS_INPUT = {
    // Text
    //ZACCP: '195808' /* ACCP 6, Posting period YYYYMM */,
    //ZCHAR: 'ABCDEFGHIJ' /* CHAR 10,  Character String */,
    //ZCLNT: '801' /* CLNT 3,  Client */,
    //ZLANG: 'E' /* LANG 1,  Language key, see T002, exit ISOLA */,
    //ZNUMC: '012345' /* NUMC 6,  Character string with only digits */,
    //ZLCHR: '' /* LCHR 257,  Long character string, requires preceding INT2 field */,
    // Checkbox
    //ZCHECKBOX: 'X' /* CHAR 1,  Checkbox */,
    // Date, Time
    //ZDATS: '20110316' /* DATS 8,  Date field (YYYYMMDD) stored as char(8) */,
    //ZTIMS: '123456' /* TIMS 6,  Time field (hhmmss), stored as char(6) */,
    // Input w. Domain Values
    //ZFVAL_LEIN_STATU: 'a' /* CHAR 1,  Status */,
    // Numbers
    //ZINT1: 254 /* INT1 3,  1-byte integer, integer number <= 254 */,
    //ZINT2: 32767 /* INT2 5,  2-byte integer, only for length field before LCHR or LRAW */,
    //ZINT4: 2147483647 /* INT4 10,  4-byte integer, integer number with sign */,
    //ZDEC: Decimal(1234567.89) /* DEC 7.2,  Counter or amount field with comma and sign */,
    //ZFLTP: 21474836470.0 /* FLTP 15.16,  Floating point number, accurate to 8 bytes */,
    //ZCURR: Decimal(1234567.89) /* CURR 7.2,  Currency field, stored as DEC */,
    //ZQUAN: Decimal(1234.5678) /* QUAN 4.4,  Quantity field, points to a unit field with format UNIT */,
    //ZQUAN_SIGN: Decimal(-1234567890.123) /* QUAN +10.3,  Difference Quantity */,
    //ZPREC: 0 /* PREC 2,  Precision of a QUAN field (obsolete) ! */,
    // Currency and quantuty keys, usually not used out of currency and quantity inputs
    //ZCUKY_DTEL: 'eur' /* CUKY 5,  Currency */,
    //ZUNIT_DTEL: 'Kg' /* UNIT 3,  Unit of measure */,
    // Strings
    // Byte
    //ZRAW: '\x41\x42\x43\x44\x45\x46\x47\x48\x49\x50\x51\x52\x53\x54\x55\x56\x57', // /* RAW 17,  Uninterpreted sequence of bytes */,
    //ZRAW: 'abcdefghijklmnopq', // /* RAW 17,  Uninterpreted sequence of bytes */,
    //ZRAW: Buffer('abcdefghijklmnopq'), // /* RAW 17,  Uninterpreted sequence of bytes */,
    //ZSSTRING: 'Lorem Ipsum [SSTRING]' /* SSTR 15,  Short Character String with Variable Length, 1 to 1333 */,
    // Xstring
    //ZRAWSTRING: '\x42', // '0001020304050607080910ABCDEF' /* RSTR -1,  Byte string of variable length */,
    //ZSTRING: 'Lorem Ipsum Lorem Ipsum [STRING]' /* STRG -1,  Character string of variable length */,
    ZRAWSTRING: 'abcd',
};

let NUMERIC_INPUTS = {
    decs: {
        // Float
        ZFLTP: Decimal('0.123456789'),

        // Decimal
        ZDEC: Decimal('12345.67'),

        // Currency, Quantity
        ZCURR: Decimal('1234.56'),
        ZQUAN: Decimal('12.3456'),
        ZQUAN_SIGN: Decimal('-12.345'),
    },

    numbers: {
        // Float
        ZFLTP: 0.123456789,

        // Decimal
        ZDEC: 12345.67,

        // Currency, Quantity
        ZCURR: 1234.56,
        ZQUAN: 12.3456,
        ZQUAN_SIGN: -12.345,
    },

    strings: {
        // Float
        ZFLTP: '0.123456789',

        // Decimal
        ZDEC: '12345.67',

        // Currency, Quantity
        ZCURR: '1234.56',
        ZQUAN: '12.3456',
        ZQUAN_SIGN: '-12.345',
    },
};

client.connect((err, res) => {
    if (err) {
        console.error('err:', err);
    } else {
        client.invoke('/COE/RBP_FE_DATATYPES', { IS_INPUT: IS_INPUT, IV_COUNT: 0 }, function(err, res) {
            //console.log(r.ES_OUTPUT);
            for (let k of Object.keys(IS_INPUT)) {
                if (k.indexOf('ZRAW') !== 0) continue;
                let inVal = IS_INPUT[k];
                let outVal = res.ES_OUTPUT[k];
                console.log('\n', k, typeof inVal, typeof outVal);
                console.log('\n', k, inVal.length, outVal.length);

                if (!Buffer(inVal).equals(Buffer(outVal))) {
                    console.log('\n', k, inVal.length, outVal.length);
                    console.log('\n', k, inVal, outVal);
                }
            }
            client.close();
        });
    }
});

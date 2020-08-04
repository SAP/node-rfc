// SPDX-FileCopyrightText: 2014 SAP SE Srdjan Boskovic <srdjan.boskovic@sap.com>
//
// SPDX-License-Identifier: Apache-2.0

// Numeric types
//
// ABAP:       https://help.sap.com/doc/abapdocu_752_index_htm/7.52/en-US/index.htm?file=abenddic_builtin_types_intro.htm
// JavaScript: https://www.ecma-international.org/ecma-262/10.0/index.html#Title
//
const RFC_MATH = {
    RFC_INT1: {
        MIN: 0,
        MAX: 255,
    },
    RFC_INT2: {
        NEG: -32768,
        POS: 32767,
    },
    RFC_INT4: {
        NEG: -2147483648,
        POS: 2147483647,
    },
    RFC_INT8: {
        NEG: -9223372036854775808,
        POS: 9223372036854775807,
    },
    FLOAT: {
        NEG: {
            MIN: "-2.2250738585072014E-308",
            MAX: "-1.7976931348623157E+308",
        },
        POS: {
            MIN: "2.2250738585072014E-308",
            MAX: "1.7976931348623157E+308",
        },
    },
    DECF16: {
        NEG: {
            MIN: "-1E-383",
            MAX: "-9.999999999999999E+384",
        },
        POS: {
            MIN: "1E-383",
            MAX: "9.999999999999999E+384",
        },
    },
    DECF34: {
        NEG: {
            MIN: "-1E-6143",
            MAX: "-9.999999999999999999999999999999999E+6144",
        },
        POS: {
            MIN: "1E-6143",
            MAX: "9.999999999999999999999999999999999E+6144",
        },
    },
    DATE: {
        MIN: "00010101",
        MAX: "99991231",
    },
    TIME: {
        MIN: "000000",
        MAX: "235959",
    },
    UTCLONG: {
        MIN: "0001-01-01T00:00:00.0000000",
        MAX: "9999-12-31T23:59:59.9999999",
        INITIAL: "0000-00-00T00:00:00.0000000",
    },
};

module.exports = {
    RFC_MATH: RFC_MATH,
};

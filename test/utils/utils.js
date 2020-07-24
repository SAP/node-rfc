let clientOptions = {};
clientOptions.bcd = {
    // Date to ABAP YYYYMMDD
    toABAP: function (date) {
        if (!(date instanceof Date))
            return new TypeError(`Date object required: ${date}`);
        let mm = date.getMonth() + 1;
        let dd = date.getUTCDate();
        return [
            date.getFullYear(),
            mm > 9 ? mm : "0" + mm,
            dd > 9 ? dd : "0" + dd,
        ].join("");
    },

    fromABAP: function (dats) {
        // ABAP YYYYMMDD to Date()
        return new Date(
            0 | dats.substring(0, 4),
            (0 | dats.substring(4, 6)) - 1,
            (0 | dats.substring(6, 8)) + 1
        );
    },
};

module.exports = {
    toABAPdate: function (date) {
        // Date to ABAP YYYYMMDD
        if (!(date instanceof Date))
            return new TypeError(`Date object required: ${date}`);
        let mm = date.getMonth() + 1;
        let dd = date.getUTCDate();
        return [
            date.getFullYear(),
            mm > 9 ? mm : "0" + mm,
            dd > 9 ? dd : "0" + dd,
        ].join("");
    },

    fromABAPdate: function (dats) {
        // ABAP YYYYMMDD to Date()
        return new Date(
            0 | dats.substring(0, 4),
            (0 | dats.substring(4, 6)) - 1,
            (0 | dats.substring(6, 8)) + 1
        );
    },

    // https://nodejs.org/api/buffer.html#buffer_buffers_and_character_encodings
    XBYTES_TEST: Buffer.from("01414243444549500051fdfeff", "hex"),
};

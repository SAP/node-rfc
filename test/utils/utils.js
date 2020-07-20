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

    compareBuffers(c1, c2) {
        //console.log(c1);
        //console.log(c2);
        return Buffer.compare(c1, c2);
        /*
        a1 = c1.constructor.name == "Buffer" ? c1 : Buffer.from(c1, "hex");
        a2 = c2.constructor.name == "Buffer" ? c2 : Buffer.from(c2, "hex");
        let compareResult = {
            content: true,
            length:
                a1.length == a2.length ? true : { a1: a1.length, a2: a2.length }
        };

        if (!a1.equals(a2)) {
            for (let i = 0; i < a1.length; i++) {
                if (a1[i] != a2[i]) {
                    compareResult.content = false;
                    compareResult.diff = {
                        pos: i,
                        a1: a1[i],
                        a2: a2[i],
                        a1: a1,
                        a2: a2
                    };
                    break;
                }
            }
        }
        return compareResult;
        */
    },
    // https://nodejs.org/api/buffer.html#buffer_buffers_and_character_encodings
    XBYTES_TEST: Buffer.from("01414243444549500051fdfeff", "hex"),
};

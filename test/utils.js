module.exports = {
    toABAPdate: function(date) {
        if (!(date instanceof Date)) return new TypeError(`Date object required: ${date}`);
        let mm = date.getMonth() + 1;
        let dd = date.getDate();
        return [date.getFullYear(), mm > 9 ? mm : '0' + mm, dd > 9 ? dd : '0' + dd].join('');
    },

    compareBuffers(a1, a2) {
        if (a1.constructor.name == 'String') a1 = Buffer.from(a1, 'ascii');
        if (a2.constructor.name == 'String') a2 = Buffer.from(a2, 'ascii');

        for (let i = 0; i < a1.length; i++) {
            if (a1[i] != a2[i]) return { pos: i, a1: a1[i], a2: a2[i] };
        }
        return true;
    },
};

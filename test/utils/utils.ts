// SPDX-FileCopyrightText: 2014 SAP SE Srdjan Boskovic <srdjan.boskovic@sap.com>
//
// SPDX-License-Identifier: Apache-2.0

// Date to ABAP YYYYMMDD
export function toABAPdate(date: Date): string {
    if (!(date instanceof Date))
        throw new TypeError(`Date object required: ${String(date)}`);
    const mm = date.getMonth() + 1;
    const dd = date.getUTCDate();
    return [
        date.getFullYear(),
        mm > 9 ? mm : `0${mm}`,
        dd > 9 ? dd : `0${dd}`,
    ].join("");
}

// ABAP YYYYMMDD to Date()
export function fromABAPdate(dats: string): Date {
    return new Date(
        +dats.substring(0, 4),
        +dats.substring(4, 6) - 1,
        1 + +dats.substring(6, 8)
    );
}

export const clientOptions = {
    bcd: {
        // Date to ABAP YYYYMMDD
        toABAP: toABAPdate,

        // ABAP YYYYMMDD to Date()
        fromABAP: fromABAPdate,
    },
};

// https://nodejs.org/api/buffer.html#buffer_buffers_and_character_encodings
export const XBYTES_TEST = Buffer.from("01414243444549500051fdfeff", "hex");

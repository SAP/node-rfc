// SPDX-FileCopyrightText: 2014 SAP SE Srdjan Boskovic <srdjan.boskovic@sap.com>
//
// SPDX-License-Identifier: Apache-2.0

const FULL = {
    user: "demo",
    passwd: "welcome",
    ashost: "10.68.110.51",
    sysnr: "00",
    client: "620",
    lang: "EN",
    //trace:"3",
};

export function abapSystem(sid = "MME", trace = false) {
    if (sid == "full") return FULL;
    const as = { dest: sid };
    if (trace) as["trace"] = "3";
    return as;
}

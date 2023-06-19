// SPDX-FileCopyrightText: 2014 SAP SE Srdjan Boskovic <srdjan.boskovic@sap.com>
//
// SPDX-License-Identifier: Apache-2.0

import { binding } from "./utils/setup";
import { LANGUAGES } from "./utils/config";

describe("Addon methods", () => {
    test("Language Conversions", function () {
        expect.assertions(2 * Object.keys(LANGUAGES).length);
        for (const [lang, L] of Object.entries(LANGUAGES)) {
            const sapLang = binding.languageIsoToSap(lang);
            expect(sapLang).toEqual(L.lang_sap);
            const isoLang = binding.languageSapToIso(sapLang);
            expect(isoLang).toEqual(lang);
        }
    });

    test("Language Conversions Errors", function () {
        expect.assertions(2);
        const errIso = "ŠĐ";
        const errSap = "Š";
        try {
            binding.languageIsoToSap(errIso);
        } catch (ex) {
            expect((ex as Error).message).toEqual(
                `Language ISO code not found: ${errIso}`
            );
        }
        try {
            binding.languageSapToIso(errSap);
        } catch (ex) {
            expect((ex as Error).message).toEqual(
                `Language SAP code not found: ${errSap}`
            );
        }
    });

    test("Reload INI file", function () {
        expect.assertions(1);
        try {
            binding.reloadIniFile();
            expect(1).toEqual(1);
        } catch {
            expect(1).toEqual(0);
        }
    });
});

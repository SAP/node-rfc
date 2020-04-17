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

"use strict";

const setup = require("../setup");
const QM7 = require("../abapSystem")("QM7");
const UTCLONG = require("../config").RFC_MATH.UTCLONG;
const client = setup.client(QM7);

beforeAll(() => {
    return client.open();
});

afterAll(() => {
    return client.close();
});

describe("Datatype: UTCLONG", () => {
    test("UTCLONG accepts min, max, initial", () => {
        return (async () => {
            let res = await client.call("ZDATATYPES", {
                IV_UTCLONG: UTCLONG.MIN,
            });
            expect(res.EV_UTCLONG).toEqual(UTCLONG.MIN);

            res = await client.call("ZDATATYPES", {
                IV_UTCLONG: UTCLONG.MAX,
            });
            expect(res.EV_UTCLONG).toEqual(UTCLONG.MAX);

            res = await client.call("ZDATATYPES", {
                IV_UTCLONG: UTCLONG.INITIAL,
            });
            expect(res.EV_UTCLONG).toEqual(UTCLONG.INITIAL);
        })();
    });

    test("UTCLONG rejects non string", () => {
        expect.assertions = 1;
        return client
            .call("ZDATATYPES", {
                IV_UTCLONG: 1,
            })
            .catch((ex) => {
                expect(ex).toEqual(
                    expect.objectContaining({
                        name: "TypeError",
                        message:
                            "UTCLONG string expected when filling field IV_UTCLONG of type 32",
                    })
                );
            });
    });

    test("UTCLONG rejects invalid format", () => {
        expect.assertions = 1;
        return client
            .call("ZDATATYPES", {
                IV_UTCLONG: "1",
            })
            .catch((ex) => {
                expect(ex).toEqual(
                    expect.objectContaining({
                        name: "RfcLibError",
                        code: 22,
                        key: "RFC_CONVERSION_FAILURE",
                        message:
                            "Cannot convert 1 to RFCTYPE_UTCLONG : illegal format",
                    })
                );
            });
    });
});

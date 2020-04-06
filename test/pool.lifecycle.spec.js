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

const setup = require("./setup");
const Pool = setup.rfcPool;
const abapSystem = setup.abapSystem;

describe('Pool', () => {

    it("lifecycle", function () {
        expect.assertions(7);
        const pool = new Pool(abapSystem, {
            min: 4,
            max: 8
        });
        return (async () => {
            expect(pool.status).toEqual({
                active: 0,
                ready: 0,
                options: {
                    min: 4,
                    max: 8
                }
            });

            let c1 = await pool.acquire();
            expect(pool.status).toEqual({
                active: 1,
                ready: 3,
                options: {
                    min: 4,
                    max: 8
                }

            })

            let c2 = await pool.acquire();
            expect(pool.status).toEqual({
                active: 2,
                ready: 3,
                options: {
                    min: 4,
                    max: 8
                }
            })

            let c3 = await pool.acquire();
            expect(pool.status).toEqual({
                active: 3,
                ready: 3,
                options: {
                    min: 4,
                    max: 8
                }
            })

            let c4 = await pool.acquire();
            expect(pool.status).toEqual({
                active: 4,
                ready: 3,
                options: {
                    min: 4,
                    max: 8
                }
            })

            await pool.release(c1);
            expect(pool.status).toEqual({
                active: 3,
                ready: 4,
                options: {
                    min: 4,
                    max: 8
                }
            })

            await pool.releaseAll();
            expect(pool.status).toEqual({
                active: 3,
                ready: 0,
                options: {
                    min: 4,
                    max: 8
                }
            });
        })()
    });
})

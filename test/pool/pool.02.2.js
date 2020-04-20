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

module.exports = () => {
    const setup = require("../testutils/setup");
    const Pool = setup.rfcPool;
    const abapSystem = setup.abapSystem;

    const pool = new Pool(abapSystem);

    test("Acquire 3, release 3", function () {
        expect.assertions(7);

        return (async () => {
            expect(pool.status.active).toEqual(0);

            let c1 = await pool.acquire();
            expect(pool.status.active).toEqual(1);

            let c2 = await pool.acquire();
            expect(pool.status.active).toEqual(2);

            let c3 = await pool.acquire();
            expect(pool.status.active).toEqual(3);

            await pool.release(c1);
            expect(pool.status.active).toEqual(2);

            await pool.release(c2);
            expect(pool.status.active).toEqual(1);

            await pool.release(c3);
            expect(pool.status.active).toEqual(0);
        })();
    });

    afterAll((done) => {
        setTimeout(() => {
            pool.releaseAll().then((closed) => {
                done();
            });
        }, 2000);
    });
};

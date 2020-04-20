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

    test("Release without client", function (done) {
        expect.assertions(1);
        pool.acquire().then(() => {
            pool.release().catch((ex) => {
                expect(ex).toEqual(
                    expect.objectContaining(
                        new TypeError(
                            "Pool release() method requires a client instance as argument"
                        )
                    )
                );
                done();
            });
        });
    });

    afterAll(function (done) {
        setTimeout(() => {
            pool.releaseAll().then((closed) => {
                //console.log("released", closed);
                done();
            });
        }, 2000);
    });
};

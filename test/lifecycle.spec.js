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

describe('Lifecycle', () => {
    it("Connection Lifecycle Promises", function () {
        const client = require("./setup").client();
        expect(client.status.created).toBeGreaterThan(0);
        expect(client.status.lastopen).toBe(0);
        expect(client.status.lastcall).toBe(0);
        expect(client.status.lastclose).toBe(0);

        return (async function () {
            await client.open();
            expect(client.status.created).toBeGreaterThan(0);
            expect(client.status.lastopen).toBeGreaterThan(client.status.created);
            expect(client.status.lastcall).toBe(0);
            expect(client.status.lastclose).toBe(0);

            let result = await client.call('/COE/RBP_FE_WAIT', {
                IV_SECONDS: 1
            });
            expect(client.status.created).toBeGreaterThan(0);
            expect(client.status.lastopen).toBeGreaterThan(client.status.created);
            expect(client.status.lastcall).toBeGreaterThan(client.status.lastopen);
            expect(client.status.lastclose).toBe(0);

            await client.close();
            expect(client.status.created).toBeGreaterThan(0);
            expect(client.status.lastopen).toBeGreaterThan(client.status.created);
            expect(client.status.lastcall).toBeGreaterThan(client.status.lastopen)
            expect(client.status.lastclose).toBeGreaterThan(client.status.lastcall);
        })()
    });

    it("Connection Lifecycle Callbacks", function (done) {

        const client = require("./setup").client();
        expect(client.status.created).toBeGreaterThan(0);
        expect(client.status.lastopen).toBe(0);
        expect(client.status.lastcall).toBe(0);
        expect(client.status.lastclose).toBe(0);

        client.connect(err => {
            if (err) done(err);
            expect(client.status.created).toBeGreaterThan(0);
            expect(client.status.lastopen).toBeGreaterThan(client.status.created);
            expect(client.status.lastcall).toBe(0);
            expect(client.status.lastclose).toBe(0);;

            client.invoke('/COE/RBP_FE_WAIT', {
                IV_SECONDS: 1
            }, (err, res) => {
                if (err) done(err);
                expect(client.status.created).toBeGreaterThan(0);
                expect(client.status.lastopen).toBeGreaterThan(client.status.created);
                expect(client.status.lastcall).toBeGreaterThan(client.status.lastopen);
                expect(client.status.lastclose).toBe(0);

                client.close(() => {
                    expect(client.status.created).toBeGreaterThan(0);
                    expect(client.status.lastopen).toBeGreaterThan(client.status.created);
                    expect(client.status.lastcall).toBeGreaterThan(client.status.lastopen)
                    expect(client.status.lastclose).toBeGreaterThan(client.status.lastcall);
                    done();
                })
            });
        });
    });
})

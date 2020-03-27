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

const client = require("./setup").client;

it("Connection Lifecycle", function () {
    return (async function () {
        expect(client.status.created).toBeGreaterThan(0);
        expect(client.status.lastopen).toBe(0);
        expect(client.status.lastcall).toBe(0);
        expect(client.status.lastclose).toBe(0);

        await client.open();
        expect(client.status.created).toBeGreaterThan(0);
        expect(client.status.lastopen).toBeGreaterThan(0);
        expect(client.status.lastcall).toBe(0);
        expect(client.status.lastclose).toBe(0);

        let result = await client.call('STFC_CONNECTION', {
            REQUTEXT: 'H€llö SAP!'
        });
        expect(client.status.created).toBeGreaterThan(0);
        expect(client.status.lastopen).toBeGreaterThan(0);
        expect(client.status.lastcall).toBeGreaterThan(0)
        expect(client.status.lastclose).toBe(0);

        await client.close();
        expect(client.status.created).toBeGreaterThan(0);
        expect(client.status.lastopen).toBeGreaterThan(0);
        expect(client.status.lastcall).toBeGreaterThan(0)
        expect(client.status.lastclose).toBeGreaterThan(0);

        expect(client.status.lastclose).toBeGreaterThan(client.status.lastcall);
        expect(client.status.lastcall).toBeGreaterThan(client.status.lastopen);
        expect(client.status.lastopen).toBeGreaterThan(client.status.created);
    })()
});

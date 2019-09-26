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
const client = setup.client;

beforeEach(function() {
    return client.reopen();
});

afterEach(function() {
    return client.close();
});

afterAll(function(done) {
    delete setup.client;
    delete setup.rfcClient;
    delete setup.rfcPool;
    done();
});

it("call() STFC_CONNECTION should return unicode string", function() {
    return client
        .call("STFC_CONNECTION", {
            REQUTEXT: setup.UNICODETEST
        })
        .then(res => {
            expect(res).toBeDefined();
            expect(res).toHaveProperty("ECHOTEXT");
            expect(res.ECHOTEXT.indexOf(setup.UNICODETEST)).toEqual(0);
        });
});

it("call() STFC_STRUCTURE should return structure and table", function() {
    let importStruct = {
        RFCFLOAT: 1.23456789,
        RFCCHAR1: "A",
        RFCCHAR2: "BC",
        RFCCHAR4: "DEFG",

        RFCINT1: 1,
        RFCINT2: 2,
        RFCINT4: 345,

        RFCHEX3: Buffer.from("\x01\x02\x03", "ascii"),

        RFCTIME: "121120",
        RFCDATE: "20140101",

        RFCDATA1: "1DATA1",
        RFCDATA2: "DATA222"
    };
    let importTable = [importStruct];

    return client
        .call("STFC_STRUCTURE", {
            IMPORTSTRUCT: importStruct,
            RFCTABLE: importTable
        })
        .then(res => {
            expect(res).toBeDefined();
            expect(res).toHaveProperty("ECHOSTRUCT");
            expect(res).toHaveProperty("RFCTABLE");

            expect(res.ECHOSTRUCT.RFCCHAR1).toEqual(importStruct.RFCCHAR1);
            expect(res.ECHOSTRUCT.RFCCHAR2).toEqual(importStruct.RFCCHAR2);
            expect(res.ECHOSTRUCT.RFCCHAR4).toEqual(importStruct.RFCCHAR4);
            expect(res.ECHOSTRUCT.RFCFLOAT).toEqual(importStruct.RFCFLOAT);
            expect(res.ECHOSTRUCT.RFCINT1).toEqual(importStruct.RFCINT1);
            expect(res.ECHOSTRUCT.RFCINT2).toEqual(importStruct.RFCINT2);
            expect(res.ECHOSTRUCT.RFCINT4).toEqual(importStruct.RFCINT4);
            expect(res.ECHOSTRUCT.RFCDATA1).toContain(importStruct.RFCDATA1);
            expect(res.ECHOSTRUCT.RFCDATA2).toContain(importStruct.RFCDATA2);
            expect(res.ECHOSTRUCT.RFCHEX3.toString()).toEqual(
                importStruct.RFCHEX3.toString()
            );

            expect(res.RFCTABLE.length).toBe(2);
            expect(res.RFCTABLE[1].RFCFLOAT).toEqual(importStruct.RFCFLOAT + 1);
            expect(res.RFCTABLE[1].RFCINT1).toEqual(importStruct.RFCINT1 + 1);
            expect(res.RFCTABLE[1].RFCINT2).toEqual(importStruct.RFCINT2 + 1);
            expect(res.RFCTABLE[1].RFCINT4).toEqual(importStruct.RFCINT4 + 1);
        });
});

it("isAlive and ping() should be true when connected", function() {
    expect(client.isAlive).toBeTruthy();
    return client.ping().then(res => {
        expect(res).toBeTruthy();
    });
});

it("isAlive ands ping() should be false when disconnected", function(done) {
    client.close().then(() => {
        expect(client.isAlive).toBeFalsy();
        client.ping().then(res => {
            expect(res).toBeFalsy();
            done();
        });
    });
});

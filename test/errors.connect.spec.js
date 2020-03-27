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

it("error: new client requires connection parameters", function (done) {
    try {
        new setup.rfcClient();
    } catch (err) {
        expect(err).toBeDefined();
        expect(err).toEqual(
            expect.objectContaining({
                name: "TypeError",
                message: "Connection parameters must be an object"
            })
        );
        done();
    }
});

it("error: connect() requires minimum of connection parameters", function (done) {
    let wrongParams = Object.assign({}, setup.abapSystem);
    delete wrongParams.ashost;

    let wrongClient = new setup.rfcClient(wrongParams);
    wrongClient.connect(function (err) {
        expect(err).toBeDefined();
        expect(err).toEqual(
            expect.objectContaining({
                message: "Parameter ASHOST, GWHOST, MSHOST or PORT is missing.",
                code: 20,
                key: "RFC_INVALID_PARAMETER",
                name: "RfcLibError"
            })
        );
        done();
    });
});

it("error: conect() rejects invalid credentials", function (done) {
    let wrongParams = Object.assign({}, setup.abapSystem);
    wrongParams.user = "WRONGUSER";

    let wrongClient = new setup.rfcClient(wrongParams);
    wrongClient.connect(function (err) {
        expect(err).toBeDefined();
        expect(err).toEqual(
            expect.objectContaining({
                message: "Name or password is incorrect (repeat logon)",
                code: 2,
                key: "RFC_LOGON_FAILURE"
            })
        );
        done();
    });
});

it("error: connect() requires a callback function", function (done) {
    try {
        client.connect();
    } catch (err) {
        expect(err).toBeDefined();
        expect(err).toEqual(
            expect.objectContaining({
                name: "TypeError",
                message: "First argument must be callback function"
            })
        );
        done();
    }
});

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

beforeEach(function (done) {
    client.reopen(function (err) {
        done(err);
    });
});

afterEach(function (done) {
    client.close(function () {
        done();
    });
});

it("error: invoke() requires at least three arguments", function (done) {
    try {
        client.invoke("rfc", {});
    } catch (err) {
        expect(err).toBeDefined();
        expect(err).toEqual(
            expect.objectContaining({
                message: "Callback function must be supplied"
            })
        );
        done();
    }
});

it("error: invoke() rejects non-string rfm name", function (done) {
    client.invoke(23, {}, function (err) {
        expect(err).toBeDefined();
        expect(err).toEqual(
            expect.objectContaining({
                name: "TypeError",
                message: "First argument (remote function module name) must be an string"
            })
        );
        done();
    });
});

it("error: invoke() rejects non-existing parameter", function (done) {
    client.invoke("STFC_CONNECTION", {
        XXX: "wrong param"
    }, function (err) {
        expect(err).toBeDefined();
        expect(err).toEqual(
            expect.objectContaining({
                code: 20,
                key: "RFC_INVALID_PARAMETER",
                message: "field 'XXX' not found"
            })
        );
        done();
    });
});

it("error: non-existing field in input structure", function (done) {
    let importStruct = {
        XRFCCHAR1: "A",
        RFCCHAR2: "BC",
        RFCCHAR4: "DEFG"
    };

    client.invoke("STFC_STRUCTURE", {
        IMPORTSTRUCT: importStruct
    }, function (
        err
    ) {
        expect(err).toBeDefined();
        expect(err).toEqual(
            expect.objectContaining({
                name: "RfcLibError",
                code: 20,
                key: "RFC_INVALID_PARAMETER",
                message: "field 'XRFCCHAR1' not found"
            })
        );
        done();
    });
});

it("error: non-existing field in input table", function (done) {
    let importTable = [{
        XRFCCHAR1: "A",
        RFCCHAR2: "BC",
        RFCCHAR4: "DEFG"
    }];

    client.invoke("STFC_STRUCTURE", {
        RFCTABLE: importTable
    }, function (err) {
        expect(err).toBeDefined();
        expect(err).toEqual(
            expect.objectContaining({
                name: "RfcLibError",
                code: 20,
                key: "RFC_INVALID_PARAMETER",
                message: "field 'XRFCCHAR1' not found"
            })
        );
        done();
    });
});

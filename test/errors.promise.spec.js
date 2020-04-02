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

beforeEach(function () {
    return client.reopen();
});

afterEach(function () {
    return client.close();
});

it("error: call() promise rejects invalid credentials", function () {
    let wrongParams = Object.assign({}, setup.abapSystem);
    wrongParams.user = "WRONGUSER";

    let wrongClient = new setup.rfcClient(wrongParams);

    expect.assertions(1);
    return wrongClient
        .open()
        .catch(err => {
            expect(err).toEqual(
                expect.objectContaining({
                    message: "Name or password is incorrect (repeat logon)",
                    code: 2,
                    key: "RFC_LOGON_FAILURE"
                })
            );
        });
});

it("error: call() promise rejects non-existing parameter", function () {
    expect.assertions(1);
    return client
        .call("STFC_CONNECTION", {
            XXX: "wrong param"
        })
        .catch(err => {
            expect(err).toEqual(
                expect.objectContaining({
                    code: 20,
                    key: "RFC_INVALID_PARAMETER",
                    message: "field 'XXX' not found"
                })
            );
        });
});

it("error: promise call() RFC_RAISE_ERROR", function () {
    expect.assertions(1);
    return client
        .call("RFC_RAISE_ERROR", {
            MESSAGETYPE: "A"
        })
        .catch(err => {
            expect(err).toEqual(
                expect.objectContaining({
                    code: 4,
                    key: "Function not supported",
                    abapMsgClass: "SR",
                    abapMsgType: "A",
                    abapMsgNumber: "006",
                    message: "Function not supported"
                })
            );
        });
});

it("error: open() promise requires minimum of connection parameters", function () {
    let wrongParams = Object.assign({}, setup.abapSystem);
    delete wrongParams.ashost;
    let wrongClient = new setup.rfcClient(wrongParams);

    expect.assertions(1);
    return wrongClient
        .open()
        .catch(err => {
            expect(err).toEqual(
                expect.objectContaining({
                    message: "Parameter ASHOST, GWHOST, MSHOST or PORT is missing.",
                    code: 20,
                    key: "RFC_INVALID_PARAMETER",
                    name: "RfcLibError"
                })
            );
        });
});

it("error: promise call() requires at least two arguments", function () {
    expect.assertions(1);
    return client.call("rfc").catch(err => {
        expect(err).toEqual(
            expect.objectContaining({
                name: "TypeError",
                message: "Please provide remote function module name and parameters as arguments"
            })
        );
    });
});

it("error: promise call() rejects non-string rfm name", function () {
    expect.assertions(1);
    return client.call(23, {}, 2).catch(err => {
        expect(err).toEqual(
            expect.objectContaining({
                name: "TypeError",
                message: "First argument (remote function module name) must be an string"
            })
        );
    });
});

it("error: promise call() rejects non-object second argument (remote function module parameters)", function () {
    expect.assertions(1);
    return client.call("rfc", 41, 2).catch(err => {
        expect(err).toEqual(
            expect.objectContaining({
                name: "TypeError",
                message: "Second argument (remote function module parameters) must be an object"
            })
        );
    });
});

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

'use strict';

const setup = require('./setup');
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

afterAll(function (done) {
    delete setup.client;
    delete setup.rfcClient;
    delete setup.rfcPool;
    done();
});

it('error: new client requires connection parameters', function (done) {
    try {
        new setup.rfcClient();
    } catch (err) {
        expect(err).toBeDefined();
        expect(err).toEqual(expect.objectContaining({
            name: 'TypeError',
            message: 'Connection parameters must be an object',
        }));
        done();
    }
});

it('error: connect() requires minimum of connection parameters', function (done) {
    let wrongParams = Object.assign({}, setup.abapSystem);
    delete wrongParams.ashost;

    let wrongClient = new setup.rfcClient(wrongParams);
    wrongClient.connect(function (err) {
        expect(err).toBeDefined();
        expect(err).toEqual(expect.objectContaining({
            message: 'Parameter ASHOST, GWHOST, MSHOST or SERVER_PORT is missing.',
            code: 20,
            key: 'RFC_INVALID_PARAMETER',
        }));
        done();
    });
});

it('error: conect() rejects invalid credentials', function (done) {
    let wrongParams = Object.assign({}, setup.abapSystem);
    wrongParams.user = 'WRONGUSER';

    let wrongClient = new setup.rfcClient(wrongParams);
    wrongClient.connect(function (err) {
        expect(err).toBeDefined();
        expect(err).toEqual(expect.objectContaining({
            message: 'Name or password is incorrect (repeat logon)',
            code: 2,
            key: 'RFC_LOGON_FAILURE',
        }));
        done();
    });
});

it('error: connect() requires a callback function', function (done) {
    try {
        client.connect();
    } catch (err) {
        expect(err).toBeDefined();
        expect(err).toEqual(expect.objectContaining({
            name: 'TypeError',
            message: 'First argument must be callback function',
        }));
        done();
    }
});

it('error: invoke() requires at least three arguments', function (done) {
    try {
        client.invoke('rfc', {});
    } catch (err) {
        expect(err).toBeDefined();
        expect(err).toEqual(expect.objectContaining({
            message: 'Callback function must be supplied',
        }));
        done();
    }
});

it('error: invoke() rejects non-existing parameter', function (done) {
    client.invoke('STFC_CONNECTION', { XXX: 'wrong param' }, function (err) {
        expect(err).toBeDefined();
        expect(err).toEqual(expect.objectContaining({
            code: 20,
            key: 'RFC_INVALID_PARAMETER',
            message: "field 'XXX' not found",
        }));
        done();
    });
});

it('error: invoke() rejects non-string rfm name', function (done) {
    client.invoke(23, {}, function (err) {
        expect(err).toBeDefined();
        expect(err).toEqual(expect.objectContaining({
            name: 'TypeError',
            message: 'First argument (remote function module name) must be an string',
        }));
        done();
    });
});

it('error: invoke() RFC_RAISE_ERROR', function (done) {
    client.invoke('RFC_RAISE_ERROR', { MESSAGETYPE: 'A' }, function (err) {
        expect(err).toBeDefined();
        expect(err).toEqual(expect.objectContaining({
            code: 4,
            key: 'Function not supported',
            abapMsgClass: 'SR',
            abapMsgType: 'A',
            abapMsgNumber: '006',
            message: 'Function not supported',
        }));
        done();
    });
});

it('error: non-existing field in input structure', function (done) {
    let importStruct = {
        XRFCCHAR1: 'A',
        RFCCHAR2: 'BC',
        RFCCHAR4: 'DEFG',
    };

    client.invoke('STFC_STRUCTURE', { IMPORTSTRUCT: importStruct }, function (err) {
        expect(err).toBeDefined();
        expect(err).toEqual(expect.objectContaining({
            name: 'RfcLibError',
            code: 20,
            key: 'RFC_INVALID_PARAMETER',
            message: "field 'XRFCCHAR1' not found",
        }));
        done();
    });
});

it('error: non-existing field in input table', function (done) {
    let importTable = [
        {
            XRFCCHAR1: 'A',
            RFCCHAR2: 'BC',
            RFCCHAR4: 'DEFG',
        },
    ];

    client.invoke('STFC_STRUCTURE', { RFCTABLE: importTable }, function (err) {
        expect(err).toBeDefined();
        expect(err).toEqual(expect.objectContaining({
            name: 'RfcLibError',
            code: 20,
            key: 'RFC_INVALID_PARAMETER',
            message: "field 'XRFCCHAR1' not found",
        }));
        done();
    });
});

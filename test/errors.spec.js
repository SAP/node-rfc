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

const rfcClient = require('./noderfc').Client;
const should = require('should');

const abapSystem = require('./abapSystem')();

describe('Errors', function() {
    let client = new rfcClient(abapSystem);

    beforeEach(function() {
        //if (!client.isAlive) return client.open();
        return client.open();
    });

    afterEach(function() {
        //if (client.isAlive) return client.close();
        return client.close();
    });

    after(function() {
        return client.close();
    });

    it('error: new client requires connection parameters', function(done) {
        try {
            new rfcClient();
        } catch (err) {
            should.exist(err);
            err.should.have.properties({
                name: 'TypeError',
                message: 'Connection parameters must be an object',
            });
            done();
        }
    });

    it('error: connect() requires minimum of connection parameters', function(done) {
        let wrongParams = Object.assign({}, abapSystem);
        delete wrongParams.ashost;

        let wrongClient = new rfcClient(wrongParams);
        wrongClient.connect(function(err) {
            should.exist(err);
            err.should.have.properties({
                message: 'Parameter ASHOST, GWHOST, MSHOST or SERVER_PORT is missing.',
                code: 20,
                key: 'RFC_INVALID_PARAMETER',
            });
            done();
        });
    });

    it('error: conect() rejects invalid credentials', function(done) {
        let wrongParams = Object.assign({}, abapSystem);
        wrongParams.user = 'WRONGUSER';

        let wrongClient = new rfcClient(wrongParams);
        wrongClient.connect(function(err) {
            should.exist(err);
            err.should.have.properties({
                message: 'Name or password is incorrect (repeat logon)',
                code: 2,
                key: 'RFC_LOGON_FAILURE',
            });
            done();
        });
    });

    it('error: connect() requires a callback function', function(done) {
        try {
            client.connect();
        } catch (err) {
            should.exist(err);
            err.should.have.properties({
                name: 'TypeError',
                message: 'First argument must be callback function',
            });
            done();
        }
    });

    it('error: invoke() requires at least three arguments', function(done) {
        try {
            client.invoke('rfc', {});
        } catch (err) {
            should.exist(err);
            err.should.have.properties({
                message: 'Please provide rfc module name, parameters and callback as arguments',
            });
        } finally {
            done();
        }
    });

    it('error: invoke() rejects non-existing parameter', function(done) {
        client.invoke('STFC_CONNECTION', { XXX: 'wrong param' }, function(err) {
            should.exist(err);
            err.should.be.an.Object();
            err.should.have.properties({
                code: 20,
                key: 'RFC_INVALID_PARAMETER',
                message: "field 'XXX' not found",
            });
            done();
        });
    });

    it('error: invoke() rejects non-string rfm name', function(done) {
        try {
            client.invoke(23, {}, 2);
        } catch (err) {
            should.exist(err);
            err.should.have.properties({
                name: 'TypeError',
                message: 'First argument (rfc module name) must be an string',
            });
        } finally {
            done();
        }
    });

    it('error: invoke() RFC_RAISE_ERROR', function(done) {
        client.invoke('RFC_RAISE_ERROR', { MESSAGETYPE: 'A' }, function(err) {
            should.exist(err);
            err.should.be.an.Object();
            err.should.have.properties({
                code: 4,
                key: 'Function not supported',
                abapMsgClass: 'SR',
                abapMsgType: 'A',
                abapMsgNumber: '006',
                message: 'Function not supported',
            });
            done();
        });
    });
});

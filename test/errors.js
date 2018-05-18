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

const rfc = require('../sapnwrfc');
const should = require('should');

const connParams = require('./connParams');

describe('Error handling', function() {
    let client;

    beforeEach(function(done) {
        client = new rfc.Client(connParams);
        client.connect(function(err) {
            if (err) return done(err);
            done();
        });
    });

    afterEach(function(done) {
        client.close();
        done();
    });

    it('Logon failure with wrong credentials', function(done) {
        let wrongParams = Object.assign({}, connParams);
        wrongParams.user = 'WRONGUSER';

        let wrongClient = new rfc.Client(wrongParams);
        wrongClient.connect(function(err) {
            should.exist(err);
            err.should.have.properties({
                message: 'Name or password is incorrect (repeat logon)',
                code: 2,
                key: 'RFC_LOGON_FAILURE',
            });
            done();
        });
        wrongClient.close();
    });

    it('Invoke with wrong parameter should return err RFC_INVALID_PARAMETER', function(done) {
        client.invoke('STFC_CONNECTION', { XXX: 'wrong param' }, function(err) {
            should.exist(err);
            err.should.be.an.Object;
            err.should.have.properties({
                code: 20,
                key: 'RFC_INVALID_PARAMETER',
                message: "field 'XXX' not found",
            });
            done();
        });
    });

    it('RFC_RAISE_ERROR should return error', function(done) {
        client.invoke('RFC_RAISE_ERROR', { MESSAGETYPE: 'A' }, function(err) {
            should.exist(err);
            err.should.be.an.Object;
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

    it('Connection parameter missing', function(done) {
        let wrongParams = {};
        for (let attr in connParams) {
            if (connParams.hasOwnProperty(attr)) {
                wrongParams[attr] = connParams[attr];
            }
        }
        delete wrongParams.ashost;

        let wrongClient = new rfc.Client(wrongParams);
        wrongClient.connect(function(err) {
            should.exist(err);
            err.should.have.properties({
                message: 'Parameter ASHOST, GWHOST, MSHOST or SERVER_PORT is missing.',
                code: 20,
                key: 'RFC_INVALID_PARAMETER',
            });
            done();
        });
        wrongClient.close();
    });

    it('[new] No connection parameters provided at all', function(done) {
        try {
            new rfc.Client();
        } catch (err) {
            should.exist(err);
            err.should.have.properties({
                message: 'Please provide connection parameters as argument',
            });
        } finally {
            done();
        }
    });

    it('[connect] First connection arg must be a callback function', function(done) {
        try {
            client.connect();
        } catch (err) {
            should.exist(err);
            err.should.have.properties({
                message: 'First argument must be callback function',
            });
        } finally {
            done();
        }
    });

    it('[invoke] At least three arguments must be provided', function(done) {
        try {
            client.invoke('rfc', {});
        } catch (err) {
            should.exist(err);
            err.should.have.properties({
                message: 'Please provide function module, parameters and callback as arguments',
            });
        } finally {
            done();
        }
    });

    it('[invoke] First argument (rfc function name) must be an string', function(done) {
        try {
            client.invoke(23, {}, 2);
        } catch (err) {
            should.exist(err);
            err.should.have.properties({
                message: 'First argument (rfc function name) must be an string',
            });
        } finally {
            done();
        }
    });
});

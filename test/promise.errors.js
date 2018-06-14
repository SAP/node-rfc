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

const rfcClient = require('../lib');
const should = require('should');

const connParams = require('./connParams');

describe('[promise] Error handling', function() {
    let client;

    beforeEach(function(done) {
        client = new rfcClient(connParams);
        client
            .open()
            .then(() => {
                done();
            })
            .catch(err => {
                return done(err);
            });
    });

    afterEach(function(done) {
        client.close();
        done();
    });

    it('No callback argument allowed in promise based call()', function(done) {
        client.open().then(() => {
            client
                .call('rfc', {}, () => {})
                .then(res => {
                    should.not.exist(res);
                    return done(res);
                })
                .catch(err => {
                    should.exist(err);
                    err.should.be.an.Object;
                    err.should.have.properties({
                        name: 'TypeError',
                        message: 'No callback argument allowed in promise based call()',
                    });
                    done();
                });
        });
    });

    it('Logon failure with wrong credentials', function(done) {
        let wrongParams = Object.assign({}, connParams);
        wrongParams.user = 'WRONGUSER';

        let wrongClient = new rfcClient(wrongParams);
        wrongClient
            .open()
            .then(res => {
                should.not.exist(res);
                return done(res);
            })
            .catch(err => {
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
        client
            .call('STFC_CONNECTION', { XXX: 'wrong param' })
            .then(res => {
                should.not.exist(res);
                return done(res);
            })
            .catch(err => {
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
        client
            .call('RFC_RAISE_ERROR', { MESSAGETYPE: 'A' })
            .then(res => {
                should.not.exist(res);
                return done(res);
            })
            .catch(err => {
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
        let wrongParams = Object.assign({}, connParams);
        delete wrongParams.ashost;

        let wrongClient = new rfcClient(wrongParams);
        wrongClient
            .open()
            .then(res => {
                should.not.exist(res);
                return done(res);
            })
            .catch(err => {
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

    it('No connection parameters provided at all', function(done) {
        try {
            new rfcClient();
        } catch (err) {
            should.exist(err);
            err.should.have.properties({
                message: 'Please provide connection parameters as argument',
            });
        } finally {
            done();
        }
    });

    it('At least three arguments must be provided', function(done) {
        client.call('rfc').catch(err => {
            should.exist(err);
            err.should.have.properties({
                name: 'TypeError',
                message: 'Second argument (rfc module parameters) must be an object',
            });
            done();
        });
    });

    it('First argument (rfc module name) must be an string', function(done) {
        client.call(23, {}, 2).catch(err => {
            should.exist(err);
            err.should.have.properties({
                name: 'TypeError',
                message: 'First argument (rfc module name) must be an string',
            });
            done();
        });
    });

    it('Second argument (rfc module parameters) must be an object', function(done) {
        client.call('rfc', 41, 2).catch(err => {
            should.exist(err);
            err.should.have.properties({
                name: 'TypeError',
                message: 'Second argument (rfc module parameters) must be an object',
            });
            done();
        });
    });
});

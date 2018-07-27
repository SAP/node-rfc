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

describe('Errors promise', function() {
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

    it('error: call() promise rejects invalid credentials', function(done) {
        let wrongParams = Object.assign({}, abapSystem);
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
    });

    it('error: call() promise rejects non-existing parameter', function(done) {
        client
            .call('STFC_CONNECTION', { XXX: 'wrong param' })
            .then(res => {
                should.not.exist(res);
                return done(res);
            })
            .catch(err => {
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

    it('error: promise call() RFC_RAISE_ERROR', function(done) {
        client
            .call('RFC_RAISE_ERROR', { MESSAGETYPE: 'A' })
            .then(res => {
                should.not.exist(res);
                return done(res);
            })
            .catch(err => {
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

    it('error: open() promise requires minimum of connection parameters', function(done) {
        let wrongParams = Object.assign({}, abapSystem);
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
    });

    it('error: promise call() requires at least two arguments', function(done) {
        client.call('rfc').catch(err => {
            should.exist(err);
            err.should.have.properties({
                name: 'TypeError',
                message: 'Please provide remote function module name and parameters as arguments',
            });
            done();
        });
    });

    it('error: promise call() rejects non-string rfm name', function(done) {
        client.call(23, {}, 2).catch(err => {
            should.exist(err);
            err.should.have.properties({
                name: 'TypeError',
                message: 'First argument (remote function module name) must be an string',
            });
            done();
        });
    });

    it('error: promise call() rejects non-object second argument (remote function module parameters)', function(done) {
        client.call('rfc', 41, 2).catch(err => {
            should.exist(err);
            err.should.have.properties({
                name: 'TypeError',
                message: 'Second argument (remote function module parameters) must be an object',
            });
            done();
        });
    });
});

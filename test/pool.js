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

const Pool = require('../lib').Pool;
const should = require('should');
const abapSystem = require('./abapSystem')();

describe('Pool', function() {
    let pool;
    let ID;

    before(function(done) {
        pool = new Pool(abapSystem);
        done();
    });

    after(function() {
        pool.releaseAll();
    });

    it('acquire id', function(done) {
        pool
            .acquire()
            .then(client => {
                client.id.should.be.number;
                ID = client.id;
                //client.id.should.equal(1);
                client.isAlive.should.be.true;
                pool.status.ready.should.equal(1);
                pool.releaseAll();
                pool.status.ready.should.equal(0);
                done();
            })
            .catch(err => {
                should.not.exist(err);
                done(err);
            });
    });

    it('acquire id=1', function(done) {
        pool
            .acquire()
            .then(client => {
                client.id.should.be.number;
                client.id.should.equal(ID + 2);
                ID = client.id;
                client.isAlive.should.be.true;
                pool.status.ready.should.equal(1);
                pool.releaseAll();
                pool.status.ready.should.equal(0);
                done();
            })
            .catch(err => {
                should.not.exist(err);
                done(err);
            });
    });

    it('acquire id=3', function(done) {
        pool
            .acquire()
            .then(client => {
                client.id.should.be.number;
                client.id.should.equal(ID + 2);
                ID = client.id;
                client.isAlive.should.be.true;
                pool.status.ready.should.equal(1);
                done();
            })
            .catch(err => {
                should.not.exist(err);
                done(err);
            });
    });

    it('acquire 10', function(done) {
        let id = new Set();

        for (let i = ID + 1; i < ID + 11; i++) {
            id.add(i);
            pool.acquire().then(client => {
                if (client.id > ID) ID = client.id;
                id.delete(client.id);
                if (id.size === 0) {
                    done();
                }
            });
        }
    });

    it('unique client id across pools', function(done) {
        pool
            .acquire()
            .then(client => {
                client.id.should.be.number;
                client.id.should.equal(ID + 1);
                ID = client.id;
                client.isAlive.should.be.true;

                let pool2 = new Pool(abapSystem);

                pool2.acquire().then(c2 => {
                    c2.id.should.equal(ID + 2);
                    c2.isAlive.should.be.true;
                    pool2.releaseAll();
                    done();
                });
            })
            .catch(err => {
                should.not.exist(err);
                done(err);
            });
    });

    it('internal pool error', function(done) {
        let xpool = new Pool(abapSystem, { min: 0, max: 10 });
        xpool.acquire().catch(err => {
            err.name.should.equal('TypeError');
            err.message.should.equal('Internal pool error, size = 0');
            done();
        });
    });
});

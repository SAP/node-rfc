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
const Pool = setup.rfcPool;
const abapSystem = setup.abapSystem;

let pool;
let ID;

beforeAll(function (done) {
    pool = new Pool(abapSystem);
    done();
});

afterAll(function () {
    delete setup.client;
    delete setup.rfcClient;
    delete setup.rfcPool;
    return pool.releaseAll();
});

it("pool: acquire id", function (done) {
    pool.acquire()
        .then(client => {
            expect(client.id).toBeGreaterThan(0);
            ID = client.id;
            //client.id.should.equal(1);
            expect(client.isAlive).toBeTruthy();
            expect(pool.status.ready).toBe(1);
            pool.releaseAll().then(() => {
                expect(pool.status.ready).toBe(0);
                done();
            });
        })
        .catch(err => {
            if (err) return done(err);
        });
});

it("pool: acquire id=1", function (done) {
    pool.acquire()
        .then(client => {
            expect(ID).not.toBeNaN();
            expect(client.id).toBe(ID + 2);
            ID = client.id;
            expect(client.isAlive).toBeTruthy();
            expect(pool.status.ready).toBe(1);
            pool.releaseAll().then(() => {
                expect(pool.status.ready).toBe(0);
                done();
            });;
        })
        .catch(err => {
            if (err) return done(err);
        });
});

it("pool: acquire id=3", function (done) {
    pool.acquire()
        .then(client => {
            expect(ID).not.toBeNaN();
            expect(client.id).toBe(ID + 2);
            ID = client.id;
            expect(client.isAlive).toBeTruthy();
            expect(pool.status.ready).toBe(1);
            done();
        })
        .catch(err => {
            if (err) return done(err);
        });
});

it("pool: acquire 10", function (done) {
    let id = new Set();
    for (let i = ID + 1; i < ID + 11; i++) {
        id.add(i);
        pool.acquire().then(client => {
            expect(ID).not.toBeNaN();
            if (client.id > ID) ID = client.id;
            id.delete(client.id);
            if (id.size === 0) {
                done();
            }
        });
    }
});

it("pool: unique client id across pools", function (done) {
    pool.acquire()
        .then(client => {
            expect(ID).not.toBeNaN();
            expect(client.id).toBe(ID + 1);
            ID = client.id;
            expect(client.isAlive).toBeTruthy();

            let pool2 = new Pool(abapSystem);

            pool2.acquire().then(c2 => {
                expect(c2.id).toBe(ID + 2);
                expect(c2.isAlive).toBeTruthy();
                pool2.releaseAll();
                done();
            });
        })
        .catch(err => {
            if (err) return done(err);
        });
});

it("error: pool internal error", function (done) {
    let xpool = new Pool(abapSystem, {
        min: 0,
        max: 10
    });
    xpool.acquire().catch(err => {
        expect(err).toBeDefined();
        expect(err).toEqual(
            expect.objectContaining({
                name: "TypeError",
                message: "Internal pool error, size = 0"
            })
        );
        done();
    });
});

it("pool: default options", function (done) {
    pool.acquire()
        .then(client => {
            expect(client.id).toBeGreaterThan(0);
            // expect(client.id).toEqual(2);
            expect(client.isAlive).toBeTruthy();
            expect(pool.status.ready).toBe(1);
            expect(client.options.bcd).toEqual("string");
            pool.releaseAll().then(() => {
                expect(pool.status.ready).toBe(0);
                done();
            });;
        })
        .catch(err => {
            if (err) return done(err);
        });
});

xit("pool: bcd number", function (done) {
    const poolOptions = new Pool(abapSystem, undefined, {
        bcd: "number"
    });
    poolOptions.acquire()
        .then(client => {
            expect(client.id).toBeGreaterThan(0);
            // expect(client.id).toEqual(2);
            expect(client.isAlive).toBeTruthy();
            expect(poolOptions.status.ready).toBe(1);
            expect(client.options.bcd).toEqual("number");
            poolOptions.releaseAll().then(() => {
                expect(poolOptions.status.ready).toBe(0);
                done();
            });
        })
        .catch(err => {
            if (err) return done(err);
        });
});

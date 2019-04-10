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
    client.reopen(err => {
        done(err);
    });
});

afterEach(function (done) {
    client.close(() => {
        done();
    });
});

afterAll(function (done) {
    delete setup.client;
    delete setup.rfcClient;
    delete setup.rfcPool;
    done();
});

it('performance: invoke() BAPI_USER_GET_DETAIL', function (done) {
    //this.timeout(15000);
    client.invoke('BAPI_USER_GET_DETAIL', { USERNAME: 'DEMO' }, function (err, res) {
        if (err) return done(err);
        expect(res).toBeDefined();
        expect(Object.keys(res).sort()).toEqual(expect.arrayContaining([
            'ADDRESS',
            'ACTIVITYGROUPS',
            'DEFAULTS',
            'GROUPS',
            'ISLOCKED',
            'LOGONDATA',
            'PARAMETER',
            'PROFILES',
            'RETURN'
        ]));
        done();
    });
});

it('performance: invoke() STFC_PERFORMANCE', function (done) {
    let COUNT = 10000;
    client.invoke(
        'STFC_PERFORMANCE',
        { CHECKTAB: 'X', LGET0332: COUNT.toString(), LGET1000: COUNT.toString() },

        function (err, res) {
            if (err) return done(err);
            expect(res.ETAB0332.length).toBe(COUNT);
            expect(res.ETAB1000.length).toBe(COUNT);
            done();
        }
    );
});

/*
it('performance: invoke() SWNC_READ_SNAPSHOT', function(done) {
    function toABAPdate(date) {
        let mm = date.getMonth() + 1;
        let dd = date.getDate();
        return [date.getFullYear(), mm > 9 ? mm : '0' + mm, dd > 9 ? dd : '0' + dd].join('');
    }
    this.timeout(120000);
    let endDate = new Date();
    let startDate = new Date(endDate);
    startDate.setDate(startDate.getDate() - 1);
    client.invoke(
        'SWNC_READ_SNAPSHOT',
        {
            READ_TIMEZONE: 'UTC',
            READ_START_DATE: toABAPdate(startDate),
            READ_START_TIME: '000000',
            READ_END_DATE: toABAPdate(endDate),
            READ_END_TIME: '235959',
            TIME_RESOLUTION: 60,
        },

        function(err, res) {
            if (err) return done(err);
            expect(res.HITLIST_DATABASE.length).toBe(0);
            expect(res.HITLIST_RESPTIME.length).toBe(0);
            done();
        }
    );
});
*/

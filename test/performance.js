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

const rfcClient = require('../lib').Client;
const should = require('should');

const abapSystem = require('./abapSystem')('MME');

describe('Performance', function() {
    let client = new rfcClient(abapSystem);

    before(function(done) {
        client.connect(function(err) {
            if (err) return done(err);
            done();
        });
    });

    after(function() {
        client.close();
    });

    it('Invoke BAPI_USER_GET_DETAIL', function(done) {
        this.timeout(15000);
        client.invoke('BAPI_USER_GET_DETAIL', { USERNAME: 'DEMO' }, function(err, res) {
            should.not.exist(err);
            res.should.be.an.Object();
            res.should.have.properties(
                'ADDRESS',
                'ACTIVITYGROUPS',
                'DEFAULTS',
                'GROUPS',
                'ISLOCKED',
                'LOGONDATA',
                'PARAMETER',
                'PROFILES',
                'RETURN'
            );
            done();
        });
    });

    it('Invoke STFC_PERFORMANCE', function(done) {
        let COUNT = 10000;
        client.invoke(
            'STFC_PERFORMANCE',
            { CHECKTAB: 'X', LGET0332: COUNT.toString(), LGET1000: COUNT.toString() },

            function(err, res) {
                should.not.exist(err);
                res.ETAB0332.length.should.equal(COUNT);
                res.ETAB1000.length.should.equal(COUNT);
                done();
            }
        );
    });

    /*
    it('Invoke SWNC_READ_SNAPSHOT', function(done) {
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
                should.not.exist(err);
                res.HITLIST_DATABASE.length.should.not.equal(0);
                res.HITLIST_RESPTIME.length.should.not.equal(0);
                done();
            }
        );
    });
    */
});

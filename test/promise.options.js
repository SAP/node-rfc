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

describe('[promise] Options', function() {
    let client;
    beforeEach(function(done) {
        client = new rfcClient(abapSystem);
        client
            .open()
            .then(() => {
                done();
            })
            .catch(err => {
                return done(err);
            });
    });

    afterEach(function() {
        client.close();
    });

    it('Skip parameters, no error if some params skipped', function() {
        this.timeout(5000);
        let notRequested = [
            'ET_COMPONENTS',
            'ET_HDR_HIERARCHY',
            'ET_MPACKAGES',
            'ET_OPERATIONS',
            'ET_OPR_HIERARCHY',
            'ET_PRTS',
            'ET_RELATIONS',
        ];
        return client
            .call(
                'EAM_TASKLIST_GET_DETAIL',
                {
                    IV_PLNTY: 'A',
                    IV_PLNNR: '00100000',
                },
                { notRequested: notRequested }
            )
            .then(res => {
                res.should.be.an.Object;
                res.should.have.properties('ET_RETURN');
                res.ET_RETURN.should.have.length(0);
            });
    });

    it('Skip parameters, error if all requested', function() {
        return client
            .call('EAM_TASKLIST_GET_DETAIL', {
                IV_PLNTY: 'A',
                IV_PLNNR: '00100000',
            })
            .then(res => {
                // ET_RETURN error if all params requested
                res.should.be.an.Object;
                res.should.have.properties('ET_RETURN');
                res.ET_RETURN.should.have.length(1);
            });
    });
});

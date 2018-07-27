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

describe('Options', function() {
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

    it('options: pass when some parameters skipped', function(done) {
        this.timeout(10000);
        let notRequested = [
            'ET_COMPONENTS',
            'ET_HDR_HIERARCHY',
            'ET_MPACKAGES',
            'ET_OPERATIONS',
            'ET_OPR_HIERARCHY',
            'ET_PRTS',
            'ET_RELATIONS',
        ];
        client.connect(function(err) {
            should.not.exist(err);
            client.invoke(
                'EAM_TASKLIST_GET_DETAIL',
                {
                    IV_PLNTY: 'A',
                    IV_PLNNR: '00100000',
                },
                function(err, res) {
                    should.not.exist(err);
                    // ET_RETURN clean if certain params not requested
                    res.should.be.an.Object();
                    res.should.have.properties('ET_RETURN');
                    res.ET_RETURN.should.have.length(0);
                    done();
                },
                { notRequested: notRequested }
            );
        });
    });

    it('options: error when all requested', function(done) {
        client.connect(function(err) {
            should.not.exist(err);
            client.invoke(
                'EAM_TASKLIST_GET_DETAIL',
                {
                    IV_PLNTY: 'A',
                    IV_PLNNR: '00100000',
                },
                function(err, res) {
                    // ET_RETURN error if all params requested
                    should.not.exist(err);
                    res.should.be.an.Object();
                    res.should.have.properties('ET_RETURN');
                    res.ET_RETURN.should.have.length(1);
                    done();
                }
            );
        });
    });
});

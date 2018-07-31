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
const abapSystem = require('./abapSystem')();

const should = require('should');

const CONNECTIONS = 50;

describe('Concurrency await (node > 7.6.0)', function() {
	this.timeout(15000);

	let client = new rfcClient(abapSystem);

	beforeEach(function(done) {
		client.reopen(() => {
			done();
		});
	});
	
	afterEach(function(done) {
		client.close(() => {
			done();
		});
	});
	const REQUTEXT = 'Hellö SÄP!';

	it(`await: ${CONNECTIONS} sequential calls using single connection`, function(done) {
		(async () => {
			for (let i = 0; i < CONNECTIONS; i++) {
				try {
					let res = await client.call('STFC_CONNECTION', { REQUTEXT: REQUTEXT + i });

					res.should.be.an.Object();
					res.should.have.property('ECHOTEXT');
					res.ECHOTEXT.should.startWith(REQUTEXT + i);
				} catch (ex) {
					return done(ex);
				}
			}
			done();
		})();
	});

	it(`await: ${CONNECTIONS} recursive calls using single connection`, function(done) {
		async function run(depth) {
			if (depth == CONNECTIONS) {
				return done();
			}
			try {
				let res = await client.call('STFC_CONNECTION', { REQUTEXT: REQUTEXT + depth });
				res.should.be.an.Object();
				res.should.have.property('ECHOTEXT');
				res.ECHOTEXT.should.startWith(REQUTEXT + depth);
				run(depth + 1);
			} catch (ex) {
				return done(ex);
			}
		}
		run(0);
	});

	it(`await: ${CONNECTIONS} parallel connections`, function(done) {
		let CLIENTS = [];
		for (let i = 0; i < CONNECTIONS; i++) {
			CLIENTS.push(new rfcClient(abapSystem));
		}
		(async () => {
			for (let client of CLIENTS) {
				try {
					await client.open();
					let res = await client.call('STFC_CONNECTION', { REQUTEXT: REQUTEXT + client.id });
					res.should.be.an.Object();
					res.should.have.property('ECHOTEXT');
					res.ECHOTEXT.should.startWith(REQUTEXT + client.id);
					client.close();
				} catch (ex) {
					return done(ex);
				}
			}
			done();
		})();
	});
});

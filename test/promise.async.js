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

const abapSystem = require('./abapSystem')();

const CONNECTIONS = 50;

describe('[async/await] parallel and sequential, node > 7.6.0', function() {
	this.timeout(15000);

	let client = new rfcClient(abapSystem);
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

	const REQUTEXT = 'Hellö SÄP!';

	it(`async/await ${CONNECTIONS} calls with single connection`, function(done) {
		async function run() {
			for (let i = CONNECTIONS; i > 0; i--) {
				let res = await client.call('STFC_CONNECTION', { REQUTEXT: REQUTEXT + i });
				res.should.be.an.Object();
				res.should.have.property('ECHOTEXT');
				res.ECHOTEXT.should.startWith(REQUTEXT + i);
				if (i === 1) done();
			}
		}

		run();
	});

	it(`async/await ${CONNECTIONS} recursive calls with single connection`, function(done) {
		async function run(depth) {
			if (depth == CONNECTIONS) {
				done();
				return;
			}
			let res = await client.call('STFC_CONNECTION', { REQUTEXT: REQUTEXT + depth });
			res.should.be.an.Object();
			res.should.have.property('ECHOTEXT');
			res.ECHOTEXT.should.startWith(REQUTEXT + depth);
			run(depth + 1);
		}
		run(0);
	});

	it(`async/await ${CONNECTIONS} parallel connections`, function(done) {
		let count = CONNECTIONS;
		let CLIENTS = [];
		for (let i = 0; i < count; i++) {
			CLIENTS.push(new rfcClient(abapSystem));
		}
		for (let client of CLIENTS) {
			(async () => {
				await client.open();
				let res = await client.call('STFC_CONNECTION', { REQUTEXT: REQUTEXT + client.id });
				res.should.be.an.Object();
				res.should.have.property('ECHOTEXT');
				res.ECHOTEXT.should.startWith(REQUTEXT + client.id);
				client.close();
				if (--count === 1) done();
			})();
		}
	});
});

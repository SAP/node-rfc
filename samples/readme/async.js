'use strict';

// const rfcClient = require('node-rfc').Client;
const rfcClient = require('../../lib').Client;

const abapSystem = require('./abapSystem');

const client = new rfcClient(abapSystem);

async function getData() {
	await client.open(); // remove await to test the catch block below

	let result = await client.call('STFC_CONNECTION', { REQUTEXT: 'H€llö SAP!' });

	result.ECHOTEXT += '#';

	result = await client.call('STFC_CONNECTION', { REQUTEXT: result.ECHOTEXT });

	return result;
}

(async () => {
	let result;
	try {
		result = await getData();
		console.log(result.ECHOTEXT); // 'H€llö SAP!#'
	} catch (ex) {
		console.log(ex);
	}
})();

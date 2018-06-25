//import { Client } from 'node-rfc';
import { Client } from '../../lib';

import abapSystem from './abapSystem';

const client = new Client(abapSystem);

async function getSomeAsyncData() {
	await client.open();

	let result = await client.call('STFC_CONNECTION', { REQUTEXT: 'H€llö SAP!' });

	result.ECHOTEXT += '#';

	await client.call('STFC_CONNECTION', { REQUTEXT: result.ECHOTEXT });

	return result.ECHOTEXT;
}

(async function() {
	try {
		let result = await getSomeAsyncData();

		console.log(result); // should be 'H€llö SAP!#'
	} catch (ex) {
		console.error(ex);
	}
})();

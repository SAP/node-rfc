import { Client } from 'node-rfc';

import { DSP } from './abapSystem';

const client: Client = new Client(DSP);

//(async function() {
	try {
		await client.open();

		let result = await client.call('STFC_CONNECTION', { REQUTEXT: 'H€llö SAP!' });

		result.ECHOTEXT += '#';

		await client.call('STFC_CONNECTION', { REQUTEXT: result.ECHOTEXT });

		console.log(result.ECHOTEXT); // 'H€llö SAP!#'
	} catch (ex) {
		console.error(ex);
	}
//})();

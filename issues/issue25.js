let rfc = require('../build/linux_x64/rfc.node');
const connParams = require('../test/connParams');

client = new rfc.Client(connParams);

console.log('client:', client.getVersion());

console.log('RFC Client is connecting...');
client.connect(function(err) {
	if (err) return console.error('(rfc.Client) Could not connect to server', err);

	setInterval(function() {
		console.log(client.ping()); // <---- THIS is what causes the segmentation fault
		//console.log(client.connectionInfo());
	}, 100);

	setInterval(function() {
		console.log('Calling RFC function...');
		client.invoke('BAPI_USER_GET_DETAIL', { USERNAME: 'DEMO' }, (rfcErr, rfcRes) => {
			//client.invoke('RFC_SYSTEM_INFO', {}, (rfcErr, rfcRes) => {
			if (rfcErr) return console.error('Error invoking function:', rfcErr);
			console.log('RFC call OK');
		});
	}, 2000);
});

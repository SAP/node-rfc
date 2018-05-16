let rfc = require('../build/linux_x64/rfc.node');
const connParams = require('../test/connParams');
const branchName = require('current-git-branch')();

let fs = require('fs');

const COUNT = 200;
const SIZE = '10000';

let fileName = `exp/${branchName}-${COUNT}-x-${SIZE}-${process.version}.txt`;
fs.writeFileSync(fileName, `iteration\t\trss\t\theapTotal\t\theapUsed\t\texternal\n`);

function run(i) {
	console.log(i);

	if (i == COUNT) {
		console.log('Bye!');
		return;
	}

	let client = new rfc.Client(connParams);

	client.connect(function(err) {
		if (err) {
			// check for login/connection errors
			return console.error('could not connect to server', err);
		}
		/*
        client.invoke(
            'SWNC_READ_SNAPSHOT',
            {
                READ_TIMEZONE: 'UTC',
                READ_START_DATE: '20180411',
                READ_START_TIME: '080000',
                READ_END_DATE: '20180418',
                READ_END_TIME: '000000',
                TIME_RESOLUTION: 60,
            },
            function(err, res) {
        */

		client.invoke('STFC_PERFORMANCE', { CHECKTAB: 'X', LGET0332: SIZE, LGET1000: SIZE }, function(err, res) {
			if (err) {
				console.log(err);
				fs.appendFileSync(fileName, err);
			} else {
				let memUsage = process.memoryUsage();
				let line = `${i}\t\t${(memUsage.rss / 1024 / 1024).toFixed(3)}MB\t\t${(
					memUsage.heapTotal /
					1024 /
					1024
				).toFixed(3)}MB\t\t${(memUsage.heapUsed / 1024 / 1024).toFixed(3)}MB\t\t${(
					memUsage.external /
					1024 /
					1024
				).toFixed(3)}MB\t\t${err ? err : ''}\n`;
				client.close();
				delete client;
				fs.appendFileSync(fileName, line);
				run(++i);
			}
		});
	});
}

run(1);

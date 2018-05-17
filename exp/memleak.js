const rfcClient = require('../sapnwrfc').Client;
const connParams = require('../test/connParams');
const branchName = require('current-git-branch')();

let fs = require('fs');

const COUNT = 100;
const SIZE = '10000';

let fileName = `test/${branchName}-${COUNT}-x-${SIZE}-${process.version}.txt`;
fs.writeFileSync(fileName, `iteration\t\trss\t\theapTotal\t\theapUsed\t\texternal\n`);

let client = new rfcClient(connParams);
console.log(rfcClient.getVersion());

let events = require('events');
let eventEmitter = new events.EventEmitter();
let i = 0;

let run = () => {
    console.log(i);

    if (i == COUNT) {
        console.log(rfcClient.getVersion());
        return;
    }

    client.connect(function(err) {
        if (err) {
            // check for login/connection errors
            console.error('could not connect to server', err);
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
        //let importStruct = { RFCCHAR4: 'A' };
        //let importTable = [importStruct];
        //client.invoke('STFC_STRUCTURE', { IMPORTSTRUCT: importStruct, RFCTABLE: importTable }, function(err, res) {
        client.invoke('STFC_PERFORMANCE', { CHECKTAB: 'X', LGET0332: SIZE, LGET1000: SIZE }, function(err, res) {
            if (err) {
                console.log(err);
                fs.appendFileSync(fileName, err);
            } else {
                client.close();
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

                fs.appendFileSync(fileName, line);
                i++;
                eventEmitter.emit('next');
            }
        });
    });
};

eventEmitter.on('next', run);
eventEmitter.emit('next');

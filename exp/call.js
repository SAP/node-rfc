const rfcClient = require('../lib');
const connParams = require('../test/connParams');

let client = new rfcClient(connParams);

client
    .open()

    .then(() => {
        let importStruct = {
            RFCINT1: 1,
            RFCINT2: 2.3,
            RFCINT4: 3.7,
        };
        let importTable = [importStruct];
        client
            .call('STFC_STRUCTURE', { IMPORTSTRUCT: importStruct, RFCTABLE: importTable })
            .then(res => {
                console.log('Res:', res);
            })
            .catch(err => {
                console.log('Err:', err);
            });
    })

    .catch(err => {
        console.error('Error open:', err);
    });

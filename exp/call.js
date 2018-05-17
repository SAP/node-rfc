const rfc = require('../sapnwrfc');

const connParams = require('../test/connParams');

const rfcClient = rfc.Client;
console.log(rfcClient.getVersion());

//connParams.user = 'XXX';

const client = new rfcClient(connParams);

client.connect(function(err) {
    if (err) {
        console.error(err);
    } else {
        let importStruct = {
            RFCCHAR4: 'A',
        };
        let importTable = [importStruct];
        client.invoke('STFC_STRUCTURE', { IMPORTSTRUCT: importStruct, RFCTABLE: importTable }, function(err, res) {
            if (err) {
                console.error(err.message);
            } else {
                console.log(res);
            }
        });
    }
});

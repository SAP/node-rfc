const assert = require('assert');
const rfc = require('../sapnwrfc');
const connParams = require('../test/connParams');

const rfcClient = rfc.Client;
client = new rfcClient(connParams);

console.log(rfcClient.getVersion());
let importStruct = {
    RFCINT1: '1',
};
let importTable = [importStruct];
client.connect(() => {
    client.invoke('STFC_STRUCTURE', { IMPORTSTRUCT: importStruct, RFCTABLE: importTable }, function(err) {
        console.log(
            err instanceof TypeError,
            err.message === 'Integer number expected when filling field RFCINT1 of type 10'
        );
        /*
    should.exist(err);
    err.should.be.an.Object;
    err.should.have.properties({
        message: 'Number expected when filling field RFCINT1 of type 10',
    });
    done();
    */
    });
});

'use strict';

// const rfcClient = require('node-rfc').Client;
const rfcClient = require('../../lib').Client;

const abapSystem = require('./abapSystem');

// create new client
const client = new rfcClient(abapSystem);

// echo the node-rfc client and SAP NW RFC SDK version
console.log('RFC client lib version: ', client.version);

// open connection
client.connect(function(err) {
    if (err) {
        // check for login/connection errors
        return console.error('could not connect to server', err);
    }

    //
    // invoke remote enabled ABAP function module
    //
    client.invoke('STFC_CONNECTION', { REQUTEXT: 'H€llö SAP!' }, function(err, res) {
        if (err) {
            // check for errors (e.g. wrong parameters)
            return console.error('Error invoking STFC_CONNECTION:', err);
        }

        // result should be something like:
        // { ECHOTEXT: 'Hello SAP!',
        //   RESPTEXT: 'SAP R/3 Rel. 702   Sysid: E1Q      Date: 20140613   Time: 142530   Logon_Data: 001/DEMO/E',
        //   REQUTEXT: 'Hello SAP!' }
        console.log('STFC_CONNECTION call result:', res);
    });

    //
    // invoke more complex ABAP function module
    //

    // ABAP structure
    const importStruct = {
        RFCFLOAT: 1.23456789,
        RFCCHAR4: 'DEFG',
        RFCINT4: 345,
    };

    // ABAP table
    let importTable = [importStruct];

    client.invoke('STFC_STRUCTURE', { IMPORTSTRUCT: importStruct, RFCTABLE: importTable }, function(err, res) {
        if (err) {
            return console.error('Error invoking STFC_STRUCTURE:', err);
        }
        console.log('STFC_STRUCTURE call result:', res);
    });

    //
    // invoke possibly longer running ABAP function module, returning more data
    //
    let COUNT = 50000;
    client.invoke(
        'STFC_PERFORMANCE',
        { CHECKTAB: 'X', LGET0332: COUNT.toString(), LGET1000: COUNT.toString() },

        function(err, res) {
            if (err) {
                return err;
            }
            console.log('ETAB0332 records count:', res.ETAB0332.length);
            console.log('ETAB1000 records count:', res.ETAB1000.length);
        }
    );
});

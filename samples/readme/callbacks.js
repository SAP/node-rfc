'use strict';

// const rfcClient = require('node-rfc').Client;
const rfcClient = require('../../lib').Client;

// RFC connection parameters
const abapSystem = require('./abapSystem');

// create new client
const client = new rfcClient(abapSystem);

// echo SAP NW RFC SDK and nodejs/RFC binding version
console.log('Client version: ', client.version);

// open connection
client.connect(function(err) {
    if (err) {
        // check for login/connection errors
        return console.error('could not connect to server', err);
    }

    //
    // invoke ABAP function module with string parameter
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
    // invoke ABAP function module with structure and table parameters
    //

    // ABAP structure
    const structure = {
        RFCINT4: 345,
        RFCFLOAT: 1.23456789,
        // or RFCFLOAT: require('decimal.js')('1.23456789'),
        RFCCHAR4: 'ABCD',
        RFCDATE: '20170927',
    };

    // ABAP table
    let table = [structure];

    client.invoke('STFC_STRUCTURE', { IMPORTSTRUCT: structure, RFCTABLE: table }, function(err, res) {
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

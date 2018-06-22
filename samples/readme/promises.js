'use strict';

//const rfcClient = require('node-rfc').Client;
const rfcClient = require('../../lib').Client;

const abapSystem = require('./abapSystem');

const client = new rfcClient(abapSystem);

client
    .open()
    .then(() => {
        client
            .call('STFC_CONNECTION', { REQUTEXT: 'H€llö SAP!' })
            .then(res => {
                // process results, reuse for the next RFC call
                res.ECHOTEXT += '#';
                return new Promise(resolve => resolve(res));
            })
            .then(res => {
                client
                    .call('STFC_CONNECTION', { REQUTEXT: res.ECHOTEXT })
                    .then(res => {
                        console.log('STFC_CONNECTION call result:', res.ECHOTEXT);
                    })
                    .catch(err => {
                        console.error('Error invoking STFC_CONNECTION:', err);
                    });
            })
            .catch(err => {
                console.error('Error invoking STFC_CONNECTION:', err);
            });
    })
    .catch(err => {
        console.error('could not connect to server', err);
    });

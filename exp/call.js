const rfc = require('../sapnwrfc');

const connParams = require('../test/connParams');

const rfcClient = rfc.Client;

let client = rfc.Client.new(connParams);

console.log(client.connectionInfo());
return;

client
    .open(() => {})
    .then(() => {
        client.call(23, {}).catch(err => {
            console.log(err.name);
            console.log(err.message);
        });
    })
    .then(() => {
        client.call('rfc', 21).catch(err => {
            console.log(err.name);
            console.log(err.message);
        });
    })
    .catch(err => {
        console.error(err.name, err.message);
    })
    .finally(() => {
        console.log('finally!');
    });

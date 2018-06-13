const rfcClient = require('../sapnwrfc');

const connParams = require('../test/connParams');

let c1 = new rfcClient(connParams);
console.log(c1.id);
console.log(c1.version);
c1.call('rfc').catch(err => {
    console.dir(err);
});
/*
c1.invoke('STFC_CONNECTION', { REQUTEXT: 'Hello SAP!' }, function(err, res) {
    if (err) console.error(err);
    else console.dir(res);
});

let c2 = new rfcClient(connParams);
console.log(c2.id);
/*
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
*/

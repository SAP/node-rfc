const rfc = require('../sapnwrfc');

const connParams = require('../test/connParams');

let c1 = rfc.Client.new(connParams);
console.log(c1.id);
let c2 = rfc.Client.new(connParams);
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

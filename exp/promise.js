const rfc = require('../sapnwrfc');
const rfcClient = rfc.Client;

const connParams = require('../test/connParams');

const client = new rfcClient(connParams);

//console.log(rfcClient.getVersion());
//console.log(rfc);

client.connect((err, res) => {
    if (err) console.error('Error', err);
    else console.log('Connected', res);
});

function connect() {
    return new Promise(function(resolved, rejected) {
        client.connect((err, res) => {
            if (err) rejected(err);
            else resolved(res);
        });
    });
}

connect(connParams)
    .then(res => {
        console.log(res);
    })
    .catch(err => {
        console.error(err);
    });

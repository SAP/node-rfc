const rfcClient = require('../lib').Client;
//const connParams = require('../test/connParams');

let connParams = {
    user: 'user05',
    passwd: 'welcome123',
    ashost: '103.224.157.245',
    sysnr: '15',
    client: '800',
    lang: 'EN',
};

let client = new rfcClient(connParams);

client
    .open()

    .then(() => {
        console.log(client.connectionInfo);
        setInterval(function() {
            console.log('ping');
            client.ping(); // <---- THIS is what causes the segmentation fault
        }, 100);

        setInterval(function() {
            console.log('Calling RFC function...');
            client
                .call('BAPI_USER_GET_DETAIL', { USERNAME: 'DEMO' })
                .then(rfcRes => {
                    console.log('BAPI executed');
                })
                .catch(rfcErr => {
                    return console.error('Error invoking function:', rfcErr);
                    console.log('Result from RFC function:', rfcRes);
                });
        }, 2000);
    })

    .catch(err => {
        console.error('Error open:', err);
    });

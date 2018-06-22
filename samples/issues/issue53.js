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

let data = {
    I_MODE: 'I',
    I_S_CENTRAL1_CENTRAL: {
        KTOKD: '0001',
    },
    I_S_CENTRAL2_ADDRESS: {
        NAME: 'Customer 177',
        CITY: 'Parsippany',
        TRANSPZONE: '0000000003',
        COUNTRY: 'US',
        LANGU: 'EN',
        SORT1: 'Customer 177',
    },
};

let currentdate1 = new Date();

console.log('Start:', currentdate1);

client
    .open()

    .then(() => {
        console.log(client.connectionInfo);
        client
            .call('ZBUILTIO_FLOW_CUSTOMER_CONTRO2', data)
            .then(res => {
                console.log('Result ZBUILTIO_FLOW_CUSTOMER_CONTRO2:', res);
                var currentdate2 = new Date();
                console.log('End:', currentdate2);
            })
            .catch(err => {
                console.error('Error invoking ZBUILTIO_FLOW_CUSTOMER_CONTRO2:', JSON.stringify(err));
                var currentdate2 = new Date();
                console.log('End:', currentdate2);
            });
    })

    .catch(err => {
        console.error('Error open:', err);
    });

const connParams = require('../test/connParams');
let rfc = require('./sapnwrfc');

let pool = new rfc.Pool(connParams);

pool.acquire(client => {
    console.log('callback', client);

    console.log(1, pool.status());
});

pool
    .acquire()
    .then(client => {
        console.log(2, pool.status());
        pool
            .acquire()
            .then(client => {
                console.log(3, pool.status());
                pool.release(client);

                console.log(4, pool.status());
            })
            .catch(err => {
                console.error('promise error', err);
            });

        pool.release(client);

        console.log(5, pool.status());

        pool.dispose();

        console.log(6, pool.status());
    })
    .catch(err => {
        console.error('promise error', err);
    });

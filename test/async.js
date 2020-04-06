const setup = require("./setup");
const client = setup.client();
const WAIT_SECONDS = 3;
let A = 0;

function f(err, res) {
    console.log('err:', err);
    console.log('res:', res);
    console.log(++A);
}
let p = client
    .open()
    .then(() => {
        console.log('call');
        client.call('/COE/RBP_FE_WAIT', {
                IV_SECONDS: 1
            })
            .then(res => {
                console.log(`call callback`);
                client.close(() => {
                    console.log(`close callback`);
                });
            })
            .catch(err => {
                console.error(err);
            });
    })
console.log(p)
/*
try {
    let asyncRes = 0;
    console.log(client.isAlive);
    client.connect((err) => {
        client.invoke('/COE/RBP_FE_WAIT', {
                IV_SECONDS: 3
            },
            function (err, res) {
                if (err) throw (err)
                asyncRes = 1;
                console.log(3, asyncRes)
            });
        console.log(1, asyncRes)
        client.ping().then(res => {
            console.log(2, res)
        })

        client.invoke('/COE/RBP_FE_WAIT', {
                IV_SECONDS: WAIT_SECONDS
            },
            function (err) {
                console.log(1, asyncRes);
                if (err || (++asyncRes == 2)) console.log('done');
            });

        client.invoke('/COE/RBP_FE_WAIT', {
                IV_SECONDS: WAIT_SECONDS
            },
            function (err) {
                console.log(2, asyncRes);
                if (err || (++asyncRes == 2)) console.log('done');;
            });
    })

} catch (ex) {
    console.error('@@', ex)
}
*/

const setup = require("./setup");
const client = setup.client;
const WAIT_SECONDS = 10;
let A = 0;

function f(err, res) {
    console.log('err:', err);
    console.log('res:', res);
    console.log(++A);
}
client.close().then(() => {
    console.log(client.isAlive)
}).catch(err => console.error(err))
/*
client.close(() => {
    console.log(client.isAlive)
})

client.close().then(() => {
    console.log(client.isAlive)
    //client.ping().then(res => {
    //    console.log(res)
    //});
});
/*
client.ping().then(res => {
    console.log(res)
});

client.connect(() => {
    client.close().then(() => {
        console.log(client.isAlive)
        client.ping().then(res => {
            console.log(res)
        });
    });

})


/*
client.connect(() => {
    client.invoke('/COE/RBP_FE_WAIT', {
        IV_SECONDS: WAIT_SECONDS
    }, f);
    client.invoke('/COE/RBP_FE_WAIT', {
        IV_SECONDS: 1
    }, f);
})
*/

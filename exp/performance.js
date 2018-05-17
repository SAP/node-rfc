const rfc = require('../sapnwrfc');
const rfcClient = rfc.Client;

const connParams = require('../test/connParams');

let result;

function yyyymmdd(date) {
    let mm = date.getMonth() + 1;
    let dd = date.getDate();
    return [date.getFullYear(), mm > 9 ? mm : '0' + mm, dd > 9 ? dd : '0' + dd].join('');
}
function test(err, res) {
    if (err) {
        console.error('err:', err);
    } else {
        let COUNT = 10000;
        client.invoke(
            'STFC_PERFORMANCE',
            { CHECKTAB: 'X', LGET0332: COUNT.toString(), LGET1000: COUNT.toString() },

            function(err, res) {
                if (err) {
                    console.log(err);
                } else {
                    for (let k of Object.keys(res)) {
                        if (res[k].length) console.log(k, res[k].length);
                    }
                    client.close();
                    result = res;
                }
            }
        );
    }
}

const client = new rfcClient(connParams);

console.log(rfcClient.getVersion());

client.connect(test);

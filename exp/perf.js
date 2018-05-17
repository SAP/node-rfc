const rfc = require('../sapnwrfc');

const connParams = require('../test/connParams');

const rfcClient = rfc.Client;
console.log(rfcClient.getVersion());

//connParams.user = 'XXX';

function toABAPdate(date) {
    let mm = date.getMonth() + 1;
    let dd = date.getDate();
    return [date.getFullYear(), mm > 9 ? mm : '0' + mm, dd > 9 ? dd : '0' + dd].join('');
}
const client = new rfcClient(connParams);

client.connect(function(err) {
    if (err) {
        console.error(err);
    } else {
        let endDate = new Date();
        let startDate = new Date(endDate);
        startDate.setDate(startDate.getDate() - 1);
        client.invoke(
            'SWNC_READ_SNAPSHOT',
            {
                READ_TIMEZONE: 'UTC',
                READ_START_DATE: toABAPdate(startDate),
                READ_START_TIME: '000000',
                READ_END_DATE: toABAPdate(endDate),
                READ_END_TIME: '235959',
                TIME_RESOLUTION: 60,
            },

            function(err, res) {
                console.log(res.HITLIST_DATABASE.length, res.HITLIST_RESPTIME.length);
            }
        );
    }
});

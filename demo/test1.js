const binary = require('node-pre-gyp');
const path = require('path');
const rfc_path = binary.find(path.resolve(path.join(__dirname,'../package.json')));
const rfc = require(rfc_path);

//const rfc = require('../sapnwrfc');
const rfcClient = rfc.Client;

const connParams = {
    user: 'demo',
    passwd: 'Welcome',
    ashost:  '10.117.19.101',
    saprouter: '/H/203.13.155.17/W/xjkb3d/H/172.19.138.120/H/',
    sysnr: '00',
    lang: 'EN',
    client: '100'
}


const client = new rfcClient(connParams);

if (client.getSVersion) console.log(rfcClient.getSVersion());
console.log(client.getVersion());

client.connect(function (err, res) {
    if (err) {
        console.error('err:', err);
    } else {
        console.log('res', res);
    }
    process.exit();
});

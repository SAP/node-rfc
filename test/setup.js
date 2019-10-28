const nodeRfc = require("../package.json").dependencies["node-rfc"];
const rfcClient = require(nodeRfc ? "node-rfc" : "../lib").Client;
const rfcPool = require(nodeRfc ? "node-rfc" : "../lib").Pool;
const abapSystem = require("./abapSystem")();
const client = new rfcClient(abapSystem);
const UNICODETEST = "ทดสอบสร้างลูกค้าจากภายนอกครั้งที่".repeat(7);

const sync = function(client, done) {
    client.ping((err, res) => {
        if (res) {
            done();
        } else {
            client.reopen(err => {
                done(err);
            });
        }
    });
};

module.exports = {
    rfcClient: rfcClient,
    rfcPool: rfcPool,
    abapSystem: abapSystem,
    client: client,
    UNICODETEST: UNICODETEST,
    CONNECTIONS: 0x80,
    sync: sync
};

const nodeRfc = require("../package.json").dependencies["node-rfc"];
const rfcClient = require(nodeRfc ? "node-rfc" : "../lib").Client;
const rfcPool = require(nodeRfc ? "node-rfc" : "../lib").Pool;
const rfcThroughput = require(nodeRfc ? "node-rfc" : "../lib").Throughput;
const Promise = require(nodeRfc ? "node-rfc" : "../lib").Promise;
const abapSystem = require("./abapSystem")();
const UNICODETEST = "ทดสอบสร้างลูกค้าจากภายนอกครั้งที่".repeat(7);

const sync = function (client, done) {
    client.ping((err, res) => {
        if (res) {
            done();
        } else {
            client.reopen((err) => {
                done(err);
            });
        }
    });
};

module.exports = {
    rfcClient: rfcClient,
    rfcPool: rfcPool,
    rfcThroughput: rfcThroughput,
    Promise: Promise,
    abapSystem: abapSystem,
    UNICODETEST: UNICODETEST,
    CONNECTIONS: 0x20,
    sync: sync,
    client: (system = abapSystem, options) => {
        return new rfcClient(system, options);
    },
};

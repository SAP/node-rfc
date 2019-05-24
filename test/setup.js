const nodeRfc = require("../package.json").dependencies["node-rfc"];
const rfcClient = require(nodeRfc ? "node-rfc" : "../lib").Client;
const rfcPool = require(nodeRfc ? "node-rfc" : "../lib").Pool;
const abapSystem = require("./abapSystem")();
const client = new rfcClient(abapSystem);
const UNICODETEST = "ทดสอบสร้างลูกค้าจากภายนอกครั้งที่ 3".repeat(7);

module.exports = {
    rfcClient: rfcClient,
    rfcPool: rfcPool,
    abapSystem: abapSystem,
    client: client,
    UNICODETEST: UNICODETEST,
    CONNECTIONS: 25
};

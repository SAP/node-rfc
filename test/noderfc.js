const nodeRfc = require('../package.json').dependencies['node-rfc'];
module.exports = require(nodeRfc ? 'node-rfc' : '../lib');

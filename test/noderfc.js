process.env.DYLD_FALLBACK_LIBRARY_PATH = '/usr/local/sap/nwrfcsdk/lib';
const nodeRfc = require('../package.json').dependencies['node-rfc'];
module.exports = require(nodeRfc ? 'node-rfc' : '../lib');

'use strict';

var binary = require('node-pre-gyp');
var path = require('path');
var rfc_path = binary.find(path.resolve(path.join(__dirname,'package.json')));
var rfc = require(rfc_path);
module.exports = rfc;

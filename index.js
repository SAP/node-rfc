'use strict';

var rfc = null;

try {
  if(process.platform == "linux" && process.arch == "x64") {
    rfc = require('./build/linux_x64/rfc');
  }
  else if (process.platform == "win32" && process.arch == "x64") {
    rfc = require('./build/win32_x64/rfc');
  }
  /* win 32 bit and darwin not supported
  else if(process.platform == "win32" && process.arch == "ia32") {
    rfc = require('./build/win32_x86/rfc');
  }
  else if(process.platform == "darwin" && process.arch == "x64") {
    rfc = require('./build/osx_x64/rfc');
  }
  */
  module.exports = rfc;
}
catch (e) {
  console.log('Platform not supported', process.platform, process.arch);
}


'use strict';

var rfc = null;
var plattformPath = null;

if (process.platform === "linux" && process.arch === "x64") {
    plattformPath = './build/linux_x64/rfc';
} else if (process.platform === "win32" && process.arch === "x64") {
    plattformPath = './build/win32_x64/rfc';
} else {
    console.log('Platform not supported', process.platform, process.arch);
}
/* win 32 bit and darwin not supported
} else if (process.platform === "win32" && process.arch === "ia32") {    
    plattformPath = './build/win32_x86/rfc';
}
else if (process.platform === "darwin" && process.arch === "x64") {
    plattformPath = './build/osx_x64/rfc';    
} */

if (plattformPath) {
    try {
        rfc = require(plattformPath);
        module.exports = rfc;
    } catch (err) {
        switch (err.code) {
        case 'MODULE_NOT_FOUND':
            console.log('Could not load module ' + plattformPath + ' ' + err.toString());
            break;
        default:
            console.log('Error Loading Module - (may SAP RFC Library in path not set?) ' + err.toString());
        }
    }
}

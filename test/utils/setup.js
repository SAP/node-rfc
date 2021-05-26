// SPDX-FileCopyrightText: 2014 SAP SE Srdjan Boskovic <srdjan.boskovic@sap.com>
//
// SPDX-License-Identifier: Apache-2.0

const _node_rfc = require("../../package.json").dependencies["node-rfc"],
    _binding = require(_node_rfc ? "node-rfc" : "../../lib"),
    _abapSystem = require("./abapSystem"),
    _Client = _binding.Client,
    _Pool = _binding.Pool,
    _Throughput = _binding.Throughput,
    _Promise = _binding.Promise,
    os = require("os");

const _UNICODETEST = "ทดสอบสร้างลูกค้าจากภายนอกครั้งที่".repeat(7),
    _UNICODETEST2 = "Hällü ßärÖÄ อกครั้งที่".repeat(3);

const _sapnwrfcIniPath = require("path").join(process.cwd(), "test"),
    _CryptoLibPath = {
        darwin: "/Applications/Secure Login Client.app/Contents/MacOS/lib/libsapcrypto.dylib",
        linux: "/usr/local/sap/cryptolib/libsapcrypto.so",
        win32:
            //"C:\\Program Files\\SAP\\FrontEnd\\SecureLogin\\libsapcrypto.dll",
            "C:\\Tools\\cryptolib\\sapcrypto.dll",
    };
// _ClientPSEPath = {
//     darwin: "/Users/d037732/dotfiles/sec/rfctest.pse",
//     linux: "/home/www-admin/sec/rfctest.pse",
//     win32: "C:\\Tools\\sec\\rfctest.pse",
// };

const _environment = {
    platform: {
        name: os.platform(),
        arch: os.arch(),
        release: os.release(),
    },
    env: {
        SAPNWRFC_HOME: process.env.SAPNWRFC_HOME || "",
        RFC_INI: process.env.RFC_INI || "",
    },
    noderfc: {
        //version: "Deactivate logging: LOG_RFC_CLIENT",
        version: require("../../package.json").version,
        nwrfcsdk: { major: 7500, minor: 0, patchLevel: expect.any(Number) },
    },
    versions: process.versions,
};

const _CONNECTIONS = 0x20,
    _direct_client = (system = _abapSystem(), options) => {
        return new _Client(system, options);
    },
    _poolConfiguration = {
        connectionParameters: _abapSystem(),
    };

_binding.setIniFileDirectory(_sapnwrfcIniPath);
// _binding.loadCryptoLibrary(_CryptoLibPath[process.platform]);

module.exports = {
    binding: _binding,
    Client: _Client,
    Pool: _Pool,
    Throughput: _Throughput,
    Promise: _Promise,
    abapSystem: _abapSystem,
    RefEnvironment: _environment,
    UNICODETEST: _UNICODETEST,
    UNICODETEST2: _UNICODETEST2,
    CONNECTIONS: _CONNECTIONS,
    direct_client: _direct_client,
    poolConfiguration: _poolConfiguration,
    sapnwrfcIniPath: _sapnwrfcIniPath,
    CryptoLibPath: _CryptoLibPath,
};

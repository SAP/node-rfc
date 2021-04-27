// SPDX-FileCopyrightText: 2014 SAP SE Srdjan Boskovic <srdjan.boskovic@sap.com>
//
// SPDX-License-Identifier: Apache-2.0

const _node_rfc = require("../../package.json").dependencies["node-rfc"];
const _binding = require(_node_rfc ? "node-rfc" : "../../lib");
const _abapSystem = require("./abapSystem");

const _Client = _binding.Client;
const _Pool = _binding.Pool;
const _Throughput = _binding.Throughput;
const _Promise = _binding.Promise;
const _UNICODETEST = "ทดสอบสร้างลูกค้าจากภายนอกครั้งที่".repeat(7);
const _UNICODETEST2 = "Hällü ßärÖÄ อกครั้งที่".repeat(3);
const os = require("os");

const _INI_PATH = require("path").join(process.cwd(), "test");

_binding.setIniFileDirectory(_INI_PATH);

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

const _CONNECTIONS = 0x20;

_direct_client = (system = _abapSystem(), options) => {
    return new _Client(system, options);
};

_poolConfiguration = {
    connectionParameters: _abapSystem(),
};

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
    INI_PATH: _INI_PATH,
};

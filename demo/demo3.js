// Copyright 2014 SAP AG.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http: //www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific
// language governing permissions and limitations under the License.

"use strict";

//var rfc = require('rfc');
var rfc = require('node-rfc');

var SNC_LIB = 'C:\\Program Files\\SAP\\FrontEnd\\SecureLogin\\lib\\sapcrypto.dll'; // 64 bit
// var SNC_LIB = 'C:\\Program Files (x86)\\SECUDE\\OfficeSecurity\\secude.dll'; //32 bit

var SNCWIN64 = {
 'ashost': 'somehost.bgd.com',
 'client': '000',
 'lang': 'EN',
 'snc_lib': SNC_LIB,
 'snc_mode': '1',
 'snc_partnername': 'p:CN=I64, O=SAP-AG, C=DE',
 'snc_sso': '1',
 'sysnr': '00' };

var client = new rfc.Client(SNCWIN64);

console.log('Client Version: ', client.getVersion());

console.log('Connecting...');
client.connect(function(err) {
  if (err) {
    return console.error('could not connect to server', err);
  }

  console.log('Invoking SUT_CONNECTOR');
  client.invoke('SUT_CONNECTOR',
    { IV_CALLBACK: 'CL_SUT_CONN_ABAP_UNIT',
      IT_PARAMETER: [ {KEY: 'RESULT_KEY', VALUE: 'DEV_SUT'} ]
    },
    function(err, res) {
      if (err) {
        return console.error('Error invoking SUT_CONNECTOR:', err);
      }
      console.log(res);
    });
});


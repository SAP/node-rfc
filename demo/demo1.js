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

var rfc = require('node-rfc');

var connParams = {
  user: 'testuser',
  passwd: 'testpass',
  ashost: '11.12.13.14',
  sysnr: '00',
  client: '820',
  saprouter: '/H/113.13.52.66/S/1876/G/hjjn7v/K/12.14.17.49/U/'
};

var client = new rfc.Client(connParams, true);

console.log('Client Version: ', client.getVersion());

console.log('Connecting...');
client.connect(function(err) {
  if (err) {
    return console.error('could not connect to server', err);
  }

  console.log('Invoking BAPI_USER_GET_DETAIL');
  client.invoke('BAPI_USER_GET_DETAIL',
    { USERNAME: 'DEMO' },
    function(err, res) {
      if (err) {
        return console.error('Error invoking BAPI_USER_GET_DETAIL:', err);
      }
      console.log(res);
    });
});


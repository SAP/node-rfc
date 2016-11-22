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

  console.log('Invoking STFC_CONNECTION');
  client.invoke('STFC_CONNECTION',
    { REQUTEXT: 'H€llö SAP!' },
    function(err, res) {
      if (err) {
        return console.error('Error invoking STFC_CONNECTION:', err);
      }
      console.log('Result STFC_CONNECTION:', res);
    });


  var importStruct = {
    RFCFLOAT: 1.23456789,
    RFCCHAR1: 'A',
    RFCCHAR2: 'BC',
    RFCCHAR4: 'DEFG',

    RFCINT1: 1,
    RFCINT2: 2,
    RFCINT4: 345,

    RFCHEX3: 'fgh',

    RFCTIME: '121120',
    RFCDATE: '20140101',

    RFCDATA1: '1DATA1',
    RFCDATA2: 'DATA222'
  };
  var importTable = [importStruct];

  console.log('Invoking STFC_STRUCTURE');
  client.invoke('STFC_STRUCTURE',
    { IMPORTSTRUCT: importStruct, RFCTABLE: importTable },
    function(err, res) {
      if (err) {
        return console.error('Error invoking STFC_STRUCTURE:', err);
      }
      console.log('Result STFC_STRUCTURE:', res);
  });

});


// XXX: implement connection pooling like postgres?!
/*
var pg = require('pg');
var conString = "postgres://postgres:1234@localhost/postgres";

pg.connect(conString, function (err, client, done) {
  if (err) {
    return console.error('error fetching client from pool', err);
  }
  client.query('SELECT $1::int AS numbor', ['1'], function (err, result) {
    //call `done()` to release the client back to the pool
    done();

    if (err) {
      return console.error('error running query', err);
    }
    console.log(result.rows[0].numbor);
    //output: 1
  });
});
*/

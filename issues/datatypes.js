const rfc = require("../build/linux_x64/rfc.node");
const assert = require('assert');
const Decimal = require('decimal.js');

let connParams = {
  'user': 'demo',
  'passwd': 'welcome',
  'ashost': '10.68.110.51',
  'sysnr': '00',
  'lang': 'EN',
  'client': '620',
  'sysid': 'MME'
}

let client1 = new rfc.Client(connParams);
let client2 = new rfc.Client(connParams);
let client3 = new rfc.Client(connParams);

client1.connect(function (err) {
  if (err) {
    return console.error('could not connect to server', err);
  }
  let isInput = {
    // Float
    ZFLTP: 0.123456789,
  
    // Decimal
    ZDEC: 12345.67,
  
    // Currency, Quantity
    ZCURR: 1234.56,
    ZQUAN: 12.3456,
    ZQUAN_SIGN: -12.345
  };
  client1.invoke('/COE/RBP_FE_DATATYPES', {IS_INPUT: isInput},
    function (err, res) {
      if (err) {
        return console.error(err);
      }
      console.log('\nnumbers');
      for (let k in isInput) {
        let inVal = isInput[k];
        let outVal = res.ES_OUTPUT[k];
        console.log(k, typeof(inVal), typeof(outVal), inVal, outVal);
        if (typeof(inVal) === typeof(outVal))
          assert(inVal === outVal);
        else
          assert(outVal.toString() === inVal.toString());
      }
    });
});


client2.connect(function (err) {
  if (err) {
    return console.error('could not connect to server', err);
  }
  let isInput = {
    // Float
    ZFLTP: '0.123456789',
  
    // Decimal
    ZDEC: '12345.67',
  
    // Currency, Quantity
    ZCURR: '1234.56',
    ZQUAN: '12.3456',
    ZQUAN_SIGN: '-12.345'
  };
  client2.invoke('/COE/RBP_FE_DATATYPES', {IS_INPUT: isInput},
    function (err, res) {
      if (err) {
        return console.error(err);
      }
      console.log('\nstrings');
      for (let k in isInput) {
        let inVal = isInput[k];
        let outVal = res.ES_OUTPUT[k];
        console.log(k, typeof(inVal), typeof(outVal), inVal, outVal);
        if (typeof(inVal) === typeof(outVal))
          assert(inVal === outVal);
        else
          assert(outVal.toString() === inVal.toString());
      }
    });
});

client3.connect(function (err) {
  if (err) {
    return console.error('could not connect to server', err);
  }
  let isInput = {
    // Float
    ZFLTP: Decimal('0.123456789'),

    // Decimal
    ZDEC: Decimal('12345.67'),

    // Currency, Quantity
    ZCURR: Decimal('1234.56'),
    ZQUAN: Decimal('12.3456'),
    ZQUAN_SIGN: Decimal('-12.345')
  };
  client3.invoke('/COE/RBP_FE_DATATYPES', {IS_INPUT: isInput},
    function (err, res) {
      if (err) {
        return console.error(err);
      }
      console.log('\ndec');
      for (let k in isInput) {
        let inVal = isInput[k];
        let outVal = res.ES_OUTPUT[k];
        console.log(k, typeof(inVal), typeof(outVal),  inVal instanceof Decimal ? inVal.toString() : inVal, outVal instanceof Decimal ? outVal.toString() : outVal);
        if (typeof(inVal) === typeof(outVal))
          assert(inVal === outVal);
        else
          assert(outVal.toString() === inVal.toString());
      }
    });
});
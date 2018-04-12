let rfc = require("../build/linux_x64/rfc.node");

let connParams = {
  user: 'demo',
  passwd: 'Welcome',
  ashost: '10.117.19.101',
  saprouter: '/H/203.13.155.17/W/xjkb3d/H/172.19.138.120/H/',
  sysnr: '00',
  lang: 'EN',
  client: '100'
}

client = new rfc.Client(connParams);
/*
let MAXROWS = "2";
let MATNRSELECTION_str = {
        SIGN:         "I", 
        OPTION:     "CP", 
        MATNR_LOW:     "AB*"
    };    
let MATNRSELECTION_tab = [MATNRSELECTION_str];
    
client.connect(function(err) {
  if (err) {
    return console.error('could not connect to server', err);
  }    
  client.invoke('BAPI_MATERIAL_GETLIST', {
     MAXROWS: MAXROWS,
     MATNRSELECTION: MATNRSELECTION_tab
  },
  function(err, res) {
    if (err) {
      return console.error('Error invoking BAPI_MATERIAL_GETLIST:', err);
    }
    console.log('Result BAPI_MATERIAL_GETLIST:', res);
  });    
});

return;
*/

let importStruct = {
  //RFCINT1: 1,
  //RFCINT2: 2,
  //RFCINT4: 3,
  //RFCCHAR4: 65,
  RFCFLOAT: "A"
}

let importTable = [importStruct]


client.connect(function (err) {
  if (err) {
    return console.error('could not connect to server', err);
  }
  client.invoke('STFC_STRUCTURE',
    { IMPORTSTRUCT: importStruct, RFCTABLE: importTable },
    function (err, res) {
      if (err) {
        return console.error(err);
      }
      console.log(importStruct, { RFCFLOAT: res.ECHOSTRUCT.RFCFLOAT } );
      //console.log(importStruct, { RFCCHAR4: res.ECHOSTRUCT.RFCCHAR4 } );
      //console.log(importStruct, { RFCINT1: res.ECHOSTRUCT.RFCINT1, RFCINT2: res.ECHOSTRUCT.RFCINT2, RFCINT4: res.ECHOSTRUCT.RFCINT4});
    });
});

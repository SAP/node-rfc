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

connParams = {
  user: 'demo',
  passwd: 'welcome',
  ashost: '10.68.110.51',
  sysnr: '00',
  client: '620',
  lang: 'EN'
}

client = new rfc.Client(connParams);

NOT_REQUESTED = [
  'ET_COMPONENTS',
  'ET_HDR_HIERARCHY',
  'ET_MPACKAGES',
  'ET_OPERATIONS',
  'ET_OPR_HIERARCHY',
  'ET_PRTS',
  'ET_RELATIONS',
]

client.connect(function (err) {
  if (err) {
    return console.error('could not connect to server', err);
  }
  client.invoke('EAM_TASKLIST_GET_DETAIL',
    { IV_PLNTY:'A',
      IV_PLNNR:'00100000'
    },
    function (err, res) {
      if (err) {
        return console.error(err);
      }
      console.log(res.ET_RETURN);
    },
    {'notRequested': NOT_REQUESTED}
  );
});


/*
        result = self.conn.call('EAM_TASKLIST_GET_DETAIL', {'not_requested': NOT_REQUESTED}, IV_PLNTY=PLNTY, IV_PLNNR=PLNNR)
        assert len(result['ET_RETURN']) == 0
        result = self.conn.call('EAM_TASKLIST_GET_DETAIL', IV_PLNTY=PLNTY, IV_PLNNR=PLNNR)
        assert len(result['ET_RETURN']) == 1
*/
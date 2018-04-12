let rfc = require("../build/linux_x64/rfc.node");

let connParams = {
  user: 'demo',
  passwd: 'Welcome',
  ashost:  '10.117.19.101',
  saprouter: '/H/203.13.155.17/W/xjkb3d/H/172.19.138.120/H/',
  sysnr: '00',
  lang: 'EN',
  client: '100'
}

let fs = require ("fs");
let fileName = `xxx-withClose-${process.version}.txt`;
fs.writeFile(fileName, `iteration\t\trss\t\theapTotal\t\theapUsed\t\texternal\n`)

let connections = [];

function run (i) {

    console.log(i);

    if (i == 500) {
      console.log(connections.length, connections.every(v => v.isAlive() == false));
      return;
    }

    let client = new rfc.Client(connParams);
    connections[i] = client;

    client.connect(function (err) {
            if (err) { // check for login/connection errors
                        return console.error('could not connect to server', err);
                    }

            client.invoke("STFC_PERFORMANCE", {"CHECKTAB":"X","LGET0332":"999", "LGET1000": "999"}, function (err, res) {
              if (err) {
                console.log(err)
                fs.appendFileSync(fileName, err);
              } else {
                        let memUsage = process.memoryUsage();
                        let line = `${i}\t\t${(memUsage.rss / 1024 / 1024).toFixed(3)}MB\t\t${(memUsage.heapTotal / 1024 / 1024).toFixed(3)}MB\t\t${(memUsage.heapUsed / 1024 / 1024).toFixed(3)}MB\t\t${(memUsage.external / 1024 / 1024).toFixed(3)}MB\t\t${err ? err : ""}\n`;
                        connections[i].close();
                        fs.appendFile(fileName, line);
                        run(++i);
                        delete res;
              }
            });
        });
}

run(1);

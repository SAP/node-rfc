const Pool = require('../lib').Pool;

const connection = require('../test/connParams');

const pool = new Pool(connection);

const pool2 = new Pool(connection);

console.log(pool.status);

let id = new Set([...Array(10).keys()]); // [0, 1, 2, ... 9]

for (let i = 0; i < 10; i++) {
    pool.acquire().then(client => {
        console.log(client.id - 1, id);
        id.delete(client.id - 1);
    });
}

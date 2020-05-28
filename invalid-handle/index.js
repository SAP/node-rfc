const http = require("http");
const express = require("express");
const RFC = require("../lib");

const logger = {
    info: (...args) => console.log(...args),
    debug: (...args) => console.debug(...args),
    error: (...args) => console.error(...args),
};

const logClient = (label, client) => logger.debug(`${label}`);

const pool = new RFC.Pool({ DEST: "MME" }, { min: 3 });

const app = express();

app.get("/doc/function/:id", async (req, resp) => {
    let client = { id: -1 }; // fallback, if acquire fails

    const rno = req.params.id;

    try {
        logClient("");
        logClient(`request: ${rno}`);

        client = new RFC.Client({ DEST: "MME" });

        await client.open();

        await client.open();

        client = await pool.acquire(rno);
        logClient(
            `post acquire req ${rno} client: ${client.id}:${client._connectionHandle}`
        );

        const result = await client.call("FUNCTION_IMPORT_DOKU", {
            FUNCNAME: "FUNCTION_IMPORT_DOKU",
        });
        logClient(
            `post call req ${rno} client: ${client.id}:${client._connectionHandle}`
        );

        pool.release(client, rno);
        const output = `resultLen: ${JSON.stringify(result, null, 2).length}`;

        logClient(`post release req ${rno} ${output}`);
        resp.status(200).send(`${rno} ${output}` + "\n");
    } catch (error) {
        logClient(
            `!!error req ${rno} client: ${client.id}:${
                client._connectionHandle
            } ${JSON.stringify(error, null, 2)}`
        );
        logger.error(error);

        resp.status(500).send(error);
    }
});

const port = 3456;
http.createServer(app).listen(port, "localhost", () =>
    logger.info(`pid ${process.pid} listening on localhost:${port}`)
);

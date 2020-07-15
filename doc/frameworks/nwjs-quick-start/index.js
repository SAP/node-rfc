const Client = require("node-rfc").Client;
const client = new Client({ DEST: "MME" });
const onClick = () => {
    alert(`node-rfc:  ${client.version.binding}`);
};

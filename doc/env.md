- **[NODE_RFC_MODULE_PATH](#NODE_RFC_MODULE_PATH)**


## NODE_RFC_MODULE_PATH

- Use module path 'node_modules/node-rfc' when using 'node-gyp-build'(`path.resolve(__dirname, "..")`)

- If you want to specify the 'node_modules/node-rfc' path directly, set NODE_RFC_MODULE_PATH

### Usage

- .env file

  ```bash
  # .env
  NODE_RFC_MODULE_PATH=./node_modules/node-rfc
  ```

- node env variable

  ```bash
  $ NODE_RFC_MODULE_PATH=./node_modules/node-rfc node rfc-execute.js
  ```

## Data Types

NodeJS data types are automatically converted to ABAP data types and vice versa:

| NodeJS to ABAP                 | ABAP    | ABAP to NodeJS                   | Client Option                                             |
| :----------------------------- | :------ | :------------------------------- | :-------------------------------------------------------- |
| Number                         | INT     | Number                           |                                                           |
| String                         | CHAR    | String                           |                                                           |
| String                         | STRING  | String                           |                                                           |
| Buffer                         | BYTE    | Buffer                           |                                                           |
| Buffer                         | XSTRING | Buffer                           |                                                           |
| **String** \| Function         | DATE    | **String** \| Function           | ["date"](#date-and-time-conversion-options-date-and-time) |
| **String** \| Function         | TIME    | **String** \| Function           | ["time"](#date-and-time-conversion-options-date-and-time) |
| String                         | UTCLONG | String                           |                                                           |
| String                         | NUM     | String                           |                                                           |
| **String** \| Number \| Object | FLOAT   | Number                           |                                                           |
| **String** \| Number \| Object | BCD     | **String** \| Number \| Function | ["bcd"](#decimal-data-conversion-option-bcd)              |
| **String** \| Number \| Object | DECF16  | **String** \| Number \| Function | ["bcd"](#decimal-data-conversion-option-bcd)              |
| **String** \| Number \| Object | DECF34  | **String** \| Number \| Function | ["bcd"](#decimal-data-conversion-option-bcd)              |

References:

-   [ABAP built-in numeric types](https://help.sap.com/doc/abapdocu_752_index_htm/7.52/en-US/index.htm?file=abenbuiltin_types_numeric.htm)

-   [JavaScript Number Objects](https://www.ecma-international.org/ecma-262/#sec-number-objects)

-   [Min/max values](https://github.com/SAP/node-rfc/blob/master/test/testutils/config.js)

### Numeric types

ABAP built-in numeric types are mapped to JavaScript Number Objects.

Binary ABAP float type, FLOAT, is converted to NodeJS Number Object. Number, String, or Number Object can be sent from NodeJS to ABAP.

Decimal ABAP float types, BCD, DECF16 and DECF34, are represented as NodeJS Strings by default.

Optionally represented as the Number Object, or custom decimal number object like [Decimal](https://github.com/MikeMcl/decimal.js/), see [client option "bcd"](#decimal-data-conversion-option-bcd).

### Binary types

ABAP binary types, BYTE and XSTRING, are mapped to NodeJS Buffer objects.

### Date/Time types

ABAP date and time types, DATS and TIMS, are character types in ABAP, represented as NodeJS String by default.

Optionally represented as custom Date/Time objects, see [client options "date" and "time"](#date-and-time-conversion-options-date-and-time).

### UTCLONG

ABAP UTCLONG type is mapped to NodeJS string, with initial value `0000-00-00T00:00:00.0000000`.

## ABAP Function Module

Remote enabled ABAP function modules (RFM) parameters can be ABAP variables, structures and tables.

ABAP variables are mapped to NodeJS variables, ABAP structures to NodeJS objects (simple key-value pairs) and ABAP tables to NodeJS arrays of objects, representing ABAP structures.

Taking ABAP RFM `STFC_STRUCTURE` as example, we see four parameters:

```abap
FUNCTION STFC_STRUCTURE.
*"----------------------------------------------------------------------
*"*"Lokale Schnittstelle:
*"       IMPORTING
*"             VALUE(IMPORTSTRUCT) LIKE  RFCTEST STRUCTURE  RFCTEST
*"       EXPORTING
*"             VALUE(ECHOSTRUCT) LIKE  RFCTEST STRUCTURE  RFCTEST
*"             VALUE(RESPTEXT) LIKE  SY-LISEL
*"       TABLES
*"              RFCTABLE STRUCTURE  RFCTEST
*"----------------------------------------------------------------------
```

| Parameter name | Direction    | Parameter type |
| :------------- | :----------- | :------------- |
| IMPORTSTRUCT   | to ABAP      | structure      |
| ECHOSTRUCT     | from ABAP    | structure      |
| RESPTEXT       | from ABAP    | variable       |
| RFCTABLE       | to/from ABAP | table          |

Using ABAP transaction SE37 in ABAP backend system, you can enter the input data, run the function module and inspect results.

To consume this function module from NodeJS, first the node-rfc client connection shall be instantiated, using ABAP backend system connection parameters.

## Connection Parameters

Connection parameters are provided as simple NodeJS object. The complete list of supported parameters is given in `sapnwrfc.ini` file, located in SAP NWRFC SDK `demo` folder.

```javascript
const abapConnection = {
    user: "demo",
    passwd: "welcome",
    ashost: "10.68.110.51",
    sysnr: "00",
    client: "620",
    lang: "EN",
};
```

Connection parameters can be also read from `sapnwrfc.ini` file, located in a current folder or at path set in `RFC_INI` environment variable (including the filename). Only the destination parameter is required:

```javascript
const abapConnection = {
    dest: "QI3",
};
```

**sapnwrfc.ini**

```ini
DEST=QI3
USER=demo
PASSWD=welcome
ASHOST=10.68.110.51
SYSNR=00
CLIENT=620
LANG=EN
#TRACE=3
```

```shell
$ echo $RFC_INI
./sapnwrfc.ini
```

## Direct and Managed Clients

The type of client connection can be managed (by the Connection Pool) or direct, without using Connection Pool.

The direct client is instantiated with closed connection:

```node
const connParams = { dest: "QI3" };
const Client = require("node-rfc").Client;
const client = new Client(connParams);
console.log(client.alive); // false
```

Managed client is instantiated with open connection:

```node
const Pool = require("node-rfc").Pool;
const pool = new Pool({ connectionParameters: connParams });
pool.acquire((client) => {
    console.log(client.alive); // true
});
```

An open connection is represented by unique `RFC_CONNECTION_HANDLE` pointer, assigned by SAP NWRFC SDK and exposed as a client [`connectionHandle`](api.md#getters) getter. The value of this property is zero, when the connection is closed. The client getter [`alive`](api.md#getters) is set to true, if the value is non-zero, representing an open connection.

Direct clients have access to connection [`open()`](api.md#open) and [`close()`](api.md#open) methods and each call to[`open()`](api.md#open) method will set the new [`connectionHandle`](api.md#getters)value. After the connection is closed, the[`connectionHandle`](api.md#getters) is set to zero but new opened connection can get the same [`connectionHandle`](api.md#getters) value, as the previously closed connection. The handle is just the pointer to the C ++ object, and after a free / delete operation the C-Runtime can re-use the same address again in a subsequent malloc / new. It happens very often (especially on Windows), that memory management system “notices” the block of memory is just the right size and the block is re-used, instead of allocating a new one, causing unnecessary fragmentation. The [`connectionHandle`](api.md#getters) can therefore change during the direct client instance lifecycle and more client instances may get the same[`connectionHandle`](api.md#getters), not at the same time. If not synchronized properly, the delayed direct client [`close()`](api.md#close) call, can close the handle already assigned to another client instance, typically causing the `RFC_INVALID_HANDLE` errors when that client tries to make the RFC call.

This situation may happen when the `node-rfc` is consumed by multi-threaded nodejs modules, like `express` for example, in which case your application typically can't influence the sequence of concurrent requests` execution.

### Connection Pool

Using Connection Pool helps here because managed clients can't close or open own connections, their access to [`close()`](api.md#close) and [`open()`](api.md#open) methods is disabled. The [`connectionHandle`](api.md#getters) of the managed client is therefore constant. The managed client acquires an open connection from the Connnection Pool, using [`acquire()`](api.md#acquire) method and after no more needed, returns it back to pool, using [`release()`](api.md#release-1) method. After getting the connection back, the Connection Pool can reset the context and keep it open, ready for the next client, or close the connection. If the number of ready connections is less than pool `high` threshold parameter, the returned connection is added to ready connections, otherwise closed. The `low` threshold parameters defines a minimum number of connections, the Pool should keep open, ready for clients:

```node
const pool = new Pool({
    connectionParameters: connParams,
    clientOptions: {}, // optional
    poolOptions: { low: 2, high: 4 }, // optional
});
```

Using [ready()](api.md#ready) method, the number of ready connections can be increased, ignoring the `ready_high`:

```node
pool.ready(5).then(() => {
    // 5 ready connections
});
```

### Closing connections

The direct connection is closed by calling the client `close()` method or automatically, by client destructor.

The managed connection is returned to Connection Pool, using `release()` method and can be closed or reused by Connection Pool.

Connection Pool ready and leased connections are closed by Pool destructor.

The direct and managed connection can be automatically closed, if critical error occurs during the communication with ABAP backend system. The client [`connectionHandle`](api.md#getters) property is set to NULL and `alive` to `false`. Critical error conditions, leading to connection close, are exposed in `errorInfo` object.
The connection is closed if any of following conditions is true:

| errorInfo | Value                     | Description                                                                                                 |
| --------- | ------------------------- | ----------------------------------------------------------------------------------------------------------- |
| code      | RFC_COMMUNICATION_FAILURE | Error in Network & Communication layer.                                                                     |
| code      | RFC_ABAP_RUNTIME_FAILURE  | SAP system runtime error (SYSTEM_FAILURE): Shortdump on the backend side.                                   |
| code      | RFC_ABAP_MESSAGE          | The called function module raised an E-, A- or X-Message.                                                   |
| code      | RFC_EXTERNAL_FAILURE      | Error in external custom code. (E.g. in the function handlers or tRFC handlers.) Results in SYSTEM_FAILURE. |
|           |                           |                                                                                                             |
| group     | ABAP_RUNTIME_FAILURE      | ABAP Message raised in ABAP function modules or in ABAP runtime of the backend (e.g Kernel)                 |
| group     | LOGON_FAILURE             | Error message raised when logon fails                                                                       |
| group     | COMMUNICATION_FAILURE     | Problems with the network connection (or backend broke down and killed the connection)                      |
| group     | EXTERNAL_RUNTIME_FAILURE  | Problems in the RFC runtime of the external program (i.e "this" library)                                    |

## Pool Options

`low` is the minimum number of connections to keep open, accelerating client `acquire()` requests. **Default**: `2`.

`high` is the maximum number of connections to keep open, "recycling" returned client connections. **Default**: `4`.

## Client options

Using client options, passed to Client or Pool constructor, the default client behaviour can be modified:

| Option      | Description                                                 |
| ----------- | ----------------------------------------------------------- |
| `stateless` | Stateless connections, default **false**                    |
| `bcd`       | Decimal numbers conversion: [Numeric types](#numeric-types) |
| `date`      | Dates conversion: [Date/Time types](#datetime-types)        |
| `time`      | Times of day conversion: [Date/Time types](#datetime-types) |
| `filter`    | Result parameter types' filtering                           |

### Stateless communication option "stateless"

from [SAP NetWeaver RFC SDK ProgrammingGuide](https://support.sap.com/en/product/connectors/nwrfcsdk.html):

SAP Java Connector (JCo 3.1) and SAP .NET Connector (NCo 3.0) are stateless by default and you have to put some effort into it, if you need a stateful connection.

SAP NWRFC SDK and hence the `node-rfc` is statefull by default, which makes calling an Update-BAPI or some other RFM that stores intermediate results in the current user’s ABAP session memory very easy. For example, the program just calls the BAPI or function module, and if it succeeds, just calls
BAPI_TRANSACTION_COMMIT on the same connection, and it will run inside the same user session in the backend.

However, there are also situations, where the business logic needs a stateless connection, for example, if you have just called a function module that stored a lot of data in the ABAP session memory, you now want to call further RFMs in the same user session, and the leftover data from the previous call is now superfluous garbage that is no longer needed, which degrades the performance, or – even worse – causes unwanted side effects for the following call(s).

Of course, the application could simply close the connection and open a fresh one, which also creates a fresh ABAP user session, but this could be a bit time and resource consuming, especially when an SNC handshake must be performed during login. In this case it is easier to just use the API function `RfcResetServerContext()`. This function cleans up any user session state in the backend, but keeps the connection and the user session alive.

When this option set to `true`, the `RfcResetServerContext()` is automatically executed after each RFM call from `node-rfc`. It can be also executed any time, by calling the client `resetServerContext()` method.

### Decimal data conversion option "bcd"

With `bcd`, set to `number`, or to conversion function like [Decimal](https://github.com/MikeMcl/decimal.js/), decimal ABAP float types, BCD, DECF16 and DECF34 are represented as Number object or custom decimal number object.

:exclamation: Using `number` option is **not recommended** here, can lead to rounding errors during ABAP to NodeJS conversion.

```javascript
let clientOptions = {
    bcd: "string",
    // bcd: require("decimal.js"),
};
```

### Date and time conversion options "date" and "time"

The client option `date` shall provide `fromABAP()` function, for ABAP date string (YYYYMMDD) conversion to NodeJS date object and the `toABAP()` function, for the other way around.

3rd party libraries like [Moment](https://momentjs.com/) or standard JavaScript Date Object can be used, like shown below:

```javascript
const clientOptions = {
    date: {
        // NodeJS Date() to ABAP YYYYMMDD
        toABAP: function (date) {
            if (!(date instanceof Date))
                return new TypeError(`Date object required: ${date}`);
            let mm = date.getMonth() + 1;
            let dd = date.getUTCDate();
            return [
                date.getFullYear(),
                mm > 9 ? mm : "0" + mm,
                dd > 9 ? dd : "0" + dd,
            ].join("");
        },

        fromABAP: function (dats) {
            // ABAP YYYYMMDD to NodeJS Date()
            return new Date(
                0 | dats.substring(0, 4),
                (0 | dats.substring(4, 6)) - 1,
                (0 | dats.substring(6, 8)) + 1
            );
        },
    },
};
```

### Parameter type filter option "filter"

Using the `filter` options, certan ABAP parameter types can be removed from RFM call result JavaScript object, reducing the data volume.

Supported values are described in `sapnwrfc.h` file, located in SAP NWRFC SDK `include` folder:

```c++
typedef enum _RFC_DIRECTION
{
  RFC_IMPORT   = 0x01,                    ///< Import parameter. This corresponds to ABAP IMPORTING parameter.
  RFC_EXPORT   = 0x02,                    ///< Export parameter. This corresponds to ABAP EXPORTING parameter.
  RFC_CHANGING = RFC_IMPORT | RFC_EXPORT, ///< Import and export parameter. This corresponds to ABAP CHANGING parameter.
  RFC_TABLES   = 0x04 | RFC_CHANGING      ///< Table parameter. This corresponds to ABAP TABLES parameter.
}RFC_DIRECTION
```

```javaScript
clientOptions = {
    direction: 1 // Import parameters not copied to result object
}
```

## Invoction patterns

After getting a direct or managed client instance, the ABAP function module can be consumed from NodeJS using Async/await, Promise or callback pattern.

### Async/await

```javascript
(async function () {
    try {
        await client.open();

        const importStruct = {
            RFCINT4: 345,
            RFCFLOAT: 1.23456789,
            RFCCHAR4: "ABCD",
            RFCDATE: "20180625",
        };

        let result = await client.call("STFC_STRUCTURE", {
            IMPORTSTRUCT: importStruct,
        });

        console.log(result.RESPTEXT);
    } catch (ex) {
        console.error(ex);
    }
})();
```

### Promise

```javascript
client
    .open()
    .then(() => {
        let importStruct = {
            RFCINT4: 345,
            RFCFLOAT: 1.23456789,
            RFCCHAR4: "ABCD",
            RFCDATE: "20180625",
        };
        client
            .call("STFC_STRUCTURE", { IMPORTSTRUCT: importStruct })
            .then((result) => {
                console.log(result.RESPTEXT);
            })
            .catch((err) => {
                console.error("could not complete ABAP RFM call", err);
            });
    })
    .catch((err) => {
        console.error("could not connect to server", err);
    });
```

### Callback

```javascript
// open connection
client.connect(function (err) {
    if (err) {
        // abort if login/connection errors
        return console.error("could not connect to server", err);
    }

    // invoke the function module
    const importStruct = {
        RFCINT4: 345,
        RFCFLOAT: 1.23456789,
        RFCCHAR4: "ABCD",
        RFCDATE: "20180625",
    };

    client.invoke(
        "STFC_STRUCTURE",
        { IMPORTSTRUCT: importStruct },
        (err, result) => {
            if (err) {
                return console.error("Error invoking STFC_STRUCTURE:", err);
            }

            console.log(
                "STFC_STRUCTURE call result", Object.keys(result))
            );
        }
    );
});
```

## Environment

The `environment` object provides info on node-rfc, SAP NWRFC SDK and NodeJS platform.

It is exposed at package level, replicated at Pool and Client class and instance levels:

```shell
$ node -p "require('node-rfc')".environment         # binding
$ node -p "require('node-rfc')".Pool.environment    # class
$ node -p "require('node-rfc')".Client.environment  # class
$ node -p "new (require('node-rfc').Client)({dest: 'MME'}).environment" # instance
$ node -p "new (require('node-rfc').Pool)({connectionParameters: {dest: 'MME'}}).environment" # instance
{
    platform: { name: "darwin", arch: "x64", release: "19.5.0" },
    env: { SAPNWRFC_HOME: "/usr/local/sap/nwrfcsdk", RFC_INI: "" },
    noderfc: {
        version: "2.0.0",
        nwrfcsdk: { major: 7500, minor: 0, patchLevel: 5 },
    },
    versions: {
        node: "14.4.0",
        v8: "8.1.307.31-node.33",
        uv: "1.37.0",
        zlib: "1.2.11",
        brotli: "1.0.7",
        ares: "1.16.0",
        modules: "83",
        nghttp2: "1.41.0",
        napi: "6",
        llhttp: "2.0.4",
        openssl: "1.1.1g",
        cldr: "37.0",
        icu: "67.1",
        tz: "2019c",
        unicode: "13.0",
    },
}
```

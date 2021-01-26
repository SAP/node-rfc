-  **[Data Types](#data-types)**
   -  [Numeric types](#numeric-types)
   -  [Binary types](#binary-types)
   -  [Date/Time types](#datetime-types)
   -  [UTCLONG](#utclong)
-  **[ABAP Function Module](#abap-function-module)**
-  **[Addon](#addon)**

   -  [setIniFileDirectory](#setinifiledirectory)

    <a name="client-toc"></a>

-  **[Client](#client)**

   -  [Using `sapnwrfc.ini` file](api.md/#setIniPath)
   -  [Authentication](authentication.md)
      -  [Plain with user credentials](authentication.md/#plain-with-user-credentials)
      -  [SNC with user PSE](authentication.md/#snc-with-user-pse)
      -  [SNC with client system PSE and User X509](authentication.md/#snc-with-client-system-pse-and-user-x509)
   -  [Connection Parameters](#connection-parameters)
   -  [Direct and Managed Clients](#direct-and-managed-clients)
   -  [Client options](#client-options)
      -  [Stateless communication option "stateless"](#stateless-communication-option-stateless)
      -  [Decimal data conversion option "bcd"](#decimal-data-conversion-option-bcd)
      -  [Date and time conversion options "date" and "time"](#date-and-time-conversion-options-date-and-time)
      -  [Parameter type filter option "filter"](#parameter-type-filter-option-filter)
   -  [Error handling](#error-handling)
   -  [Invoction patterns](#invoction-patterns)
      -  [Async/await](#asyncawait)
      -  [Promise](#promise)
      -  [Callback](#callback)

-  **[Connection Pool](#connection-pool)**
   -  [Pool Options](#pool-options)
-  **[Closing connections](#closing-connections)**

<a name="server-toc"></a>

-  **[Server (experimental)](#server)**
-  **[Throughput](#throughput)**
-  **[Environment](#environment)**

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

-  [ABAP built-in numeric types](https://help.sap.com/doc/abapdocu_752_index_htm/7.52/en-US/index.htm?file=abenbuiltin_types_numeric.htm)

-  [JavaScript Number Objects](https://www.ecma-international.org/ecma-262/#sec-number-objects)

-  [Min/max values](https://github.com/SAP/node-rfc/blob/master/test/testutils/config.js)

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

## Addon

API: [api/addon](api.md#addon)

### setIniFileDirectory

Sets the directory in which the NWRFC SDK shall search for the `sapnwrfc.ini` file, exposing the `RfcSetIniPath` NWRFC SDK method. The default is the current working directory of the process. After the directory is set, the NW RFC lib automatically loads the contents of the new `sapnwrfc.ini` file from that directory. The exception is thrown if `sapnwrfc.ini` not found in the new directory.

```ts
const noderfc = require("node-rfc");
noderfc.setIniFileDirectory("/some/folder");
```

## Client

API: [api/client](api.md#client)

Using the client instance, ABAP RFMs can be consumed from NodeJS. The client constructor requires
[connection parameters](#connection-parameters) to ABAP backend system and, optionally, [client options](#client-options).

### Connection Parameters

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

### Direct and Managed Clients

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

Direct clients have access to connection [`open()`](api.md#open) and [`close()`](api.md#close) methods and each call to[`open()`](api.md#open) method will set the new [`connectionHandle`](api.md#getters)value. After the connection is closed, the[`connectionHandle`](api.md#getters) is set to zero but new opened connection can get the same [`connectionHandle`](api.md#getters) value, as the previously closed connection. The handle is just the pointer to the C ++ object and after a free / delete operation the C-Runtime can re-use the same address again in a subsequent malloc / new. It happens very often (especially on Windows), that memory management system “notices” the block of memory is just the right size and the block is re-used, instead of allocating a new one, causing unnecessary fragmentation. The [`connectionHandle`](api.md#getters) can therefore change during the direct client instance lifecycle and more client instances may get the same[`connectionHandle`](api.md#getters), not at the same time. If not synchronized properly, the delayed direct client [`close()`](api.md#close) call, can close the handle already assigned to another client instance, typically causing the `RFC_INVALID_HANDLE` errors when that client tries to make the RFC call.

This situation may happen when the `node-rfc` is consumed by multi-threaded nodejs modules, like `express` for example, when application typically can't influence the sequence of concurrent requests` execution.

Using [Connection Pool](#connection-pool) is reccomended in such scenarios.

### Client options

Using client options, passed to Client or Pool constructor, the default client behaviour can be modified:

| Option      | Description                                                 |
| ----------- | ----------------------------------------------------------- |
| `stateless` | Stateless connections, default **false**                    |
| `bcd`       | Decimal numbers conversion: [Numeric types](#numeric-types) |
| `date`      | Dates conversion: [Date/Time types](#datetime-types)        |
| `time`      | Times of day conversion: [Date/Time types](#datetime-types) |
| `filter`    | Result parameter types' filtering                           |

#### Stateless communication option "stateless"

When set to `true`, the `RfcResetServerContext()` is automatically executed after each RFM call from `node-rfc`. It can be also executed any time, by calling the client `resetServerContext()` method.

[SAP NetWeaver RFC SDK ProgrammingGuide](https://support.sap.com/en/product/connectors/nwrfcsdk.html):

SAP Java Connector (JCo 3.1) and SAP .NET Connector (NCo 3.0) are stateless by default and you have to put some effort into it, if you need a stateful connection.

SAP NWRFC SDK and hence the `node-rfc` is statefull by default, which makes calling an Update-BAPI or some other RFM that stores intermediate results in the current user’s ABAP session memory very easy. For example, the program just calls the BAPI or function module, and if it succeeds, just calls
BAPI_TRANSACTION_COMMIT on the same connection, and it will run inside the same user session in the backend.

However, there are also situations, where the business logic needs a stateless connection, for example, if you have just called a function module that stored a lot of data in the ABAP session memory, you now want to call further RFMs in the same user session, and the leftover data from the previous call is now superfluous garbage that is no longer needed, which degrades the performance, or – even worse – causes unwanted side effects for the following call(s).

Of course, the application could simply close the connection and open a fresh one, which also creates a fresh ABAP user session, but this could be a bit time and resource consuming, especially when an SNC handshake must be performed during login. In this case it is easier to just use the API function `RfcResetServerContext()`. This function cleans up any user session state in the backend, but keeps the connection and the user session alive.

#### Decimal data conversion option "bcd"

With `bcd`, set to `number`, or to conversion function like [Decimal](https://github.com/MikeMcl/decimal.js/), decimal ABAP float types, BCD, DECF16 and DECF34 are represented as Number object or custom decimal number object.

:exclamation: Using `number` option is **not recommended** here, can lead to rounding errors during ABAP to NodeJS conversion.

```javascript
let clientOptions = {
    bcd: "string",
    // bcd: require("decimal.js"),
};
```

#### Date and time conversion options "date" and "time"

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

#### Parameter type filter option "filter"

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
    filter: 1 // Import parameters not copied to result object
}
```

### Error Handling

Three types of errors can be returned to NodeJS application:

-  JavaScript exceptions, thrown when wrong parameters are provided to `Client`, `Pool` or `Throughput` constructor
-  Error objects from NWRFC SDK, like logon error or calling a non-existing RFM
-  Error objects from node node-rfc, like catching a string passed from NodeJS to ABAP RFM integer parameter field

After certain critical errors, the connection can be automatically re-opened: **[Closing connections](#closing-connections)**.

Error object is returned as Promise rejection or first callback parameter (callback pattern).

When error is related to RFM parameter field, the `rfmPath` object is attached to error object, describing the affected field location:

```node
const good = { RFCINT1: 1 };
const BAD = { RFCINT1: "1" };

client.invoke(
    "STFC_STRUCTURE",
    {
        RFCTABLE: [good, good, good, BAD, good],
    },
    (err, res) => {
        if (err) console.error("error:", err);
        //error: {
        //  name: 'nodeRfcError',
        //  message: 'Integer number expected from NodeJS for the field of type 10',
        //  rfmPath: {
        //    rfm: 'STFC_STRUCTURE',
        //    parameter: 'RFCTABLE',
        //    table: 'RFCTABLE',
        //    table_line: 3,
        //    field: 'RFCINT1'
        //  }
        //}
    }
);
```

#### Error types, codes, groups, and classes

[Schmidt and Li (2009a)](http://sap.github.io/PyRFC/bibliography.html#c09a) describe five error types
on the basis of the return code (i.e. error code), returned from NWRFC SDK:

-  Communication failure
-  Logon failure
-  System failure
-  ABAP exception
-  ABAP messages

There are in total roughly 30 possible return codes that indicate some kind of error. Error information returned
from NWRFC SDK provides an `error group`, taken as the basis for two exception classes, shown in a below table.
The `node-rfc` is using one exception class and exception class names are exposed as error object `name` property:

| type (SPJ)            | code [numeric](C)             | group (C)                | Error Object `name` |
| --------------------- | ----------------------------- | ------------------------ | ------------------- |
| Communication failure | [1] RFC_COMMUNICATION_FAILURE | COMMUNICATION_FAILURE    | RfcLibError         |
| Logon failure         | [2] RFC_LOGON_FAILURE         | LOGON_FAILURE            | RfcLibError         |
| System failure        | [3] RFC_ABAP_RUNTIME_FAILURE  | ABAP_RUNTIME_FAILURE     | AbapError           |
| ABAP message          | [4] RFC_ABAP_MESSAGE          | ABAP_RUNTIME_FAILURE     | AbapError           |
| ABAP exception        | [5] RFC_ABAP_EXCEPTION        | ABAP_APPLICATION_FAILURE | AbapError           |
| node-rfc error        |                               |                          | nodeRfcError        |

### Invoction patterns

After getting a direct or managed client instance, the ABAP function module can be consumed from NodeJS using Async/await, Promise or callback pattern.

#### Async/await

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

#### Promise

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

#### Callback

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

## Connection Pool

API: [api/pool](api.md#connection-pool)

Connection Pool provides managed clients, which can't close or open their own connections. Their access to [`close()`](api.md#close) and [`open()`](api.md#open) methods is disabled and only the pool can open and close connections. The [`connectionHandle`](api.md#getters) of the managed client is therefore constant, changed only after critical errors, leading to connection close ( see [Closing connections](#closing-connections)). The managed client acquires an open connection from the Connnection Pool, using [`acquire()`](api.md#acquire) method and after no more needed, returns it back to pool, using [`release()`](api.md#release-1) method. After getting the connection back, the Connection Pool can reset the context and keep it open, ready for the next client, or close the connection. If the number of ready connections is less than pool `high` threshold parameter, the returned connection is added to ready connections, otherwise closed. The `low` threshold parameters defines a minimum number of connections, the Pool should keep open, ready for clients:

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

### Pool Options

`low` is the minimum number of connections to keep open, accelerating client `acquire()` requests. **Default**: `2`.

`high` is the maximum number of connections to keep open, "recycling" returned client connections. **Default**: `4`.

## Closing connections

The direct connection is closed by calling the client [`close()`](api.md#close) method or automatically, by client destructor.

The managed connection is returned to Connection Pool, using [`release()`](api.md#release-1) method and can be closed or reused by Connection Pool.

Connection Pool ready and leased connections are closed by Pool destructor.

The direct and managed connection can be automatically closed, if critical error occurs during the communication with ABAP backend system.
The client [`connectionHandle`](api.md#getters) property is set to NULL and `alive` to `false`. Critical error conditions,
leading to connection close, are exposed in `errorInfo` object.

If any of errors below occur, the client connection is not considered useful any more. The connection is automatically re-opened and
in case re-open error, that error is returned as a result. The new connection handle is assigned to the client and, if the client
is managed, the pool leased connections set is updated.

| errorInfo | Value                     | Description                                                                                                 |
| --------- | ------------------------- | ----------------------------------------------------------------------------------------------------------- |
| code      | RFC_COMMUNICATION_FAILURE | Error in Network & Communication layer.                                                                     |
| code      | RFC_ABAP_RUNTIME_FAILURE  | SAP system runtime error (SYSTEM_FAILURE): Shortdump on the backend side.                                   |
| code      | RFC_ABAP_MESSAGE          | The called function module raised an E-, A- or X-Message.                                                   |
| code      | RFC_EXTERNAL_FAILURE      | Error in external custom code. (E.g. in the function handlers or tRFC handlers.) Results in SYSTEM_FAILURE. |
| group     | ABAP_RUNTIME_FAILURE      | ABAP Message raised in ABAP function modules or in ABAP runtime of the backend (e.g Kernel)                 |
| group     | LOGON_FAILURE             | Error message raised when logon fails                                                                       |
| group     | COMMUNICATION_FAILURE     | Problems with the network connection (or backend broke down and killed the connection)                      |
| group     | EXTERNAL_RUNTIME_FAILURE  | Problems in the RFC runtime of the external program (i.e "this" library)                                    |

<a name="server"></a>

## Server (experimental)

API: [api/server](api.md#server)

Using the node-rfc Server instance, NodeJS functions can be exposed and consumed from ABAP, using `CALL FUNCTION DESTINATION` ABAP statement. Just the same way like standard RFMs from ABAP systems.

To expose one NodeJS function for ABAP clients, the ABAP function definition must be provided, defining ABAP parameters through which the ABAP client can invoke the NodeJS function. When ABAP clients calls the NodeJS function, ABAP input parameters are automatically converted to NodeJS function input and the function is invoked. The function output is automatically converted to ABAP output parameters, returned back to ABAP client.

ABAP function definition defines ABAP parameters and data definitions for all variables, structures and tables used in ABAP parameers. Coding these metadata down to field level is not very exciting task and node-rfc Server provides a more elegant solution here:

-  Create an empty ABAP RFM in ABAP client system, with the same parameters that ABAP client shall use to invoke the NodeJS function
-  Tell the node server that the function definition for NodeJS function X shall be the taken from the ABAP RFM Y, in ABAP client system

When invoked by ABAP, the NodeJS function will automatically fetch the function definition from ABAP RFM and expose itself as exactly such RFM to ABAP

As an example, let make the `STFC_CONNECTION` available as NodeJS RFM and call it from ABAP.

### Function Definition

No work needed because the `STFC_CONNECTION` RFM is already available in ABAP client system. Let us have a look into signature. The function module accepts the `REQTEXT` input stirng and echoes it back as `ECHOTEXT` string. In addition, the `RESPTEXT` string with connection attributes is also sent back:

```abap
FUNCTION STFC_CONNECTION.
*"----------------------------------------------------------------------
*"*"Lokale Schnittstelle:
*"       IMPORTING
*"              REQUTEXT LIKE  SY-LISEL
*"       EXPORTING
*"              ECHOTEXT LIKE  SY-LISEL
*"              RESPTEXT LIKE  SY-LISEL
*"----------------------------------------------------------------------
```

### NodeJS function

Let provide the NodeJS function, mimicking the ABAP `STFC_CONNECTION` logic. Of course any other logic can be implemented here.

```node
function my_stfc_connection(
    request_context,
    abap_parameters: { REQUTEXT: "" }
) {
    console.log("NodeJS stfc invoked ", request_context);

    return {
        ECHOTEXT: abap_parameters.REQUTEXT,
        RESPTEXT: `Python server here. Connection attributes are:\nUser '${request_context.user}' from system '${request_context.sysId}', client '${request_context.client}', host '${request_context.partnerHost}'`,
    };
}
```

When invoked from ABAP, the first argument `request_context` provides some information about ABAP consumer and the second argument `abap_parameters` provides input parameters sent from ABAP.

### ABAP calls NodeJS RFM

ABAP call looks like this:

```abap
*&---------------------------------------------------------------------*
*& Report ZNODETEST
*&---------------------------------------------------------------------*
*&
*&---------------------------------------------------------------------*
REPORT znodetest.

DATA lv_echo LIKE sy-lisel.
DATA lv_resp LIKE sy-lisel.

CALL FUNCTION 'STFC_CONNECTION' DESTINATION 'NODEJS'
  EXPORTING
    requtext = 'Hello Nöde'
  IMPORTING
    echotext = lv_echo
    resptext = lv_resp.

WRITE lv_echo.
WRITE lv_resp.
```

### Configuration

The remote destination configuration is described in chapter "5 RFC Server Programs" of **[SAP NWRFC SDK 7.50 Programming Guide](https://support.sap.com/content/dam/support/en_us/library/ssp/products/connectors/nwrfcsdk/NW_RFC_750_ProgrammingGuide.pdf)**

For this particular example, the configuration includes:

**sapnwrfc.ini**

The `client` connection parameters are required for RFM function definition retrival, just like in Client scenario.

The `gateway` parameters are for the NodeJS server regitration in ABAP system.

```ini
DEST=client
USER=demo
PASSWD=welcome
ASHOST=coevi51
SYSNR=00
CLIENT=620
LANG=EN
#TRACE=3

DEST=gateway
GWSERV=sapgw00
GWHOST=coevi51
PROGRAM_ID=SERVER1
REG_COUNT=1
```

**SM59**

The NodeJS destination is in SM59 look like

![](assets/sm59node.png)

**ABAP**

```abap
*&---------------------------------------------------------------------*
*& Report ZSERVERTEST
*&---------------------------------------------------------------------*
*&
*&---------------------------------------------------------------------*
REPORT zservertest.


DATA lv_echo LIKE sy-lisel.
DATA lv_resp LIKE sy-lisel.

CALL FUNCTION 'STFC_CONNECTION' DESTINATION 'NODEJS'
  EXPORTING
    requtext = 'XYZ'
  IMPORTING
    echotext = lv_echo
    resptext = lv_resp.

WRITE lv_echo.
WRITE lv_resp.
```

**Node Server**

```node
const addon = require("../../lib");
const Server = addon.Server;
const server = new Server({ dest: "gateway" }, { dest: "MME" });

// Callback function
function my_stfc_connection(request_context, REQUTEXT = "") {
    console.log("stfc invoked");
    console.log("request_context", request_context);
    console.log("abap_parameters", abap_parameters);

    return {
        ECHOTEXT: REQUTEXT,
        RESPTEXT: `Node server here. Connection attributes are:\nUser '${request_context.user}' from system '${request_context.sysId}', client '${request_context.client}', host '${request_context.partnerHost}'`,
    };
}

server.start((err) => {
    if (err) return console.error("error:", err);
    console.log(
        `Server alive: ${server.alive} client handle: ${server.client_connection} server handle: ${server.server_connection}`
    );

    // Expose the my_stfc_connection function as RFM with STFC_CONNECTION pararameters (function definition)
    const RFM_NAME = "STFC_CONNECTION";
    server.addFunction(RFM_NAME, my_stfc_connection, (err) => {
        if (err) return console.error(`error adding ${RFM_NAME}: ${err}`);
    });
});

// let server serve
setTimeout(function () {
    console.log("bye!");
}, 20 * 1000);
```

## Throughput

API: [api/throughput](api.md#throughput)

The Throughput object can be attached to one or more clients, providing the RFM communication monitoring:

-  number of calls
-  sent bytes
-  received bytes
-  application time
-  total time
-  serialization time
-  deserialization time

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

- **[addon](#addon)**
  - [setIniFileDirectory](#setinifiledirectory)
  - [loadCryptoLibrary](#loadcryptolibrary)
  - [cancelClient](#cancelclient)
- **[Client](#client)**
  - [Properties](#client-properties)
  - [Constructor](#client-constructor)
  - [API](#client-api)
- **[Connection Pool](#connection-pool)**
  - [Properties](#pool-properties)
  - [Constructor](#pool-constructor)
  - [API](#pool-api)
- **[Server](#server)**
  - [Properties](#server-properties)
  - [Constructor](#server-constructor)
  - [API](#server-api)
- **[Throughput](#throughput)**
  - [Properties](#throughput-properties)
  - [Constructor](#throughput-constructor)
  - [API](#throughput-api)

## Addon

### setIniFileDirectory

Usage: [usage/addon](usage.md#addon)

```ts
setIniFileDirectory(iniFileDirectory: string)
```

### reloadIniFile

Usage: [usage/addon](usage.md#addon)

```ts
reloadIniFile()
```

### loadCryptoLibrary

Usage: [usage/addon](usage.md#addon)

```ts
loadCryptoLibrary(cryptoLibraryPath: string)
```

### cancelClient

Usage: [usage/addon](usage.md#cancelclient)

```ts
cancelClient(client: Client, callback?: Function): void | Promise<void>;
```

### languageIsoToSap

Usage: [usage/addon](usage.md#addon)

```ts
languageIsoToSap(langIsoCode: string): string|Error
```

### languageSapToIso

Usage: [usage/addon](usage.md#addon)

```ts
languageSapToIso(langSapCode: string): string|Error
```

### setLogFilePath

Usage: [usage/addon](usage.md#addon)

```ts
setLogFilePath(langSapCode: string): string|Error
```

## Client

Usage: [usage/client](usage.md#client)

<a name="client-properties"></a>

### Properties

[`environment`](./usage.md#environment) : Object, exposed at instance and class level

`binding` : Object, the Client binding object

`id` : Number, the client instance id

`_id` : String, "extended" client instance id, with connection handle, pool id (if managed) and "direct"/"managed" attribute

`config`: Object, Client configuration, exposing `connectionParameters` and `clientOptions` objects

`connectionHandle`: Number, the client connection handle

`alive`: Boolean, set to `true` if connection handle is not NULL (connection open)

`pool_id`: Number, set to non-zero value in managed clients

`connectionInfo`: Object exposing RFC Connection attributes, or Error object if the connection is closed

<a name="client-constructor"></a>

### Constructor

The direct client is instantiated using connection parameters and optional client options:

```ts
constructor(
    connectionParameters: RfcConnectionParameters,
    clientOptions?: RfcClientOptions )
```

Another constructor is used by the Connection Pool only. The client instance is already created by Connection Pool
and passed to Node.js for Node.js Client instance creation:

```ts
constructor(
    clientBinding: RfcClientBinding,
    clientOptions?: RfcClientOptions )
```

Example:

```node
const Client = require("node-rfc").Client;

const direct_client = new Client({ dest: "QI3" }, { stateless: true });
```

The managed client is instantiated using the Connection Pool [`acquire()`](#acquire) method.

### Client API

Client API methods accept optional callback argument, for callback invocation pattern. When callback not provided, the Promise is returned.

#### setIniPath

Sets the directory in which to search for the `sapnwrfc.ini` file.

By default the sapnwrfc library searches for the `sapnwrfc.ini` in the current working directory of the process. If you want to keep it in a different directory, use this function to tell the sapnwrfc library about the new path.

After you have changed the directory, the NW RFC lib automatically loads the contents of the new `sapnwrfc.ini` file from that directory.

```ts
setIniPath(pathName: string, callback?: Function): void | Promise<void>
```

#### open

Open connection (direct client only):

```ts
connect(callback?: Function): void | Promise<Client> // for backwards compatibility, to be deprecated
open(callback?: Function): void | Promise<Client>
```

#### close

Close connection (direct client only):

```ts
close(callback?: Function): void | Promise<void>
```

#### cancel

Cancel connection:

```ts
cancel(callback?: Function): void | Promise<any>
```

#### ping

RFC ping the ABAP backend system, returning:

- boolean `true` when successful
- boolean `false` and RFC error object, when not
- Error object when the connection is already closed

```ts
ping(callback?: Function): void | Promise<boolean>
```

#### resetServerContext

Reset server context, making the next call ABAP stateless, like after opening new connection:

```ts
resetServerContext(callback?: Function): void | Promise<void>
```

#### release

Release the client connection back to Pool (managed client only). No arguments required:

```ts
release(callback?: Function): void | Promise<void>
```

#### call

Invoke the ABAP RFM `rfmName`, providing RFM parameters `rfmParams` and optional callOptions:

```ts
call(
    rfmName: string,
    rfmParams: RfcObject,
    callOptions: RfcClientOptions = {}
): Promise<RfcObject>
```

#### invoke

The same as call, used in callback pattern:

```ts
invoke(
    rfmName: string,
    rfmParams: RfcObject,
    callback: Function,
    callOptions?: object
)
```

## Connection Pool

Usage: [usage/connection-pool](usage.md#connection-pool)

<a name="pool-properties"></a>

### Properties

[`environment`](./usage.md#environment) : Object, exposed at instance and class level

`binding` : Object, the Pool binding object

`id` : Number, the client instance id

`config`: Object, exposing Pool configuration, with `connectionParameters` `clientOptions` and `poolOptions` objects, which are available also as direct getters.

`status` : Object, exposing the number of `ready` and `leased` connections

<a name="pool-constructor"></a>

### Constructor

```ts
export interface RfcPoolConfiguration {
    connectionParameters: RfcConnectionParameters;
    clientOptions?: RfcClientOptions;
    poolOptions?: RfcPoolOptions;
}

constructor(poolConfiguration: RfcPoolConfiguration)
```

Example:

```node
const Pool = require("node-rfc").Pool;
const pool = new Pool({
    connectionParameters: { dest: "MME" },
    clientOptions: { filter: 2, bcd: "number" },
    poolOptions: { low: 0, high: 4 },
});
```

### Pool API

The result is returned as a promise, unless the optional callback argument provided.

#### acquire

Acquire one or more managed clients, each with own open connection.

```ts
acquire(callback?:Function)  // Acquire 1 client
acquire(1, callback?:Function) // Acquire 1 client
acquire(3, callback?:Function) // Acquire Array of 3 clients
```

Managed client:

```node
const Pool = require("node-rfc").Pool;
const pool = new Pool({
        connectionParameters: { dest: "QI3},
        clientOptions: {stateless: true},   // optional
        poolOptions: {low: 0, high: 1}  // optional
    });

pool.acquire().then(managed_client => {
    console.log(managed_client.alive); // true
});
```

#### release

Release connections of one or more managed clients.

```ts
release(client1, callback?:Function)  // Release 1 client
acquire([client1], callback?:Function) // Release 1 client
acquire([client1, client2], callback?:Function) // Release Array of 2 clients
```

#### cancel

Cancel connection:

```ts
cancel(client, callback?: Function): void | Promise<any>
```

#### ready

Check if the number of ready connections is below the pool `ready_low` and open new connections if needed.

Optionally, the number connections can be provided, to be used instead of the `ready_low`.

```ts
pool.ready(); // check if pool ready_low connections are ready
pool.ready(5); // check if 5 connections are ready
pool.ready(5, callback); // check if 5 connections are ready and call the callback when they are
pool.ready(callback, 5); // check if 5 connections are ready and call the callback when they are
```

```ts
release(client1, callback?:Function)  // Release 1 client
acquire([client1], callback?:Function) // Release 1 client
acquire([client1, client2], callback?:Function) // Release Array of 2 clients
```

#### closeAll

:exclamation_mark: Internal use only.

Close all open connections, ready and leased. Not synchronized with client or pool mutexes, therefore not supported.

All open connections are anyway closed when Pool destructor called.

```ts
closeAll(callback?: Function) // close all ready and leased connections
```

<a name="server"></a>

## Server

Usage: [usage/server](usage.md#server)

<a name="server-properties"></a>

### Properites

<a name="server-constructor"></a>

### Constructor

```ts

export type RfcServerConfiguration = {
    serverConnection: RfcConnectionParameters;
    clientConnection: RfcConnectionParameters;
    serverOptions?: RfcServerOptions;
};

export interface RfcServerBinding {
    new (serverConfiguration: RfcServerConfiguration): RfcServerBinding;
}
```

<a name="server-api"></a>

### Server API

#### start

```ts
 start(callback?: Function): void | Promise<void>
```

Launch Server instance.

#### stop

```ts
stop(callback?: Function): void | Promise<void>
```

Stop Server instance.

### addFunction

```ts
    addFunction(
        abapFunctionName: string,
        jsFunction: Function,
        callback?: Function
    ): void | Promise<void>
```

Register JavaScript function as ABAP function on Node.js server.

### removeFunction

```ts
removeFunction(
    abapFunctionName: string,
    callback?: Function
): void | Promise<void>
```

Un-register JavaScript function on Node.js server.

## Throughput

Usage: [usage/throughput](usage.md#throughput)

<a name="throughput-properties"></a>

### Properties

`status` providing the monitoring statistics:

```ts
export interface RfcThroughputStatus {
    numberOfCalls: number;
    sentBytes: number;
    receivedBytes: number;
    applicationTime: number;
    totalTime: number;
    serializationTime: number;
    deserializationTime: number;
}
```

`clients` exposing a Set of monitored clients

`handle` exposing the internal Throughput object handle, assigned by SAP NWRFC SDK

<a name="throughput-constructor"></a>

### Constructor

```ts
constructor(client?: Client | Array<Client>)
```

<a name="throughput-api"></a>

### Throughput API

#### setOnConnection

Assign one or more clients to Throughput instance:

```ts
setOnConnection(client: Client | Array<Client>)
```

Example:

```node
throughput.setOnConnection([client1, client2]);
```

#### removeFromConnection

The opposite of `setOnConnection`:

```ts
removeFromConnection(client: Client | Array<Client>)
```

#### reset

Reset Throughput counters:

```ts
reset();
```

#### destroy

Free the Throughput object, normally not needed becuse called in a destructor:

```ts
destroy();
```

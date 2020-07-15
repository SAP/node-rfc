# Client

Usage: [usage/connection-pool](https://github.wdf.sap.corp/D037732/noderfc2/blob/master/doc/usage.md#direct-and-managed-clients)

## Getters

[`environment`](./usage.md#environment) : Object, exposed at instance and class level

`binding` : Object, the Client binding object

`id` : Number, the client instance id

`_id` : String, "extended" client instance id, with connection handle, pool id (if managed) and "direct"/"managed" attribute

`config`: Object, Client configuration, exposing `connectionParameters` and `clientOptions` objects

`connectionHandle`: Number, the client connection handle

`alive`: Boolean, set to `true` if connection handle is not NULL (connection open)

`pool_id`: Number, set to non-zero value in managed clients

`connectionInfo`: Object exposing RFC Connection attributes, or Error object if the connection is closed

## Constructor

### Direct connection

```ts

interface RfcClientBinding {
    new (
        connectionParameters: RfcConnectionParameters,
        clientOptions?: RfcClientOptions
    ): RfcClientBinding;
```

Example:

```node
const Client = require("node-rfc").Client;

const client = new Client({ dest: "QI3" }, { stateless: true });
```

### Managed connection

```ts
interface RfcPoolConfiguration {
    connectionParameters: RfcConnectionParameters;
    clientOptions?: RfcClientOptions;
    poolOptions?: RfcPoolOptions;
}

interface RfcPoolBinding {
    new (poolConfiguration: RfcPoolConfiguration): RfcPoolBinding;
```

Example:

```node
const Pool = require("node-rfc").Pool;
const pool = new Pool({
        connectionParameters: { dest: "QI3},
        clientOptions: {stateless: true},   // optional
        poolOptions: {low: 0, high: 1}  // optional
    });
```

## Client API

Client API methods accept optional callback argument, for callback invocation pattern. When callback not provided, the Promise is returned.

### open

Open connection (direct client only):

```ts
connect(callback?: Function): void | Promise<Client> // for backwards compatibility, to be deprecated
open(callback?: Function): void | Promise<Client>
```

### close

Close connection (direct client only):

```ts
close(callback?: Function): void | Promise<void>
```

### ping

RFC ping the ABAP backend system, returning:

-   boolean `true` when successful
-   boolean `false` and RFC error object, when not
-   Error object when the connection is already closed

```ts
ping(callback?: Function): void | Promise<boolean>
```

### resetServerContext

Reset server context, making the next call ABAP stateless, like after opening new connection:

```ts
resetServerContext(callback?: Function): void | Promise<void>
```

### release

Release the client connection back to Pool (managed client only). No arguments required:

```ts
release(callback?: Function): void | Promise<void>
```

### call

Invoke the ABAP RFM `rfmName`, providing RFM parameters `rfmParams` and optional callOptions:

```ts
call(
    rfmName: string,
    rfmParams: RfcObject,
    callOptions: RfcClientOptions = {}
): Promise<RfcObject>
```

### invoke

The same as call, used in callback pattern:

```ts
invoke(
    rfmName: string,
    rfmParams: RfcObject,
    callback: Function,
    callOptions?: object
)
```

# Pool

Usage: [usage/connection-pool](https://github.wdf.sap.corp/D037732/noderfc2/blob/master/doc/usage.md#connection-pool)

## Getters

[`environment`](./usage.md#environment) : Object, exposed at instance and class level

`binding` : Object, the Pool binding object

`id` : Number, the client instance id

`config`: Object, exposing Pool configuration, with `connectionParameters` `clientOptions` and `poolOptions` objects, which are available also as direct getters.

`status` : Object, exposing the number of `ready` and `leased` connections

## Constructor

```ts
export interface RfcPoolConfiguration {
    connectionParameters: RfcConnectionParameters;
    clientOptions?: RfcClientOptions;
    poolOptions?: RfcPoolOptions;
}
export interface RfcPoolBinding {
    new (poolConfiguration: RfcPoolConfiguration): RfcPoolBinding;
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

## API

The result is returned as a promise, unless the optional callback argument provided.

### acquire

Acquire one or more managed clients, each with own open connection.

```ts
acquire(callback?:Function)  // Acquire 1 client
acquire(1, callback?:Function) // Acquire 1 client
acquire(3, callback?:Function) // Acquire Array of 3 clients
```

### release

Release connections of one or more managed clients.

```ts
release(client1, callback?:Function)  // Release 1 client
acquire([client1], callback?:Function) // Release 1 client
acquire([client1, client2], callback?:Function) // Release Array of 2 clients
```

### ready

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

### closeAll

:exclamation_mark: Internal use only.

Close all open connections, ready and leased. Not synchronized with client or pool mutexes, therefore not supported.

All open connections are anyway closed when Pool destructor called.

```ts
closeAll(callback?: Function) // close all ready and leased connections
```

# Throughput

Usage: [usage/connection-pool](https://github.wdf.sap.corp/D037732/noderfc2/blob/master/doc/usage.md#throughput)

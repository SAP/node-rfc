// SPDX-FileCopyrightText: 2014 SAP SE Srdjan Boskovic <srdjan.boskovic@sap.com>
//
// SPDX-License-Identifier: Apache-2.0

"use strict";

describe("Throughput: Client", () => {
    const setup = require("../utils/setup");
    const Throughput = setup.Throughput;
    const client = setup.direct_client();
    const throughput = new Throughput();

    test("Throughput set on closed client", function (done) {
        expect(() => throughput.setOnConnection(client)).toThrow(
            new Error(`Throughput can't be set on closed client: ${client.id}`)
        );
        expect(throughput.clients.size).toEqual(0);
        done();
    });

    test("Throughput set on invalid client", function (done) {
        expect(() => throughput.setOnConnection(1)).toThrow(
            new Error(
                "Client instance or array of Client instances required as argument"
            )
        );
        expect(throughput.clients.size).toEqual(0);
        done();
    });

    test("Throughput removal from closed client", function (done) {
        expect(() => throughput.removeFromConnection(client)).not.toThrow();
        done();
    });

    test("Throughput get from closed client", function (done) {
        expect(Throughput.getFromConnection(client)).toBeUndefined();
        done();
    });

    test("Throughput initial status", function (done) {
        expect(throughput.status).toEqual(
            expect.objectContaining({
                numberOfCalls: 0,
                sentBytes: 0,
                receivedBytes: 0,
                applicationTime: 0,
                totalTime: 0,
                serializationTime: 0,
                deserializationTime: 0,
            })
        );
        done();
    });

    test("Throughput set/remove connection", function () {
        return (async () => {
            await client.open();

            throughput.setOnConnection(client);
            expect(throughput.clients.size).toEqual(1);

            throughput.removeFromConnection(client);
            expect(throughput.clients.size).toEqual(0);

            await client.close();
        })();
    });

    test("Throughput get from connection", function () {
        return (async () => {
            await client.open();

            throughput.setOnConnection(client);
            expect(throughput.clients.size).toEqual(1);

            let throughput1 = Throughput.getFromConnection(client);
            expect(throughput._handle).toEqual(throughput1._handle);

            throughput.removeFromConnection(client);
            expect(throughput.clients.size).toEqual(0);

            await client.close();
        })();
    });

    test("Throughput single connection", function () {
        return (async () => {
            await client.open();

            throughput.setOnConnection(client);
            expect(throughput.clients.size).toEqual(1);
            expect(throughput.status).toMatchObject({
                numberOfCalls: 0,
                sentBytes: 0,
                receivedBytes: 0,
                applicationTime: 0,
                totalTime: 0,
                serializationTime: 0,
                deserializationTime: 0,
            });

            await client.call("STFC_CONNECTION", {
                REQUTEXT: "hello",
            });
            expect(throughput.status).toMatchObject({
                numberOfCalls: 2,
                sentBytes: 1089,
                receivedBytes: 2812,
            });

            await client.call("STFC_CONNECTION", {
                REQUTEXT: "hello",
            });
            expect(throughput.status).toMatchObject({
                numberOfCalls: 3,
                sentBytes: 1737,
                receivedBytes: 4022,
            });

            throughput.reset();
            expect(throughput.status).toMatchObject({
                numberOfCalls: 0,
                sentBytes: 0,
                receivedBytes: 0,
                applicationTime: 0,
                totalTime: 0,
                serializationTime: 0,
                deserializationTime: 0,
            });

            await client.call("BAPI_USER_GET_DETAIL", {
                USERNAME: "DEMO",
            });
            expect(throughput.status).toMatchObject({
                numberOfCalls: 87,
                sentBytes: 64968,
                receivedBytes: 393716,
            });

            throughput.removeFromConnection(client);
            expect(throughput.clients.size).toEqual(0);

            await client.close();
        })();
    }, 10000);

    test("Throughput multiple connection", function () {
        return (async () => {
            const client1 = setup.direct_client();
            const client2 = setup.direct_client();

            await client1.open();
            await client2.open();

            throughput.reset();

            // set two connections
            throughput.setOnConnection([client1, client2]);
            expect(throughput.clients.size).toEqual(2);

            // remove two connections
            throughput.removeFromConnection([client1, client2]);
            expect(throughput.clients.size).toEqual(0);

            // create with multiple connections
            let throughput1 = new Throughput([client1, client2]);
            expect(throughput1.clients.size).toEqual(2);

            await client1.close();
            await client2.close();
        })();
    });
});

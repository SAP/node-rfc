// SPDX-FileCopyrightText: 2014 SAP SE Srdjan Boskovic <srdjan.boskovic@sap.com>
//
// SPDX-License-Identifier: Apache-2.0

import { direct_client, Client, Throughput } from "../utils/setup";

describe("Throughput: Client", () => {
    const client = direct_client();
    const throughput = new Throughput();

    test("Throughput set on closed client", function (done) {
        expect(() => throughput.setOnConnection(client)).toThrow(
            new Error(`Throughput can't be set on closed client: ${client.id}`)
        );
        expect(throughput.clients.size).toEqual(0);
        done();
    });

    test("Throughput set on invalid client", function (done) {
        expect(() => throughput.setOnConnection({} as Client)).toThrow(
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

    test("Throughput set/remove connection", async () => {
        expect.assertions(2);

        await client.open();

        throughput.setOnConnection(client);
        expect(throughput.clients.size).toEqual(1);

        throughput.removeFromConnection(client);
        expect(throughput.clients.size).toEqual(0);

        await client.close();
    });

    test("Throughput get from connection", async () => {
        expect.assertions(3);
        await client.open();

        throughput.setOnConnection(client);
        expect(throughput.clients.size).toEqual(1);

        const throughput1 = Throughput.getFromConnection(client) as Throughput;
        expect(throughput.handle).toEqual(throughput1.handle);

        throughput.removeFromConnection(client);
        expect(throughput.clients.size).toEqual(0);

        await client.close();
    });

    test("Throughput single connection", async () => {
        expect.assertions(13);

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

        expect(throughput.status.numberOfCalls).toEqual(2);
        expect(throughput.status.sentBytes).toBeGreaterThan(1000);
        expect(throughput.status.receivedBytes).toBeGreaterThan(2000);

        await client.call("STFC_CONNECTION", {
            REQUTEXT: "hello",
        });

        expect(throughput.status.numberOfCalls).toEqual(3);
        expect(throughput.status.sentBytes).toBeGreaterThan(1000);
        expect(throughput.status.receivedBytes).toBeGreaterThan(3000);

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

        expect(throughput.status.numberOfCalls).toEqual(87);
        expect(throughput.status.sentBytes).toBeGreaterThan(60000);
        expect(throughput.status.receivedBytes).toBeGreaterThan(300000);

        throughput.removeFromConnection(client);
        expect(throughput.clients.size).toEqual(0);

        await client.close();
    }, 10000);

    test("Throughput multiple connection", async () => {
        expect.assertions(3);
        const client1 = direct_client();
        const client2 = direct_client();

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
        const throughput1 = new Throughput([client1, client2]);
        expect(throughput1.clients.size).toEqual(2);

        await client1.close();
        await client2.close();
    });
});

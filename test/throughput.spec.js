// Copyright 2014 SAP AG.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http: //www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific
// language governing permissions and limitations under the License.

"use strict";

const Client = require("./setup").rfcClient;
const Throughput = require("./setup").rfcThroughput;
const abapSystem = require("./setup").abapSystem;

const client = new Client(abapSystem);
const throughput = new Throughput();

describe('Throughput', () => {

    it("Throughput set on closed client", function (done) {
        expect(() => throughput.setOnConnection(client)).toThrow(
            new Error("Throughput can't be set on closed client: 5")
        )
        expect(throughput.clients.size).toEqual(0);
        done();
    });

    it("Throughput set on invalid client", function (done) {
        expect(() => throughput.setOnConnection(1)).toThrow(
            new Error("Single Client instance or array of Client instances required as argument")
        )
        expect(throughput.clients.size).toEqual(0);
        done();
    });

    it("Throughput removal from closed client", function (done) {
        expect(() => throughput.removeFromConnection(client)).not.toThrow();
        done();
    });

    it("Throughput get from closed client", function (done) {
        expect(Throughput.getFromConnection(client)).toBeUndefined();
        done();
    });

    it("Throughput initial status", function (done) {
        expect(throughput.status).toEqual(
            expect.objectContaining({
                numberOfCalls: 0,
                sentBytes: 0,
                receivedBytes: 0,
                applicationTime: 0,
                totalTime: 0,
                serializationTime: 0,
                deserializationTime: 0
            })
        );
        done();
    });

    it("Throughput set/remove connection", function () {
        return (async () => {
            await client.open();

            throughput.setOnConnection(client);
            expect(throughput.clients.size).toEqual(1);

            throughput.removeFromConnection(client);
            expect(throughput.clients.size).toEqual(0);

            await client.close();
        })();
    });

    it("Throughput get from connection", function () {
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

    it("Throughput trace single connection", function () {
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
                deserializationTime: 0
            });

            await client.call('STFC_CONNECTION', {
                REQUTEXT: 'hello'
            });
            expect(throughput.status).toMatchObject({
                "numberOfCalls": 2,
                "sentBytes": 1089,
                "receivedBytes": 2812,
            });

            await client.call('STFC_CONNECTION', {
                REQUTEXT: 'hello'
            });
            expect(throughput.status).toMatchObject({
                "numberOfCalls": 3,
                "sentBytes": 1737,
                "receivedBytes": 4022,
            });

            throughput.reset();
            expect(throughput.status).toMatchObject({
                numberOfCalls: 0,
                sentBytes: 0,
                receivedBytes: 0,
                applicationTime: 0,
                totalTime: 0,
                serializationTime: 0,
                deserializationTime: 0
            });

            await client.call('BAPI_USER_GET_DETAIL', {
                USERNAME: 'DEMO'
            });
            expect(throughput.status).toMatchObject({
                "numberOfCalls": 87,
                "sentBytes": 64968,
                "receivedBytes": 393716,
            });

            throughput.removeFromConnection(client);
            expect(throughput.clients.size).toEqual(0);

            await client.close();
        })();
    });

    it("Throughput multiple connection", function () {
        return (async () => {
            const client1 = new Client(abapSystem);
            const client2 = new Client(abapSystem);

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

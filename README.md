Requirement:
Implement seeder servers which store and maintain information of nodes in the network. New nodes can fetch a list of nodes from seeder servers and choose which nodes they want to connect to.
Implement simple client to test seeder servers

Main features:
1. Receive queries from clients and returns a list of available nodes.

2. Receive "hello" message from other clients which want to introduce themselves to the Seeder. The seeder will add such clients to the peer list.

3. Frequently ping existing nodes inthe database to update their information(e.g. check whether they are alive, which clients they are using).

4. Support queries like: which nodes are alive during the last 1hours, 2hours, 1 day.


Prequisite for build:
    ubuntu18.04
    apt install cmake make g++ gcc automake libtool gdb valgrind libuv1-dev

Design for server:
    Data structure that contain information of status of a node
        if the latest ping result from server to node A is ok:
            {node A, {alive, start_time_of_consequence_successful_ping}}

        if the latest ping result from server to node A is ok:
            {node A, {dead, 0}}
    EX:
        7:00 -> first time receive "hello" from client
            {node A, {alive, 7:00}}
        7:01 -> ping success
            {node A, {alive, 7:00}}
        7:02 -> ping success
            {node A, {alive, 7:00}}
        7:03 -> ping fail
            {node A, {dead, 0}}
        7:04 -> ping fail
            {node A, {dead, 0}}
        7:05 -> ping ping success
            {node A, {alive, 7:05}}
        7:06 -> ping ping success
            {node A, {alive, 7:05}}

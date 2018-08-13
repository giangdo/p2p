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

    Or we can use my docker image:
        docker pull giangdo/ubuntu18.04

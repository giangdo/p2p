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
    apt install cmake make g++ gcc automake libtool
    apt install libuv1-dev
    apt install python3-minimal

Prequisite tool:
    apt install gdb valgrind

This source code use some

# Some reference to create http rest api:
rest api for c++
-> I won't use these library because it take time to understand
apt install libboost1.65-dev
git clone https://github.com/skypjack/uvw.git
https://github.com/Microsoft/cpprestsdk
https://github.com/oktal/pistache

database: https://unqlite.org/
          https://github.com/symisc/unqlite

Design

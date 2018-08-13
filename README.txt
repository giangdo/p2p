* Requirement:
Implement seeder servers which store and maintain information of nodes in the network. New nodes can fetch a list of nodes from seeder servers and choose which nodes they want to connect to.
Implement simple client to test seeder servers

Main features:
1. Receive queries from clients and returns a list of available nodes.

2. Receive "hello" message from other clients which want to introduce themselves to the Seeder. The seeder will add such clients to the peer list.

3. Frequently ping existing nodes inthe database to update their information(e.g. check whether they are alive, which clients they are using).

4. Support queries like: which nodes are alive during the last 1hours, 2hours, 1 day.


* Prequisite for build:
    ubuntu18.04
    apt install cmake make g++ gcc automake libtool gdb valgrind libuv1-dev

    Or we can use my docker image:
        docker pull giangdo/ubuntu18.04

* How to test:
    + Prepare docker environment:
        $docker network create giang_net
        $docker run -d -it --net=giang_net -v ~/p2p:/root/p2p --name m1 giangdo/ubuntu18.04
        $docker run -d -it --net=giang_net -v ~/p2p:/root/p2p --name m2 giangdo/ubuntu18.04
        $docker run -d -it --net=giang_net -v ~/p2p:/root/p2p --name m3 giangdo/ubuntu18.04
        $docker run -d -it --net=giang_net -v ~/p2p:/root/p2p --name m4 giangdo/ubuntu18.04

    + Build and Run server in m1:
        $docker exec -it m1 bash
        #cd ~/p2p/server
        #make
        #./server
        #ip addr show |grep inet
        inet 172.18.0.2/16 brd 172.18.255.255 scope global eth0

    + Build and Run Client in m2:
        $docker exec -it m2 bash
        #cd ~/p2p/client
        #make
        #./client -i 172.18.0.2 -p 9090 -c hello

    + Run Client in m3:
        $docker exec -it m3 bash
        #cd ~/p2p/client
        #./client -i 172.18.0.2 -p 9090 -c hello

     + List and Query in m4:
        $docker exec -it m4 bash
        #cd ~/p2p/client
        #./client -i 172.18.0.2 -p 9090 -c list
        [
          "172.18.0.3",
          "172.18.0.4"
        ]

        #./client -i 172.18.0.2 -p 9090 -c query -d 1 -h 1
        {
          "172.18.0.3": "Mon Aug 13 09:34:26 2018\n",
          "172.18.0.4": "Mon Aug 13 09:34:22 2018\n"
        }

    + Then stop client process in m2
    + Then run List and Query in m4 (Expect -> don't see client infor in m2)

        #./client -i 172.18.0.2 -p 9090 -c list
        [
          "172.18.0.3",
        ]

        #./client -i 172.18.0.2 -p 9090 -c query -d 1 -h 1
        {
          "172.18.0.3": "Mon Aug 13 09:34:26 2018\n",
        }

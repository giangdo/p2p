* reference:
    tcp server:
        curl -LO https://raw.githubusercontent.com/nikhilm/uvbook/master/code/tcp-echo-server/main.c
        https://stackoverflow.com/questions/13656702/sending-and-receiving-strings-over-tcp-socket-separately

* to build:
    $make

* to run:
    EX: ./server

* Design:
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

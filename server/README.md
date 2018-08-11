Thank Eli Bendersky for his amazing example of how to use libuv
Reference: https://github.com/eliben/code-for-blog/tree/master/2017/async-socket-server

Command to build:
    + use make:
        $cd p2p/server
        $make

    + or use long command:
        $g++ -std=c++11 -Wall -O3 -g -DNDEBUG -pthread utils.c main.cpp -o server -lpthread -pthread -luv -Wl,-rpath=/usr/local/lib -L/usr/local/include

* How to run:
    # ./server
    Serving on port 9090

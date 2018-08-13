* reference:
    tcp client:
        https://www.binarytides.com/code-a-simple-socket-client-class-in-c/

    parse parameter: (getopt)
        https://codeyarns.com/2015/01/30/how-to-parse-program-options-in-c-using-getopt_long/
        https://www.informit.com/articles/article.aspx?p=175771&seqNum=3

    json:
        curl -LO https://github.com/nlohmann/json/releases/download/v3.1.2/json.hpp
* to build:
    $make

* to run:
    EX: ./client -i 172.18.0.2 -p 9090 -c hello
    EX: ./client -i 172.18.0.2 -p 9090 -c list
    EX: ./client -i 172.18.0.2 -p 9090 -c query -d 1 -h 1


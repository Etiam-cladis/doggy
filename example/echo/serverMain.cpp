#ifndef EXAMPLE_ECHO_SERVERMAIN_CPP
#define EXAMPLE_ECHO_SERVERMAIN_CPP

#include "./echoServer.h"
#include "doggy/net/EventLoop.h"

using namespace doggy;
using namespace doggy::net;

// ip port numThreads
int main(int argc, char *argv[])
{
        EventLoop loop;
        char *ip = argv[1];
        uint16_t port = static_cast<uint16_t>(atoi(argv[2]));
        InetAddress listenAddr(ip, port);

        int numThreads = atoi(argv[3]);

        EchoServer server(&loop, listenAddr, numThreads);
        server.start();
        loop.loop();
}

#endif // EXAMPLE_ECHO_SERVERMAIN_CPP

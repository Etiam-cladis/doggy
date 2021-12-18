#ifndef EXAMPLE_ECHO_CLIENTMAIN_CPP
#define EXAMPLE_ECHO_CLIENTMAIN_CPP

#include "./echoClient.h"
#include "doggy/net/EventLoop.h"

using namespace doggy;
using namespace doggy::net;

// server ip
// server port
int main(int argc, char *argv[])
{
        EventLoop loop;
        InetAddress serverAddr(argv[1], static_cast<uint16_t>(atoi(argv[2])));

        EchoClient client(&loop, serverAddr, "here is echo");
        client.connect();
        loop.loop();
}

#endif // EXAMPLE_ECHO_CLIENTMAIN_CPP

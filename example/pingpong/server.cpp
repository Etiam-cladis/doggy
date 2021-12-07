#include "doggy/net/header.h"

#include "doggy/net/TcpServer.h"

#include "doggy/net/EventLoop.h"
#include "doggy/net/InetAddress.h"

#include <utility>

#include <stdio.h>
#include <unistd.h>

using namespace doggy;
using namespace doggy::net;

using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

void onConnection(const TcpConnectionPtr &conn)
{
        if (conn->connected())
        {
                conn->setTcpNoDelay(true);
        }
}

void onMessage(const TcpConnectionPtr &conn, Buff &buf)
{
        conn->send(buf);
}

int main(int argc, char *argv[])
{
        if (argc < 4)
        {
                fprintf(stderr, "Usage: server <address> <port> <threads>\n");
        }
        else
        {
                const char *ip = argv[1];
                uint16_t port = static_cast<uint16_t>(atoi(argv[2]));
                InetAddress listenAddr(ip, port);
                int threadCount = atoi(argv[3]);

                EventLoop loop;

                TcpServer server(&loop, listenAddr);

                server.setConnectionCallback(onConnection);
                server.setMessageCallback(onMessage);

                if (threadCount > 1)
                {
                        server.setNumThreads(threadCount);
                }

                server.start();

                loop.loop();
        }
}

// Benchmark inspired by libevent/test/bench.c
// See also: http://libev.schmorp.de/bench.html

#include "doggy/net/header.h"

#include "doggy/net/Channel.h"
#include "doggy/net/EventLoop.h"

#include <stdio.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <unistd.h>

using namespace doggy;
using namespace doggy::net;

using Timestamp = std::chrono::time_point<std::chrono::system_clock, std::chrono::microseconds>;

std::vector<int> g_pipes;
int numPipes;
int numActive;
int numWrites;
EventLoop *g_loop;
std::vector<std::unique_ptr<Channel>> g_channels;

int g_reads, g_writes, g_fired;

void readCallback(int fd, int idx)
{
        char ch;

        g_reads += static_cast<int>(::recv(fd, &ch, sizeof(ch), 0));
        if (g_writes > 0)
        {
                int widx = idx + 1;
                if (widx >= numPipes)
                {
                        widx -= numPipes;
                }
                ::send(g_pipes[2 * widx + 1], "m", 1, 0);
                g_writes--;
                g_fired++;
        }
        if (g_fired == g_reads)
        {
                g_loop->quit();
        }
}

std::pair<int, int> runOnce()
{
        Timestamp beforeInit(std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::system_clock::now()));
        for (int i = 0; i < numPipes; ++i)
        {
                Channel &channel = *g_channels[i];
                channel.setReadCallback([&channel, i]()
                                        { readCallback(channel.fd(), i); });
                channel.enableRead();
        }

        int space = numPipes / numActive;
        space *= 2;
        for (int i = 0; i < numActive; ++i)
        {
                ::send(g_pipes[i * space + 1], "m", 1, 0);
        }

        g_fired = numActive;
        g_reads = 0;
        g_writes = numWrites;
        Timestamp beforeLoop(std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::system_clock::now()));
        g_loop->loop();

        Timestamp end(std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::system_clock::now()));

        int64_t iterTime = (end - beforeInit).count();
        int64_t loopTime = (end - beforeLoop).count();
        return std::make_pair(iterTime, loopTime);
}

int main(int argc, char *argv[])
{
        numPipes = 100;
        numActive = 1;
        numWrites = 100;
        int c;
        while ((c = getopt(argc, argv, "n:a:w:")) != -1)
        {
                switch (c)
                {
                case 'n':
                        numPipes = atoi(optarg);
                        break;
                case 'a':
                        numActive = atoi(optarg);
                        break;
                case 'w':
                        numWrites = atoi(optarg);
                        break;
                default:
                        fprintf(stderr, "Illegal argument \"%c\"\n", c);
                        return 1;
                }
        }

        rlimit rl;
        rl.rlim_cur = rl.rlim_max = numPipes * 2 + 50;
        if (::setrlimit(RLIMIT_NOFILE, &rl) == -1)
        {
                perror("setrlimit");
        }
        g_pipes.resize(2 * numPipes);
        for (int i = 0; i < numPipes; ++i)
        {
                if (::socketpair(AF_UNIX, SOCK_STREAM, 0, &g_pipes[i * 2]) == -1)
                {
                        perror("pipe");
                        return 1;
                }
        }

        EventLoop loop;
        g_loop = &loop;

        for (int i = 0; i < numPipes; ++i)
        {
                Channel *channel = new Channel(&loop, g_pipes[i * 2]);
                g_channels.emplace_back(channel);
        }

        for (int i = 0; i < 25; ++i)
        {
                std::pair<int, int> t = runOnce();
                printf("%8d %8d\n", t.first, t.second);
        }

        for (const auto &channel : g_channels)
        {
                channel->disableAll();
                channel->remove();
        }
        g_channels.clear();
}

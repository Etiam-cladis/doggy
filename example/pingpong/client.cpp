#include "doggy/net/header.h"

#include "doggy/net/TcpClient.h"
#include "doggy/net/TcpConnection.h"

#include "doggy/net/EventLoop.h"
#include "doggy/net/EventLoopPool.h"
#include "doggy/net/InetAddress.h"

#include <utility>

#include <stdio.h>
#include <unistd.h>

using namespace doggy;
using namespace doggy::net;

class Client;

class Session
{
        using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
        using Timestamp = std::chrono::time_point<std::chrono::system_clock, std::chrono::microseconds>;

public:
        Session(EventLoop *loop,
                const InetAddress &serverAddr,
                Client *owner)
            : client_(loop, serverAddr),
              owner_(owner),
              bytesRead_(0),
              bytesWritten_(0),
              messagesRead_(0)
        {
                client_.setConnectionCallback([this](const TcpConnectionPtr &conn)
                                              { this->onConnection(conn); });
                client_.setMessageCallback([this](const TcpConnectionPtr &conn, Buff &buf)
                                           { this->onMessage(conn, buf); });
        }

        void start()
        {
                client_.connect();
        }

        void stop()
        {
                client_.disconnect();
        }

        int64_t bytesRead() const
        {
                return bytesRead_;
        }

        int64_t messagesRead() const
        {
                return messagesRead_;
        }

private:
        void onConnection(const TcpConnectionPtr &conn);

        void onMessage(const TcpConnectionPtr &conn, Buff &buf)
        {
                ++messagesRead_;
                bytesRead_ += buf.readableBytes();
                bytesWritten_ += buf.readableBytes();
                conn->send(buf);
        }

        TcpClient client_;
        Client *owner_;
        int64_t bytesRead_;
        int64_t bytesWritten_;
        int64_t messagesRead_;
};

class Client
{

        using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

public:
        Client(EventLoop *loop,
               const InetAddress &serverAddr,
               int blockSize,
               int sessionCount,
               std::chrono::microseconds timeout,
               int threadCount)
            : loop_(loop),
              threadPool_(threadCount),
              sessionCount_(sessionCount),
              timeout_(timeout),
              message_(""),
              numConnected_(0)
        {
                loop->runAfter(timeout, [this]()
                               { this->handleTimeout(); });
                if (threadCount > 1)
                {
                        threadPool_.setNumThreads(threadCount);
                }
                threadPool_.start();

                for (int i = 0; i < blockSize; ++i)
                {
                        message_.push_back(static_cast<char>(i % 128));
                }

                for (int i = 0; i < sessionCount; ++i)
                {
                        Session *session = new Session(threadPool_.getNextLoop(), serverAddr, this);
                        session->start();
                        sessions_.emplace_back(session);
                }
        }

        const std::string &message() const
        {
                return message_;
        }

        void onConnect()
        {
                if (numConnected_.fetch_add(1, std::memory_order_relaxed) == sessionCount_)
                {
                }
        }

        void onDisconnect(const TcpConnectionPtr &conn)
        {
                if (numConnected_.fetch_sub(1, std::memory_order_relaxed) == 0)
                {

                        int64_t totalBytesRead = 0;
                        int64_t totalMessagesRead = 0;
                        for (const auto &session : sessions_)
                        {
                                totalBytesRead += session->bytesRead();
                                totalMessagesRead += session->messagesRead();
                        }
                        conn->getLoop()->queueInLoop([this]()
                                                     { this->quit(); });
                }
        }

private:
        void quit()
        {
                loop_->queueInLoop([loop_ = loop_]()
                                   { loop_->quit(); });
        }

        void handleTimeout()
        {
                for (auto &session : sessions_)
                {
                        session->stop();
                }
        }

        EventLoop *loop_;
        EventLoopPool threadPool_;
        int sessionCount_;
        std::chrono::microseconds timeout_;
        std::string message_;
        std::atomic<int> numConnected_;

        std::mutex mutex_;
        std::vector<std::unique_ptr<Session>> sessions_;
};

void Session::onConnection(const TcpConnectionPtr &conn)
{
        if (conn->connected())
        {
                conn->setTcpNoDelay(true);
                conn->send(owner_->message());
                owner_->onConnect();
        }
        else
        {
                owner_->onDisconnect(conn);
        }
}

int main(int argc, char *argv[])
{
        if (argc != 7)
        {
                fprintf(stderr, "Usage: client <host_ip> <port> <threads> <blocksize> ");
                fprintf(stderr, "<sessions> <time>\n");
        }
        else
        {

                const char *ip = argv[1];
                uint16_t port = static_cast<uint16_t>(atoi(argv[2]));
                int threadCount = atoi(argv[3]);
                int blockSize = atoi(argv[4]);
                int sessionCount = atoi(argv[5]);
                std::chrono::microseconds timeout(atoi(argv[6]));

                EventLoop loop;
                InetAddress serverAddr(ip, port);

                Client client(&loop, serverAddr, blockSize, sessionCount, timeout, threadCount);
                loop.loop();
        }
}

#ifndef DOGGY_NET_BUFF_H
#define DOGGY_NET_BUFF_H

#include "doggy/net/header.h"

namespace doggy
{
        namespace net
        {
                class Buff
                {
                public:
                        Buff(size_t init = kInitializerSize, size_t prepend = kPrepend)
                            : buff_(init + prepend), readIndex_(kPrepend), writeIndex_(kPrepend)
                        {
                                assert(kPrepend != 0);
                                assert(readIndex_ == kPrepend);
                                assert(readIndex_ == writeIndex_);
                        }

                        ~Buff() {}

                public:
                        size_t readableBytes() const { return writeIndex_ - readIndex_; };
                        size_t writableBytes() const { return buff_.size() - writeIndex_; };
                        size_t prependableBytes() const { return readIndex_; };

                        void swap(Buff &rhs)
                        {
                                buff_.swap(rhs.buff_);
                                std::swap(readIndex_, rhs.readIndex_);
                                std::swap(writeIndex_, rhs.writeIndex_);
                        }

                        const char *peek() const
                        {
                                return begin() + readIndex_;
                        }

                        void appen(const char *ch, size_t len)
                        {
                                ensureWritable(len);
                                std::copy(ch, ch + len, beginWrite());
                                hasWritten(len);
                        }

                        void hasWritten(size_t len)
                        {
                                assert(len <= writableBytes());
                                writeIndex_ += len;
                        }

                        void unWritten(size_t len)
                        {
                                assert(len <= readableBytes());
                                writeIndex_ -= len;
                        }

                        void ensureWritable(size_t len)
                        {
                                assert(len >= 0);
                                if (len > writableBytes())
                                {
                                        makeSpace(len);
                                }
                        }

                        // return CRLF or nullptr
                        const char *findCRLF() const
                        {
                                const char *crlf = std::search(peek(), beginWrite(), CRLF, CRLF + 2);
                                return (crlf == begin() + writeIndex_ ? nullptr : crlf);
                        }

                        // return CRLF or nullptr
                        const char *findCRLF(const char *start) const
                        {
                                assert(peek() <= start);
                                assert(start <= beginWrite());
                                const char *crlf = std::search(start, beginWrite(), CRLF, CRLF + 2);
                                return (crlf == begin() + writeIndex_ ? nullptr : crlf);
                        }

                        // return EOL or nullptr
                        const char *findEOL() const
                        {
                                const char *eol = std::find(peek(), beginWrite(), '\n');
                                return eol == beginWrite() ? nullptr : eol;
                        }

                        void shrink()
                        {
                                buff_.shrink_to_fit();
                        }

                        void retrieve(size_t len)
                        {
                                if (len < readableBytes())
                                {
                                        readIndex_ += len;
                                }
                                else
                                {
                                        retrieveAll();
                                }
                        }

                        std::string retrieveAllAsString()
                        {
                                return retrieveAsString(readableBytes());
                        }

                        std::string retrieveAsString(size_t len)
                        {
                                assert(len <= readableBytes());
                                return std::string().assign(peek(), len);
                        }

                        void retrieveAll()
                        {
                                readIndex_ = kPrepend;
                                writeIndex_ = kPrepend;
                        }

                        size_t internalCapacity() const
                        {
                                return buff_.capacity();
                        }

                        ssize_t readFdToThis(int fd, int *savedErrno);
                        ssize_t readFdToThis(int fd, int *savedErrno, off_t offset);
                        ssize_t readFdToThis(int fd, int *saveErrno, off_t offsest, int flags);

                private:
                        char *begin()
                        {
                                return &*buff_.begin();
                        }

                        const char *begin() const
                        {
                                return &*buff_.begin();
                        }

                        const char *beginWrite() const
                        {
                                return begin() + writeIndex_;
                        }

                        char *beginWrite()
                        {
                                return begin() + writeIndex_;
                        }

                        void makeSpace(size_t len)
                        {
                                if (writableBytes() + prependableBytes() - kPrepend < len)
                                {
                                        buff_.resize(writeIndex_ + len);
                                }
                                else
                                {
                                        assert(kPrepend < readIndex_);
                                        size_t read = readableBytes();
                                        std::copy(begin() + readIndex_, begin() + writeIndex_, begin() + kPrepend);
                                        writeIndex_ = writeIndex_ - readIndex_ - kPrepend;
                                        readIndex_ = kPrepend;
                                        assert(read == readableBytes());
                                }
                        }

                private:
                        static constexpr size_t kPrepend = 8;
                        static constexpr size_t kInitializerSize = 1024;
                        static constexpr char CRLF[] = "\r\n";

                private:
                        std::vector<char> buff_;
                        size_t readIndex_;
                        size_t writeIndex_;
                };

        } // namespace base

} // namespace doggy

#endif // DOGGY_NET_BUFF_H

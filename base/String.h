#pragma once

#include "./header.h"

namespace doggy
{
        class String
        {
        public:
                String()
                    : ptr_(nullptr), length_(0) {}
                String(const char *ptr)
                    : ptr_(ptr), length_(static_cast<uint64_t>(std::strlen(ptr_))) {}
                String(const unsigned char *ptr)
                    : ptr_(reinterpret_cast<const char *>(ptr)), length_(static_cast<uint64_t>(std::strlen(ptr_))) {}
                String(const std::string &str)
                    : ptr_(str.data()), length_(static_cast<uint64_t>(str.size())) {}
                String(const char *offset, uint64_t length)
                    : ptr_(offset), length_(length) {}

        public:
                const char *data() const { return ptr_; };
                uint64_t size() const { return length_; };
                bool empty() const { return length_ == 0; };
                const char *begin() { return ptr_; };
                const char *end() { return ptr_ + length_; };

                void set(const char *ptr)
                {
                        ptr_ = ptr;
                        length_ = static_cast<uint64_t>(std::strlen(ptr_));
                }

                void set(const char *ptr, uint64_t length)
                {
                        ptr_ = ptr;
                        length_ = length;
                }

                void set(const void *ptr, uint64_t length)
                {
                        ptr_ = reinterpret_cast<const char *>(ptr);
                        length_ = length;
                }

                void clear()
                {
                        ptr_ = nullptr;
                        length_ = 0;
                }

                // clear() when n >length_
                void removePrefix(uint64_t n)
                {
                        if (n > length_)
                        {
                                clear();
                        }
                        else
                        {
                                ptr_ += n;
                                length_ -= n;
                        }
                }

                // clear() when n>length_
                void removeSuffix(uint64_t n)
                {
                        if (n > length_)
                        {
                                clear();
                        }
                        else
                        {
                                length_ -= n;
                        }
                }

                char operator[](int i) const
                {
                        return ptr_[i];
                };

                bool operator==(const String &str) const
                {
                        return (length_ == str.length_) && (std::memcmp(ptr_, str.ptr_, str.length_) == 0);
                }

                bool operator!=(const String &str) const
                {
                        return !(*this == str);
                }

                std::string asString() const
                {
                        return std::string(data(), size());
                }

                void copyToString(std::string &str) const
                {
                        str.assign(data(), size());
                }

        private:
                const char *ptr_;
                uint64_t length_;
        };

} // namespace doggy
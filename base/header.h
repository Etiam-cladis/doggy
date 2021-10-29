#pragma once

#include <algorithm>
#include <vector>
#include <functional>
#include <string>
#include <cstring>
#include <cassert>
#include <errno.h>
#include <iostream>
#include <thread>
#include <mutex>
#include <queue>
#include <atomic>
#include <condition_variable>
#include <memory>
#include <unordered_map>

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/epoll.h>

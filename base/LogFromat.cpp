#pragma once

#include "./header.h"

int main()
{
        struct stat status;
        stat("/", &status);
        std::cout << status.st_blksize << std::endl;
}
link_directories(/usr/local/include/lib)
set(CMAKE_CXX_FLAGS "-g  -Wall               -Wextra              -Werror              -Wconversion         -Wno-unused-parameter  -Woverloaded-virtual   -Wpointer-arith     -Wshadow           -Wwrite-strings     -march=native      -std=c++17          -rdynamic           -pthread")



add_executable(pingpong_client client.cpp)
target_link_libraries(pingpong_client libdoggy.so )

add_executable(pingpong_server server.cpp)
target_link_libraries(pingpong_server libdoggy.so )

add_executable(pingpong_bench bench.cpp)
target_link_libraries(pingpong_bench libdoggy.so )


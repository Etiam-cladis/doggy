
link_directories(/usr/local/include/lib)
set(CMAKE_CXX_FLAGS "-g  -Wall               -Wextra              -Werror              -Wconversion         -Wno-unused-parameter  -Woverloaded-virtual   -Wpointer-arith     -Wshadow           -Wwrite-strings     -march=native      -std=c++17          -rdynamic           -pthread")

add_executable(echoClient clientMain.cpp echoClient.cpp)
target_link_libraries(echoClient libdoggy.so)

add_executable(echoServer serverMain.cpp echoServer.cpp)
target_link_libraries(echoServer libdoggy.so)
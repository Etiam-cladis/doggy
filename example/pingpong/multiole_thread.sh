#!/bin/sh

rm -rf ./build
mkdir build && cd build
cmake ..
make 

killall server
timeout=${timeout:-100}
bufsize=16384

for nosessions in 100 1000; do
  for nothreads in 1 2 3 4; do
    sleep 5
    echo "Bufsize: $bufsize Threads: $nothreads Sessions: $nosessions"
    ./pingpong_server 0.0.0.0 55555 $nothreads $bufsize & srvpid=$!
    ./pingpong_client 127.0.0.1 55555 $nothreads $bufsize $nosessions $timeout
    kill -9 $srvpid
  done
done
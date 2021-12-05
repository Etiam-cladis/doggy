
aux_source_directory(. net_SRCS)

add_library(doggy SHARED ${net_SRCS})
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

add_library(doggy_static STATIC ${net_SRCS})
set_target_properties(doggy_static PROPERTIES OUTPUT_NAME " doggy")
set_target_properties(doggy PROPERTIES CLEAN_DIRECT_OUTPUT 1)
set_target_properties(doggy_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)
SET_TARGET_PROPERTIES(doggy  PROPERTIES  VERSION 0.1  SOVERSION 0)

install(TARGETS doggy  doggy_static
LIBRARY DESTINATION include/lib
ARCHIVE DESTINATION include/lib)

set(HEADERS
Acceptor.h
  Buffer.h
  Channel.h
  Connector.h
  Endian.h
  EventLoop.h
  EventLoopThread.h
  EventLoopPool.h
  header.h
  Socket.h
  SocketsOps.h
  InetAddress.h
  TcpClient.h
  TcpConnection.h
  TcpServer.h
  Timer.h
  TimerId.h
  TimerQueue.h
  )
install(FILES ${HEADERS} DESTINATION include/doggy/net)
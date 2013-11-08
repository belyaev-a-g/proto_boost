#  Асинхронный ТСР сервер и клиент с использованием BOOST
#  e-mail:  <al.g.belyaev@gmail.com>
#
######################################################################

project(server_TCP CXX C)
#project(client_TCP CXX C)

cmake_minimum_required(VERSION 2.8)

include(FindProtobuf)
find_package( Protobuf REQUIRED )

# Debugging BOOST
ADD_DEFINITIONS( -DBOOST_ASIO_ENABLE_HANDLER_TRACKING)


#protobuf compiler
include_directories(${PROTOBUF_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
file(GLOB ProtoFiles "${CMAKE_CURRENT_SOURCE_DIR}/*.proto")
PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS ${ProtoFiles})
#PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS stringdb.proto)


#add_library(isup_psql SHARED qsql_psql.cpp)


set( server_TCP_SRC
    db_server.cpp server_main.cpp ${PROTO_SRCS})

 #set( client_TCP_SRC
 #       blocking_tcp_echo_client.cpp )

add_executable( server_TCP
                ${server_TCP_SRC})

#add_executable( client_TCP
#                ${client_TCP_SRC})

target_link_libraries( server_TCP boost_system boost_thread boost_date_time boost_filesystem boost_regex protobuf pthread)
#target_link_libraries( client_TCP boost_system boost_thread boost_date_time boost_filesystem boost_regex pthread protobuf)

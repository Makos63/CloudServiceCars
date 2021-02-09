cmake_minimum_required(VERSION 3.16)
project(Provider)
#set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -pthread")

#PKG-Conf
#find_package(PkgConfig REQUIRED)

# Protobuf
set(protobuf_MODULE_COMPATIBLE TRUE)
find_package(Protobuf CONFIG REQUIRED)
message(STATUS "Using protobuf ${protobuf_VERSION}")

# Protobuf-compiler
set(_PROTOBUF_PROTOC $<TARGET_FILE:protobuf::protoc>)

# gRPC
find_package(gRPC CONFIG REQUIRED )
message(STATUS "Using gRPC ${gRPC_VERSION}")
set(_GRPC_GRPCPP gRPC::grpc++)
set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:gRPC::grpc_cpp_plugin>)

#mysql
#include(FindPkgConfig)
#pkg_check_modules(LIBMYSQLCLIENT REQUIRED mysqlclient)

# Proto file
get_filename_component(hw_proto "dataproto.proto" ABSOLUTE)
get_filename_component(hw_proto_path "${hw_proto}" PATH)

# Generated sources
set(hw_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/dataproto.pb.cc")
set(hw_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/dataproto.pb.h")
set(hw_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/dataproto.grpc.pb.cc")
set(hw_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/dataproto.grpc.pb.h")
add_custom_command(

        OUTPUT "${hw_proto_srcs}" "${hw_proto_hdrs}" "${hw_grpc_srcs}" "${hw_grpc_hdrs}"
        COMMAND ${_PROTOBUF_PROTOC}
        ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
        --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${hw_proto_path}"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${hw_proto}"
        DEPENDS "${hw_proto}")

# Include generated *.pb.h files
include_directories("${CMAKE_CURRENT_BINARY_DIR}")
set(SOURCE
        ${SOURCE}
        ${CMAKE_CURRENT_SOURCE_DIR}/MariaDB.cpp
        )
set(HEADERS
        ${HEADERS}
        ${CMAKE_CURRENT_SOURCE_DIR}/MariaDB.h
        )
find_package(PkgConfig)

#include(FindPkgConfig)
pkg_check_modules(LIBMYSQLCLIENT REQUIRED mysqlclient)
foreach(FLAG ${LIBMYSQLCLIENT_CFLAGS_OTHER})
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${FLAG}")
endforeach()
#link_directories(${LIBMYSQLCLIENT_LIBRARY_DIRS})

# Targets (client|server)
foreach(_target Provider)

    add_executable(${_target} ${SOURCE} ${HEADERS} "${_target}.cpp"
            ${hw_proto_srcs}
            ${hw_grpc_srcs})
    target_link_libraries(${_target}
            ${_REFLECTION}
            ${_GRPC_GRPCPP}
            ${_PROTOBUF_LIBPROTOBUF}
            ${LIBMYSQLCLIENT_LIBRARIES}
            )

    target_include_directories(${_target} PRIVATE ${LIBMYSQLCLIENT_INCLUDE_DIRS})
endforeach()
#add_executable(Provider Provider.cpp ${hw_proto_srcs} ${hw_grpc_srcs} MariaDB.cpp MariaDB.h)
#target_link_libraries(Provider ${_REFLECTION} ${_GRPC_GRPCPP} ${_PROTOBUF_LIBPROTOBUF} mysqlclient)
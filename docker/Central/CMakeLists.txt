cmake_minimum_required(VERSION 3.16)
project(Central)

#set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -pthread -lmosquitto -lmosquittopp")

#PKG-Conf
#find_package(PkgConfig REQUIRED)


#mqtt mosquitto
find_package(PkgConfig)
pkg_check_modules(LIBMQTT REQUIRED libmosquitto)
pkg_check_modules(LIBMQTTPP REQUIRED libmosquittopp)
link_libraries(${LIBMQTT_LIBRARIES})
link_libraries(${LIBMQTTPP_LIBRARIES})
link_directories(${LIBMQTT_INCLUDE_DIRS})
link_directories(${LIBMQTTPP_INCLUDE_DIRS})

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


add_executable(Central Central.cpp Central.h SensorData.cpp SensorData.h Client.cpp ${hw_proto_srcs}
        ${hw_grpc_srcs} )
target_link_libraries(Central ${_REFLECTION} ${_GRPC_GRPCPP} ${_PROTOBUF_LIBPROTOBUF})
cmake_minimum_required(VERSION 3.28.0)

project(uart_rpc_test)

add_executable(
    uart_rpc_test 
    test.c 
    ../UartRpcClient.c 
    ../UartRpcServer.c
    ../libcrc/src/crc8.c
    ../cobslib/cobs.c
    )

target_include_directories(
    uart_rpc_test PUBLIC
    ../libcrc/include 
    ../cobslib
    ..
    )
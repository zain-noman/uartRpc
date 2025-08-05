#pragma once
#include <stdint.h>
#include "cobs.h"
#include "UartRpcErrors.h"

struct UartRpcServer{
    void* context;
    uint8_t _txBuffer[256];
    uint8_t _tempTxBuffer[256];
    struct CobsDecoder _cobsDecoder;
    
    // dependencies
    void (*uartSend)(void* context, const uint8_t* data,
         uint8_t dataSize);

    // callbacks
    void (*onRequestReceived)
        (uint8_t* message, uint8_t msgLen);
    void (*onError)(enum UartRpcError error);
};

// for servers only
void uartRpcServerSendResponse(
    struct UartRpcServer* rpc,
    uint8_t type,
    uint8_t* data,
    uint8_t dataSize
);

// for servers only
void uartRpcServerSendStreamPacket(
    struct UartRpcServer* rpc,
    uint8_t type,
    uint8_t sub_type,
    int index,
    uint8_t* data,
    uint8_t dataSize
);

void uartRpcServerOnReceiveUartData(
    struct UartRpcServer* client,
    uint8_t data
);

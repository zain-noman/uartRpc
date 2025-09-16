#pragma once
#include <stdint.h>
#include "cobs.h"
#include "UartRpcCommon.h"

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
        (void *context, uint8_t type, uint8_t* message, uint8_t msgLen);
    void (*onError)
        (void *context, enum UartRpcError error);
};

void uartRpcServerInit(struct UartRpcServer* rpc);

// for servers only
void uartRpcServerSendResponse(
    struct UartRpcServer* rpc,
    uint8_t type,
    const uint8_t* data,
    uint8_t dataSize
);

// for servers only
void uartRpcServerSendStreamPacket(
    struct UartRpcServer* rpc,
    uint8_t type,
    int index,
    const uint8_t* data,
    uint8_t dataSize
);

void uartRpcSendEndOfStream(struct UartRpcServer* rpc);

void uartRpcServerOnReceiveData(
    struct UartRpcServer* server,
    uint8_t data
);

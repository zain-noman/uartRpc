#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "cobs.h"
#include "UartRpcCommon.h"

enum UartRpcClientState{
    UART_RPC_IDLE,
    UART_RPC_AWAITING_RESPONSE,
    UART_RPC_RECEIVING_STREAM,
    UART_RPC_AWAITING_STOP_STREAM
};

struct UartRpcResponse {
    uint8_t type;
    const uint8_t* data;
    uint8_t len;

    bool isStream;
    uint8_t streamSubtype;
    uint16_t streamIndex;
};

struct UartRpcClient
{
    void* context;
    int responseTimeMs;

    //private
    struct CobsDecoder _cobsDecoder;
    enum UartRpcClientState _state;
    uint8_t _txBuffer[256];
    uint8_t _tempTxBuffer[256];
    int _expectedStreamPacketId;


    // dependencies
    void (*startOrResetTimer)
        (void* context, int timerTimeMs);
    void (*stopTimer)
        (void* context);
    void (*uartSend)(void* context, const uint8_t* data,
         uint8_t dataSize);
    
    // callbacks 
    void (*onResponseReceived)
        (void* context, struct UartRpcResponse* message);
    void (*onError)(void* context, enum UartRpcError error);
    void (*onStateChanged)(void* context, enum UartRpcClientState state);
};

// the uart rpc struct must be pre-initialized 
// before calling this
void uartRpcClientInit(struct UartRpcClient* rpc);

void informUartRpcClientTimerExpired(struct UartRpcClient* rpc);

// for clients only
void uartRpcClientSendRequest(
    struct UartRpcClient* rpc,
    uint8_t type,
    uint8_t* data,
    uint8_t dataSize
);

void uartRpcClientOnReceiveData(
    struct UartRpcClient* client,
    uint8_t data
);

void stopStream(struct UartRpcClient* client);

#ifdef __cplusplus
}
#endif

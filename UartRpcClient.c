#include "UartRpcClient.h"
#include "checksum.h"

void uartRpcClientInit(struct UartRpcClient* rpc)
{
    rpc->_state = UART_RPC_IDLE;
    cobsDecoderInit(&(rpc->_cobsDecoder),NULL);
}

void informUartRpcClientTimerExpired(struct UartRpcClient* rpc)
{
    if (rpc->_state == UART_RPC_AWAITING_STOP_STREAM){
        rpc->_state = UART_RPC_IDLE;
        return;
    }
    if (rpc->onError != NULL){
        rpc->onError(UART_RPC_TIMEOUT);
    }
}

void uartRpcClientSendRequest(
    struct UartRpcClient* rpc,
    uint8_t type,
    uint8_t* data,
    uint8_t dataSize,
    bool isResponseStream
)
{
    if (type >= 128) return;
    if (dataSize > 249) return;

    rpc->_tempTxBuffer[0] = type;
    for (int i = 0; i < dataSize; i++){
        rpc->_tempTxBuffer[i+1] = data[i];
    }
    
    rpc->_tempTxBuffer[dataSize+3] = crc_8(
        rpc->_tempTxBuffer,
        dataSize+1
    );
    int finalSize = cobsEncode(rpc->_tempTxBuffer,
        dataSize+2,
        rpc->_txBuffer);
    rpc->uartSend(rpc->context,rpc->_txBuffer,finalSize);

    if (isResponseStream){
        rpc->_state = UART_RPC_RECEIVING_STREAM;
        rpc->_expectedStreamPacketId = 0;
    } else {
        rpc->_state = UART_RPC_AWAITING_RESPONSE;
    }
    rpc->startOrResetTimer(rpc->context
        ,rpc->responseTimeMs);
}

void uartRpcClientOnReceiveUartData(
    struct UartRpcClient* client,
    uint8_t data
){
    CobsDecoderStatus status = cobsDecoderAppend(
        &(client->_cobsDecoder),data);

    if (status == COBS_OK)
        return;
    
    if (status != COBS_MESSAGE_COMPLETE){
        if (client->onError != NULL)
            client->onError(UART_RPC_FRAMING_ERROR);
        return;
    }
    
    //message was successfully received

    //verify crc
    uint8_t* messagePtr = NULL;
    int messageLen;
    cobsDecoderGetMessageByRef(&(client->_cobsDecoder),
        &messagePtr, &messageLen);
    uint8_t calculatedCrc = crc_8(messagePtr,messageLen-1);
    if (messagePtr[messageLen-1] != calculatedCrc){
        if (client->onError != NULL)
            client->onError(UART_RPC_CRC_MISMATCH);
        return;
    }

    //this is the cobs encoded final command 
    static const uint8_t stopSendingStreamCommand[] = 
        {3, 0xFF, 0xAA, 0};
    switch (client->_state)
    {
    case UART_RPC_IDLE:
    {
        if (client->onError != NULL)
            client->onError(UART_RPC_UNSOLICITED_MESSAGE);
        bool wasStreamMessage = messagePtr[0] > 128;
        if (wasStreamMessage){
            client->_state = UART_RPC_AWAITING_STOP_STREAM;
            client->uartSend(client->context,
                stopSendingStreamCommand,
                sizeof(stopSendingStreamCommand)
            );
            client->startOrResetTimer(client->context,
                client->responseTimeMs
            );
        }
    }
        break;
    case UART_RPC_AWAITING_RESPONSE:
    {
        struct UartRpcResponse resp = {
            .type = messagePtr[0],
            .data = messagePtr+1,
            .len = messageLen - 2,

            .isStream = false,
            .streamSubtype = 0,
            .streamIndex = 0
        };
        client->onResponseReceived(&resp);
        client->stopTimer(client->context);
        client->_state = UART_RPC_IDLE;
    }
        break;
    case UART_RPC_RECEIVING_STREAM:
    {
        if ((messagePtr[1]>>4) == 0x0F){
            client->stopTimer(client->context);
            client->_state = UART_RPC_IDLE;
            break;
        }

        struct UartRpcResponse resp = {
            .type = messagePtr[0],
            .data = messagePtr+3,
            .len = messageLen - 4,

            .isStream = true,
            .streamSubtype = messagePtr[1]>>4,
            .streamIndex = (messagePtr[1] &0x0F) << 8 | messagePtr[2],
        };
        client->onResponseReceived(&resp);
        client->startOrResetTimer(client->context, 
            client->responseTimeMs);
    }
        break;
    case UART_RPC_AWAITING_STOP_STREAM:
    {
        if ((messagePtr[1]>>4) == 0x0F){
            client->_state = UART_RPC_IDLE;
            break;
        } else {
            client->uartSend(client->context,
                stopSendingStreamCommand,
                sizeof(stopSendingStreamCommand)
            );
            client->startOrResetTimer(client->context,
                client->responseTimeMs
            );
        }
    }
        break;
    default:
        break;
    }
}
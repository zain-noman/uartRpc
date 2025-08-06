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
    uint8_t dataSize
)
{
    if (dataSize > 249) return;

    rpc->_tempTxBuffer[0] = type;
    for (int i = 0; i < dataSize; i++){
        rpc->_tempTxBuffer[i+1] = data[i];
    }
    
    rpc->_tempTxBuffer[dataSize+1] = crc_8(
        rpc->_tempTxBuffer,
        dataSize+1
    );
    int finalSize = cobsEncode(rpc->_tempTxBuffer,
        dataSize+2,
        rpc->_txBuffer);
    
    if (type > STREAM_COMMAND_RESPONSE_CODE){
        rpc->_state = UART_RPC_RECEIVING_STREAM;
        rpc->_expectedStreamPacketId = 0;
    } else {
        rpc->_state = UART_RPC_AWAITING_RESPONSE;
    }
    rpc->startOrResetTimer(rpc->context
        ,rpc->responseTimeMs);
    
    rpc->uartSend(rpc->context,rpc->_txBuffer,finalSize);
}

void stopStream(struct UartRpcClient* client)
{
    //this is the cobs encoded final command 
    static const uint8_t stopSendingStreamCommand[] = 
        {3, 0xFF, 0xAA, 0};
    client->_state = UART_RPC_AWAITING_STOP_STREAM;
    client->uartSend(client->context,
        stopSendingStreamCommand,
        sizeof(stopSendingStreamCommand)
    );
    client->startOrResetTimer(client->context,
        client->responseTimeMs
    );
}

void uartRpcClientOnReceiveData(
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

    switch (client->_state)
    {
    case UART_RPC_IDLE:
    {
        if (client->onError != NULL)
            client->onError(UART_RPC_UNSOLICITED_MESSAGE);
        bool wasStreamMessage = messagePtr[0] > STREAM_COMMAND_RESPONSE_CODE;
        if (wasStreamMessage){
            stopStream(client);
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
        uint8_t type = messagePtr[0];
        if (type < STREAM_COMMAND_RESPONSE_CODE){
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
            break;
        }
        uint8_t subType = messagePtr[1]>>4;
        // if (subType > END_OF_STREAM_SUB_TYPE){
        //     client->stopTimer(client->context);
        //     client->_state = UART_RPC_IDLE;
        //     break;
        // }

        int streamId =
            (messagePtr[1] &0x0F) << 8 | messagePtr[2];

        if (streamId != client->_expectedStreamPacketId){
            if (client->onError != NULL){
                client->onError(UART_RPC_MISSING_PACKET_IN_STREAM);
            }
        } 
        client->_expectedStreamPacketId = streamId + 1;
        
        struct UartRpcResponse resp = {
            .type = messagePtr[0],
            .data = messagePtr+3,
            .len = messageLen - 4,

            .isStream = true,
            .streamSubtype = messagePtr[1]>>4,
            .streamIndex = (messagePtr[1] &0x0F) << 8 | messagePtr[2],
        };
        client->onResponseReceived(&resp);
        if (subType < END_OF_STREAM_SUB_TYPE){
            client->startOrResetTimer(client->context, 
                client->responseTimeMs);
        } else {
            client->stopTimer(client->context);
        }
    }
        break;
    case UART_RPC_AWAITING_STOP_STREAM:
    {
        if ((messagePtr[1]>>4) > END_OF_STREAM_SUB_TYPE){
            client->_state = UART_RPC_IDLE;
            break;
        } else {
            stopStream(client);
        }
    }
        break;
    default:
        break;
    }
}
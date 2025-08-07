#include "UartRpcServer.h"
#include "checksum.h"

void uartRpcServerSendResponse(
    struct UartRpcServer* rpc,
    uint8_t type,
    uint8_t* data,
    uint8_t dataSize
)
{
    if (type >= 128) return;
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
    rpc->uartSend(rpc->context,rpc->_txBuffer,finalSize);
}

void uartRpcServerSendStreamPacket(
    struct UartRpcServer* rpc,
    uint8_t type,
    int index,
    uint8_t* data,
    uint8_t dataSize
){
    if (type < 128) return;
    if (dataSize > 249) return;

    rpc->_tempTxBuffer[0] = type;
    rpc->_tempTxBuffer[1] = (index>>8) & 0xFF;
    rpc->_tempTxBuffer[2] = index & 0xFF;
    for (int i = 0; i < dataSize; i++){
        rpc->_tempTxBuffer[i+3] = data[i];
    }
    
    rpc->_tempTxBuffer[dataSize+3] = crc_8(
        rpc->_tempTxBuffer,
        dataSize+3
    );
    int finalSize = cobsEncode(rpc->_tempTxBuffer,
        dataSize+4,
        rpc->_txBuffer);
    rpc->uartSend(rpc->context,rpc->_txBuffer,finalSize);
}

void uartRpcServerOnReceiveData(
    struct UartRpcServer* server,
    uint8_t data
)
{
    CobsDecoderStatus status = cobsDecoderAppend(
        &(server->_cobsDecoder),data);

    if (status == COBS_OK)
        return;
    
    if (status != COBS_MESSAGE_COMPLETE){
        if (server->onError != NULL)
            server->onError(
                server->context,UART_RPC_FRAMING_ERROR);
        return;
    }
    
    //verify crc
    uint8_t* messagePtr = NULL;
    int messageLen;
    cobsDecoderGetMessageByRef(&(server->_cobsDecoder),
        &messagePtr, &messageLen);
    uint8_t calculatedCrc = crc_8(messagePtr,messageLen-1);
    if (messagePtr[messageLen-1] != calculatedCrc){
        if (server->onError != NULL)
            server->onError(server->context,
                UART_RPC_CRC_MISMATCH);
        return;
    }
    server->onRequestReceived(server->context,
        messagePtr[0],messagePtr+1,messageLen-2);
}

void uartRpcSendEndOfStream(struct UartRpcServer* rpc)
{
    uartRpcServerSendResponse(rpc,
        STOP_SENDING_STREAM_COMMAND_TYPE,NULL,0);
}

void uartRpcServerInit(struct UartRpcServer* rpc)
{
    (void) rpc;
}
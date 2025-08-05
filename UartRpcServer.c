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
    
    rpc->_tempTxBuffer[dataSize+3] = crc_8(
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
    uint8_t sub_type,
    int index,
    uint8_t* data,
    uint8_t dataSize
){
    if (type < 128) return;
    if (dataSize > 249) return;

    rpc->_tempTxBuffer[0] = type;
    rpc->_tempTxBuffer[1] = sub_type<<4 | 
        ((index>>8)& 0x0F);
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
    struct UartRpcServer* client,
    uint8_t data
)
{
    CobsDecoderStatus status = cobsDecoderAppend(
        &(client->_cobsDecoder),data);

    if (status == COBS_OK)
        return;
    
    if (status != COBS_MESSAGE_COMPLETE){
        if (client->onError != NULL)
            client->onError(UART_RPC_FRAMING_ERROR);
        return;
    }
    
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
}

void uartRpcServerInit()
{
    //nothing yet
}
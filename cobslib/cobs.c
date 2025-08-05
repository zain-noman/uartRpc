#include "cobs.h"
#include "memory.h"

void cobsDecoderInit(
    struct CobsDecoder* cobs,
    void (*messageCb)(uint8_t*, int)
){
    cobs->position = 0;
    cobs->onCompleteMessageReceived = messageCb;
    // we expect the first byte to be an encoded byte
    cobs->bytesTillEncodedByte = 0;
}

// call this function whenever a byte is received
CobsDecoderStatus cobsDecoderAppend(
    struct CobsDecoder* cobs, 
    uint8_t receivedData)
{
    // not an encoded byte
    if (cobs->bytesTillEncodedByte != 0){
        if(receivedData == 0){
            //reset
            cobs->position = 0;
            cobs->bytesTillEncodedByte = 0;

            return COBS_UNEXPECTED_EOP;
        } else {
            cobs->buffer[cobs->position] = receivedData;
            cobs->position++;
            cobs->bytesTillEncodedByte--;
            return COBS_OK;
        }
    } else{
        //encoded byte
        if (receivedData == 0){
            //packet ended. call callback and reset
            if (cobs->onCompleteMessageReceived != NULL)
            {
                cobs->onCompleteMessageReceived(
                    cobs->buffer+1,cobs->position-1
                );
            }

            cobs->messageLen = cobs->position-1;

            cobs->position = 0;
            cobs->bytesTillEncodedByte = 0;


            return COBS_MESSAGE_COMPLETE;
        } else {
            cobs->buffer[cobs->position] = 0;
            cobs->position++;

            cobs->bytesTillEncodedByte = receivedData - 1;
            return COBS_OK;
        }
    }
}

void cobsDecoderGetMessage(
    struct CobsDecoder* cobs, 
    uint8_t* messageOut
)
{
    memcpy(messageOut,cobs->buffer+1,cobs->messageLen);
}

void cobsDecoderGetMessageByRef(
    struct CobsDecoder* cobs, 
    uint8_t** messageRefOut,
    int* msgLenOut
)
{
    *messageRefOut = (cobs->buffer + 1);
    *msgLenOut = cobs->messageLen;
}

int cobsEncode(uint8_t* data, int dataLen, uint8_t*outBuf){ 
    if (dataLen > 254) return -1;

    // out buffer format = [posOfFirstZero ...encodedData 0]
    outBuf[dataLen+1] = 0;
    int distFromNextEncoded = 1;
    for (int i = dataLen-1; i >= 0; i--){
        if (data[i] == 0){
            outBuf[i+1] = distFromNextEncoded;
            distFromNextEncoded = 1;
        } else {
            distFromNextEncoded++;
            outBuf[i+1] = data[i];
        }
    }
    outBuf[0] = distFromNextEncoded;
    return dataLen + 2;
}
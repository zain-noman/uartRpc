#pragma once
#include <stdint.h>

#define COBS_BUFFER_SIZE 256

struct CobsDecoder{
    void (*onCompleteMessageReceived)
        (uint8_t* msg, int messageLen);
    uint8_t buffer[COBS_BUFFER_SIZE];
    
    //private
    int position;
    // by 'encoded byte' we mean a byte which is a delimiter or
    // or represents no of bytes until next 'encoded byte'
    int bytesTillEncodedByte;
    int messageLen;
};

typedef enum CobsDecoderStatus_e{
    COBS_OK,
    COBS_OVERFLOW,
    COBS_UNEXPECTED_EOP,
    COBS_MESSAGE_COMPLETE,
}CobsDecoderStatus;

void cobsDecoderInit(
    struct CobsDecoder* cobs,
    void (*messageCb)(uint8_t*, int)
);

// call this function whenever a byte is received
CobsDecoderStatus cobsDecoderAppend(
    struct CobsDecoder* cobs, 
    uint8_t receivedData);

void cobsDecoderGetMessage(
    struct CobsDecoder* cobs, 
    uint8_t* messageOut
);

void cobsDecoderGetMessageByRef(
    struct CobsDecoder* cobs, 
    uint8_t** messageRefOut,
    int* msgLenOut
);

/**
 * @brief encodes using cobs
 * @param data the data to encode
 * @param dataLen the size of data
 * @param outBuf the output that will be filled with encoded data 
 * and delimiter. Its size should be atleast dataLen + 2 
 * @return returns the size of the encoded data (always dataLen + 2)
 * In case of overflow returns -1
 */
int cobsEncode(uint8_t* data, int dataLen, uint8_t*outBuf);
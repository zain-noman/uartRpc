#include "cobs.h"
#include "memory.h"
#include "assert.h"
#include "stdbool.h"

bool messageReceived = false;
uint8_t receivedMessage[COBS_BUFFER_SIZE];
int receivedMessageLength = 0;

void myCobsMessageReceiveCallback(uint8_t* msg, int len)
{
    messageReceived = true;
    memcpy(receivedMessage,msg,len);
    receivedMessageLength = len;
}

int main()
{
    struct CobsDecoder cobs;
    cobsDecoderInit(&cobs, myCobsMessageReceiveCallback);
    
    uint8_t encodedData1[] = {0x03,0x11,0x22,0x02,0x33,0x00};
    //this corresponds to a message 0x11,0x22,0x00,0x33
    for (size_t i = 0; i < sizeof(encodedData1); i++)
    {
        assert(messageReceived == false);
        CobsDecoderStatus status = 
            cobsDecoderAppend(&cobs,encodedData1[i]);
        if (i != (sizeof(encodedData1)-1))
            assert(status == COBS_OK);
        else
            assert(status == COBS_MESSAGE_COMPLETE);
    }
    assert(messageReceived == true);
    assert(receivedMessageLength == 4);
    uint8_t expectedData1[] = {0x11, 0x22, 0x00, 0x33};
    for (size_t i = 0; i < sizeof(expectedData1); i++)
    {
        assert(receivedMessage[i]==expectedData1[i]);
    }
    uint8_t receivedMessageV2[4];
    cobsDecoderGetMessage(&cobs,receivedMessageV2);
    for (size_t i = 0; i < sizeof(expectedData1); i++)
    {
        assert(receivedMessageV2[i]==expectedData1[i]);
    }

    messageReceived = false;
    uint8_t encodedData2[] = 
        {0x05,0x11,0x22,0x33,0x44,0x03,0x55,0x66,0x02,0x77,0x00};
    for (size_t i = 0; i < sizeof(encodedData2); i++)
    {
        assert(messageReceived == false);
        CobsDecoderStatus status = 
            cobsDecoderAppend(&cobs,encodedData2[i]);
        if (i != (sizeof(encodedData2)-1))
            assert(status == COBS_OK);
        else
            assert(status == COBS_MESSAGE_COMPLETE);
    }
    assert(messageReceived == true);
    uint8_t expectedData2[] = {
        0x11, 0x22, 0x33, 0x44, 0x00, 0x55, 0x66,
        0x00, 0x77};
    assert(receivedMessageLength == sizeof(expectedData2));
    for (size_t i = 0; i < sizeof(expectedData2); i++)
    {
        assert(receivedMessage[i]==expectedData2[i]);
    }
}
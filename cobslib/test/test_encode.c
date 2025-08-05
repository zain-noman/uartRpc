#include "cobs.h"
#include "memory.h"
#include "assert.h"
#include "stdbool.h"

int main()
{
    {
        uint8_t data1[] = {0x11,0x22,0x00,0x33};
        uint8_t encodedData1[sizeof(data1)+2];
        int encodedLen = cobsEncode(data1,sizeof(data1),encodedData1);
        assert(encodedLen == sizeof(data1)+2);
        uint8_t expectedEncodedData1[] = {0x03,0x11,0x22,0x02,0x33,0x00};
        for (size_t i = 0; i < sizeof(encodedData1); i++)
        {
            assert(encodedData1[i] == expectedEncodedData1[i]);
        }
    }
    {
        uint8_t data2[] = 
            {0x11, 0x22, 0x33, 0x44, 0x00, 0x55, 0x66, 0x00, 0x77};
        uint8_t encodedData2[sizeof(data2)+2];
        int encodedLen = cobsEncode(data2,sizeof(data2),encodedData2);
        assert(encodedLen == sizeof(data2)+2);
        uint8_t expectedEncodedData2[] = 
            {0x05,0x11,0x22,0x33,0x44,0x03,0x55,0x66,0x02,0x77,0x00};
        for (size_t i = 0; i < sizeof(encodedData2); i++)
        {
            assert(encodedData2[i] == expectedEncodedData2[i]);
        }
    }
}
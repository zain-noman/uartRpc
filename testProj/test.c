#include "UartRpcClient.h"
#include "UartRpcServer.h"
#include <stddef.h>
#include <stdio.h>

void mockStartOrResetTimer(void *context, int timerTimeMs);
void mockStopTimer(void* context);
void mockClientUartSend(void *context, 
    const uint8_t *data, uint8_t dataSize);
void clientsResponseHandler(struct UartRpcResponse *message);
void clientErrorHandler(enum UartRpcError error);
void mockServerUartSend(void *context, const uint8_t *data,
    uint8_t dataSize);
void onRequestReceived(uint8_t type, uint8_t *message, uint8_t msgLen);
void serverErrorHandler(enum UartRpcError error);

struct UartRpcClient client = {
    .context = NULL,
    .responseTimeMs = 1000,
    .startOrResetTimer = mockStartOrResetTimer,
    .stopTimer = mockStopTimer,
    .uartSend = mockClientUartSend,
    .onResponseReceived = clientsResponseHandler,
    .onError = clientErrorHandler
};

struct UartRpcServer server = {
    .context = NULL,
    .uartSend = mockServerUartSend,
    .onRequestReceived = onRequestReceived,
    .onError = serverErrorHandler
};

void mockStartOrResetTimer(void *context, int timerTimeMs){
    printf("timer was reset\n");
}

void mockStopTimer(void* context){
    printf("timer was stopped\n");
}

void mockClientUartSend(void *context, const uint8_t *data, uint8_t dataSize)
{
    for (int i = 0; i < dataSize; i++){
        uartRpcServerOnReceiveData(&server, data[i]);
    }   
}

void clientsResponseHandler(struct UartRpcResponse *message)
{
    printf("got response from server. type %02X", 
        message->type
    );
    if (message->isStream){
        printf(", subtype %02X, stream index %d",
            message->streamSubtype,
            message->streamIndex
        );
    }
    printf("\n\t data: ");
    for (int i = 0; i < message->len; i++){
        printf("%02X ",message->data[i]);
    }
    printf("\n");
}

void clientErrorHandler(enum UartRpcError error)
{
    printf("Error on client %d\n",error);
}

void mockServerUartSend(void *context, const uint8_t *data, uint8_t dataSize)
{
    for (int i = 0; i < dataSize; i++){
        uartRpcClientOnReceiveData(&client, data[i]);
    }  
} 

void onRequestReceived(uint8_t type, uint8_t *message, uint8_t msgLen)
{
    printf("Request Received by Server. type %02X, len:%d \n",
        type, msgLen);
}

void serverErrorHandler(enum UartRpcError error)
{
    printf("Error on server %d\n",error);
}

int main()
{
    uartRpcClientInit(&client);
    uartRpcServerInit();
    
    uartRpcClientSendRequest(&client,12,"hello",5);
    uartRpcServerSendResponse(&server,12,"hi",2);

    //stream request
    uartRpcClientSendRequest(&client,130,"hello",5);
    uartRpcServerSendStreamPacket(&server,130,0,0,"hi 0",4);
    uartRpcServerSendStreamPacket(&server,130,0,1,"hi 1",4);
    uartRpcServerSendStreamPacket(&server,130,0,2,"hi 2",4);
    uartRpcServerSendStreamPacket(&server,130,0,4,"hi 4",4);
    uartRpcServerSendStreamPacket(&server,130,0,5,"hi 5",4);
    uartRpcServerSendStreamPacket(&server,130,8,5,"bye",4);
}
#include "UartRpcClient.h"
#include "UartRpcServer.h"
#include <stddef.h>
#include <stdio.h>

void mockStartOrResetTimer(void *context, int timerTimeMs);
void mockStopTimer(void* context);
void mockClientUartSend(void *context, 
    const uint8_t *data, uint8_t dataSize);
void clientsResponseHandler(void *context, struct UartRpcResponse *message);
void clientErrorHandler(void *context, enum UartRpcError error);
void mockServerUartSend(void *context, const uint8_t *data,
    uint8_t dataSize);
void onRequestReceived(void *context, uint8_t type, uint8_t *message, uint8_t msgLen);
void serverErrorHandler(void *context, enum UartRpcError error);
void onClientStateChange(void *context, enum UartRpcClientState state);


struct UartRpcClient client = {
    .context = NULL,
    .responseTimeMs = 1000,
    .startOrResetTimer = mockStartOrResetTimer,
    .stopTimer = mockStopTimer,
    .uartSend = mockClientUartSend,
    .onResponseReceived = clientsResponseHandler,
    .onError = clientErrorHandler,
    .onStateChanged = onClientStateChange
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

void clientsResponseHandler(void *context, struct UartRpcResponse *message)
{
    printf("got response from server. type %02X", 
        message->type
    );
    if (message->isStream){
        printf(" stream index %d",
            message->streamIndex
        );
    }
    printf("\n\t data: ");
    for (int i = 0; i < message->len; i++){
        printf("%02X ",message->data[i]);
    }
    printf("\n");
}

void clientErrorHandler(void *context, enum UartRpcError error)
{
    printf("Error on client %d\n",error);
}

void mockServerUartSend(void *context, const uint8_t *data, uint8_t dataSize)
{
    for (int i = 0; i < dataSize; i++){
        uartRpcClientOnReceiveData(&client, data[i]);
    }  
} 

void onRequestReceived(void *context, uint8_t type, uint8_t *message, uint8_t msgLen)
{
    printf("Request Received by Server. type %02X, len:%d \n",
        type, msgLen);
}

void serverErrorHandler(void *context, enum UartRpcError error)
{
    printf("Error on server %d\n",error);
}

void onClientStateChange(void *context, enum UartRpcClientState state)
{
    switch (state)
    {
    case UART_RPC_IDLE:
        printf("State: Idle\n");
        break;
    case UART_RPC_AWAITING_RESPONSE:
        printf("State: AWAITING_RESPONSE\n");
        break;
    case UART_RPC_RECEIVING_STREAM:
        printf("State: RECEIVING_STREAM\n");
        break;
    case UART_RPC_AWAITING_STOP_STREAM:
        printf("State: AWAITING_STOP_STREAM\n");
        break;
    default:
        break;
    }
}

int main()
{
    uartRpcClientInit(&client);
    uartRpcServerInit(&server);
    
    // normal request
    uartRpcClientSendRequest(&client,12,"hello",5);
    uartRpcServerSendResponse(&server,12,"hi",2);

    // stream request
    uartRpcClientSendRequest(&client,130,"hello",5);
    uartRpcServerSendStreamPacket(&server,130,0,"hi 0",4);
    uartRpcServerSendStreamPacket(&server,130,1,"hi 1",4);
    uartRpcServerSendStreamPacket(&server,130,2,"hi 2",4);
    // skip packet 3 and see if the client is informed 
    uartRpcServerSendStreamPacket(&server,130,4,"hi 4",4);
    uartRpcServerSendStreamPacket(&server,130,5,"hi 5",4);
    uartRpcSendEndOfStream(&server);

    //unsolicited normal response
    uartRpcServerSendResponse(&server,12,"hi",2);

    //unsolicited stream response
    // the server should automatically get a stop stream
    // command
    uartRpcServerSendStreamPacket(&server,130,0,"hi",2);
    uartRpcServerSendStreamPacket(&server,130,1,"hi",2);
    // the client should exit the awaiting stream end state
    uartRpcSendEndOfStream(&server);
}
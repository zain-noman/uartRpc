#include "UartRpcClient.h"
#include "UartRpcServer.h"
#include <stddef.h>
#include <stdio.h>

void mockStartOrResetTimer(void *context, int timerTimeMs){
    printf("timer was reset\n");
}

void mockStopTimer(void* context){
    printf("timer was stopped\n");
}

void mockClientUartSend(void *context, const uint8_t *data, uint8_t dataSize)
{

}

void clientsResponseHandler(struct UartRpcResponse *message)
{

}

void clientErrorHandler(enum UartRpcError error)
{

}

void mockServerUartSend(void *context, const uint8_t *data, uint8_t dataSize)
{

} 

void onRequestReceived(uint8_t *message, uint8_t msgLen)
{

}

void serverErrorHandler(enum UartRpcError error)
{

}

int main()
{
    struct UartRpcClient client = {
        .context = NULL,
        .responseTimeMs = 1000,
        .startOrResetTimer = mockStartOrResetTimer,
        .stopTimer = mockStopTimer,
        .uartSend = mockClientUartSend,
        .onResponseReceived = clientsResponseHandler,
        .onError = clientErrorHandler
    };
    uartRpcClientInit(&client);

    struct UartRpcServer server = {
        .context = NULL,
        .uartSend = mockServerUartSend,
        .onRequestReceived = onRequestReceived,
        .onError = serverErrorHandler
    };
    uartRpcServerInit();
    
    uartRpcClientSendRequest(&client,12,NULL,0);


}
#include <BOARD.h>
#include <xc.h>
#include <Protocol.h>
#include <sys/attribs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <MessageIDs.h>
#include <circleBuff.h>

//static struct CircleBuffer RXCB = {};

/*int main(void) {

    BOARD_Init();
    Protocol_Init();
    //init_buff(&TXCB);
    //init_buff();
    LEDS_INIT();
    
    unsigned char theID;
    unsigned int reversedPayload;
    unsigned int finalPayload;
    int thePAYLOAD;
    uint8_t name;
    

    while (1) {
        if (Protocol_IsMessageAvailable() == TRUE) {
            theID = Protocol_ReadNextID();
            if (theID == ID_PING) {
                thePAYLOAD = Protocol_GetPayload(&name); // get the payload
                name = Protocol_IntEndednessConversion(name); // convert endedness
                name = (name >> 1); // divide by 2
                name = Protocol_IntEndednessConversion(name); // convert endedness again
                Protocol_SendMessage(0x05, ID_PONG, &name); // send back message with ID_PONG
            }
        } //return finalPayload;
    }
    return 0;
}*/
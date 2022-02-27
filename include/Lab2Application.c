#include "Protocol.h"
#include "PingSensor.h"
#include "RotaryEncoder.h"
#include "RCServo.h"
#include "FreeRunningTimer.h"
#include "BOARD.h"
#include "circleBuff.h"
#include "MessageIDs.h"
#include <xc.h>
#include <sys/attribs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern unsigned int APP_PL;

typedef enum {
    WAIT, Encoder, Ping_Sensor
} state;

static state MODE = WAIT;

int main(void) {
    BOARD_Init();
    Protocol_Init();
    RCServo_Init();
    PingSensor_Init();
    RotaryEncoder_Init(ENCODER_BLOCKING_MODE);
    //    LEDS_INIT();
    FreeRunningTimer_Init();

    union {
        signed int i;
        unsigned char word[5];
    } inData;
    
    unsigned short theAngle;




    //2022-02-26 21:02:10.559528	ID_LAB2_INPUT_SELECT	0XCC 02 8B 01 B9 C6 0D 0A
    //2022-02-26 21:02:15.766923	ID_LAB2_INPUT_SELECT	0XCC 02 8B 00 B9 C5 0D 0A

    //unsigned char APP_ID;
    // unsigned char APP_PL[2];
    while (1) {
        int i;
        switch (MODE) {

            case WAIT:
                if (APP_PL == 1) {
                    MODE = Encoder;
                } else if (APP_PL == 0) {
                    MODE = Ping_Sensor;
                } else {
                    MODE = WAIT;
                }
                break;

            case Encoder:
                Protocol_SendDebugMessage("Encoder");

                for (i = 0; i < 100000; i++) {
                    asm("nop");
                }
                theAngle = RotaryEncoder_ReadRawAngle();
                theAngle = (theAngle & 0x3FFF);
                theAngle = Protocol_IntEndednessConversion(theAngle);
                Protocol_SendMessage((unsigned char) 0x4, ID_LAB2_ANGLE_REPORT, &(theAngle));
                for (i = 0; i < 100000; i++) {
                    asm("nop");
                }
//                inData.i = RotaryEncoder_ReadRawAngle();
//                inData.i = (inData.i & 0x3FFF);
//                inData.i = Protocol_ShortEndednessConversion(inData.i);
//                Protocol_SendMessage((unsigned char) 0x4, ID_LAB2_ANGLE_REPORT, &(inData.i));
                MODE = WAIT;
                break;

            case Ping_Sensor:
                Protocol_SendDebugMessage("Ping Sensor");
                for (i = 0; i < 100000; i++) {
                    asm("nop");
                }
                MODE = WAIT;
                break;
        }
        //        if (Protocol_IsMessageAvailable() == TRUE) {
        //            Protocol_GetPayload(&APP_PL);
        //            APP_ID = Protocol_ReadNextID();
        //            
        //            if (APP_PL[0] == 0x01) {
        //                Protocol_SendDebugMessage("Encoder");
        //            } else {
        //               // Potocol_SendDebugMessage("Ping Sensor");
        //            }
        //            switch (MODE) {
        //                
        //                case Encoder:
        //                    if (/*...*/ {
        //                        /* */
        //                    }
        //                    break;
        //                    
        //                case Ping_Sensor:
        //                    
        //            }
    }


    return 0;
}
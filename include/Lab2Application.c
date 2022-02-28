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

//#define APPLICATION_TEST

extern unsigned int APP_PL;
extern unsigned int distance;
extern unsigned int rc_PL1;
extern unsigned int rc_PL2;
extern unsigned int rc_PL3;
extern unsigned int rc_PL4;

typedef enum {
    WAIT, Encoder, Ping_Sensor
} state;

static state MODE = WAIT;

#ifdef APPLICATION_TEST

int main(void) {
    BOARD_Init();
    Protocol_Init();
    RCServo_Init();
    PingSensor_Init();
    RotaryEncoder_Init(ENCODER_BLOCKING_MODE);
    //    LEDS_INIT();
    FreeRunningTimer_Init();
    TRISDbits.TRISD1 = 0; // Set Pin7 to output
    TRISDbits.TRISD0 = 0;
    TRISDbits.TRISD10 = 1;

    union {
        signed int i;
        unsigned char word[5];
    } inData;

    union {
        unsigned int i;
        char word[5];
    } variable;

    unsigned short theAngle;
    unsigned short data2send;
    unsigned int prevPulse = OC3RS;
    unsigned int thisPulse;
    unsigned short distanceMillis;
    unsigned short distanceMeters;
    unsigned char testval[MAXPAYLOADLENGTH];
    unsigned int j;
    int i;




    //2022-02-26 21:02:10.559528	ID_LAB2_INPUT_SELECT	0XCC 02 8B 01 B9 C6 0D 0A
    //2022-02-26 21:02:15.766923	ID_LAB2_INPUT_SELECT	0XCC 02 8B 00 B9 C5 0D 0A

    //unsigned char APP_ID;
    // unsigned char APP_PL[2];
    while (1) {

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
                SPI2CONbits.ON = 1; // turning on SPI
                // IC3CONbits.ON = 0; // Input Capture disabled
                Protocol_SendDebugMessage("Encoder");
                for (i = 0; i < 100000; i++) {
                    asm("nop");
                }

                data2send = RotaryEncoder_ReadRawAngle();
                data2send = (data2send & 0x3FFF);
                // data2send = Protocol_ShortEndednessConversion(data2send);
                RCServo_SetPulse((unsigned int) data2send);
                //                if (prevPulse != OC3RS) {
                //                    thisPulse = RCServo_GetPulse(); // "thisPulse" should be in us after GetPulse
                //                    prevPulse = OC3RS; // update prevPulse
                //                    RCServo_SetPulse(data2send);
                //                    thisPulse = Protocol_IntEndednessConversion(thisPulse);
                //                    Protocol_SendMessage(5, ID_SERVO_RESPONSE, &thisPulse);
                //                } else {
                //                    thisPulse = ((rc_PL1 << 24) | (rc_PL2 << 16) | (rc_PL3 << 8) | rc_PL4);
                //                    RCServo_SetPulse(thisPulse); // scaling it to an unsigned int
                //                }
                //
                Protocol_SendMessage(4, ID_LAB2_ANGLE_REPORT, &(data2send));
                for (j = 0; j < 100000; j++) {
                    asm("nop");
                }

                MODE = WAIT;
                break;

            case Ping_Sensor:
                SPI2CONbits.ON = 0; // turning off SPI so only ping is running
                // IC3CONbits.ON = 1; // Input Capture enabled
                Protocol_SendDebugMessage("Ping Sensor");
                for (i = 0; i < 1000000; i++) {
                    asm("nop");
                }
                distanceMillis = PingSensor_GetDistance();
                //  Protocol_SendDebugMessage(distanceMillis);
                distanceMeters = distanceMillis * 10;
               // theAngle = Convert2Degrees((unsigned int) distanceMeters);
                //  RCServo_SetPulse(theAngle);
                sprintf(testval, "%d", distanceMeters);
                Protocol_SendDebugMessage(testval);
               // if (theAngle != OC3RS) {
                    variable.i = theAngle;
                    RCServo_SetPulse(distanceMeters);
                    variable.i = Protocol_IntEndednessConversion(variable.i);
                    Protocol_SendMessage(0x05, ID_LAB2_ANGLE_REPORT, &(variable.i));
               // }
                for (int i = 0; i < 1000000; i++) { // delay for at least 10us before resetting the interrupt flag
                    asm(" nop ");

                }

                MODE = WAIT;
                break;
        }
    }
    return 0;
    while (1);
    BOARD_End();
}
#endif
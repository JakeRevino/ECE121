#include <proc/p32mx340f512h.h>
#include <proc/PIC32MX/p32mx340f512h.h>
#include <xc.h>            
#include <sys/attribs.h> 
#include "Protocol.h"
#include "BOARD.h"
#include "RCServo.h"
#include "MessageIDS.h"
#include <stdio.h>

//#define RC_SERVO_TEST

//#define OC_TRIS TRISDbits.TRISD2 // pin 6
//#define OC_PORT PORTDbits.RD2


/*
 * THESE ARE ALREADY DEFINED IN .H
#define RC_SERVO_MIN_PULSE 600
#define RC_SERVO_CENTER_PULSE 1500
#define RC_SERVO_MAX_PULSE 2400
 */
/*******************************************************************************
 * PUBLIC FUNCTIONS                                                           *
 ******************************************************************************/
static unsigned int nextPulse;
static unsigned int tickPulse;
//static unsigned char* RC_PAYLOAD;
static unsigned int countIt;
//static unsigned char SERVO_ID;
static unsigned int thisPulse;
static unsigned int BUSY;
static int flag = 1;
//static unsigned int microPulse;
static unsigned int currentPulse;

extern unsigned int rc_PL1;
extern unsigned int rc_PL2;
extern unsigned int rc_PL3;
extern unsigned int rc_PL4;

/**
 * @Function RCServo_Init(void)
 * @param None
 * @return SUCCESS or ERROR
 * @brief initializes hardware required and set it to the CENTER PULSE */
int RCServo_Init(void) {
    /* CONFIGURE TIMER3 */
    T3CON = 0; // clear control registers
    T3CONbits.TCKPS = 0b101; // set pre-scaler 1:32
    TMR3 = 0; // set current value to zero
    PR3 = 62500; // Set period register for 50ms roll-over

    /* CONFIGURE OUTPUT COMPARE MODULE */
    OC3CON = 0; // clear and disable control register
    OC3CONbits.OCTSEL = 1; // Output compare. This slaves timer3 
    OC3CONbits.OCM = 0b110; // PWM mode with Fault pin disabled
    OC3R = 0; // this sets initial duty cycle
    OC3RS = 1000;
    nextPulse = RC_SERVO_CENTER_PULSE;

    /* OC3 Interrupts */
    IFS0bits.OC3IF = 0; // clear the interrupt flag
    IEC0bits.OC3IE = 1;
    IPC3bits.OC3IP = 5;
    IPC3bits.OC3IS = 0;

    /* ENABLE */
    T3CONbits.ON = 1; // enable timer3
    OC3CONbits.ON = 1; // enable OC3

    return SUCCESS;
}

/**
 * @Function int RCServo_SetPulse(unsigned int inPulse)
 * @param inPulse, integer representing number of microseconds
 * @return SUCCESS or ERROR
 * @brief takes in microsecond count, converts to ticks and updates the internal variables
 * @warning This will update the timing for the next pulse, not the current one */
int RCServo_SetPulse(unsigned int inPulse) {
    currentPulse = ((inPulse * 125) / 100); // convert to ticks
    if (currentPulse <= RC_SERVO_MIN_PULSE) {
        OC3RS = RC_SERVO_MIN_PULSE;
    } else if (currentPulse >= RC_SERVO_MAX_PULSE) {
        OC3RS = RC_SERVO_MAX_PULSE;
    } else {
        OC3RS = currentPulse;
    }
    //IFS0bits.OC3IF = 1;
    return SUCCESS;
}

/**
 * @Function int RCServo_GetPulse(void)
 * @param None
 * @return Pulse in microseconds currently set */
unsigned int RCServo_GetPulse(void) {
    unsigned int temp = OC3RS;
    unsigned int microPulse = ((temp * 100) / 125); // convert to us
    return microPulse;
}

/**
 * @Function int RCServo_GetRawTicks(void)
 * @param None
 * @return raw timer ticks required to generate current pulse. */
unsigned int RCServo_GetRawTicks(void) {
    return OC3RS;
}

#ifdef RC_SERVO_TEST

int main(void) {
    RCServo_Init();
    LEDS_INIT();
    BOARD_Init();
    Protocol_Init();
    unsigned int servoPayload, prevPulse;
    unsigned char SERVO_ID;
    unsigned char trash[128];
    static int flag = 0;
    LEDS_SET(0b11);

//    union {
//        unsigned int z;
//        unsigned char D[4];
//    } output;
    prevPulse = OC3RS;
    while (1) {
        if (prevPulse != OC3RS) {
            thisPulse = RCServo_GetPulse(); // "thisPulse" should be in us after GetPulse
            prevPulse = OC3RS; // update prevPulse
            //  RCServo_SetPulse(prevPulse);
            thisPulse = Protocol_IntEndednessConversion(thisPulse);
            Protocol_SendMessage(5, ID_SERVO_RESPONSE, &thisPulse);
        } else {
            thisPulse = ((rc_PL1 << 24) | (rc_PL2 << 16) | (rc_PL3 << 8) | rc_PL4);
            RCServo_SetPulse(thisPulse); // scaling it to an unsigned int
        }
//                if (Protocol_IsMessageAvailable() == TRUE) {
//                   
//                    unsigned char msg[MAXPAYLOADLENGTH];
//        
//        
//        
//                    switch (Protocol_ReadNextID()) {
//                        case ID_COMMAND_SERVO_PULSE:
//                             LEDS_SET(0b11);
//                            Protocol_GetPayload(&msg);
//                            output.D[0] = msg[1];
//                            output.D[1] = msg[2];
//                            output.D[2] = msg[3];
//                            output.D[3] = msg[4];
//                            output.z = Protocol_IntEndednessConversion(output.z);
//        
//                            RCServo_SetPulse(output.z);
//                            output.z = RCServo_GetPulse();
//                            output.z = Protocol_IntEndednessConversion(output.z);
//                            Protocol_SendMessage((unsigned char) 0x5, ID_SERVO_RESPONSE, &(output.z));
//                            break;
//                        default:
//                            Protocol_GetPayload(&msg);
//                            break;
//        
//                    }
//                }
    }
    while (1);
    BOARD_End();
    return 0;
}
#endif

void __ISR(_OUTPUT_COMPARE_3_VECTOR) __OC3Interrupt(void) {


    // OC3RS = nextPulse;
    //  flag = 1;
    
    IFS0bits.OC3IF = 0;


}

//void __ISR(_TIMER_3_VECTOR, ipl3auto) T3_IntHandler(void) {
//    if ((countIt % 2) == 0) {
//        OC3RS = OC3RS + 100;
//    } else if ((countIt % 2) == 1) {
//        OC3RS = OC3RS - 100;
//    }
//    //    if (BUSY == TRUE) {
//    //        BUSY = FALSE;
//    //        OC3RS = RCServo_SetPulse(thisPulse);
//    //        flag = 1;
//    //    }
//    IFS0bits.T3IF = 0;
//
//
//}

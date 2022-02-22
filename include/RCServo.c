#include <proc/p32mx340f512h.h>
#include <proc/PIC32MX/p32mx340f512h.h>
#include <xc.h>            
#include <sys/attribs.h> 
#include "Protocol.h"
#include "BOARD.h"
#include "RCServo.h"
#include "MessageIDS.h"
#include <stdio.h>

#define RC_SERVO_TEST

#define OC_TRIS TRISDbits.TRISD2 // pin 6
#define OC_PORT PORTDbits.RD2


/*
 * THESE ARE ALREADY DEFINED IN .H
#define RC_SERVO_MIN_PULSE 600
#define RC_SERVO_CENTER_PULSE 1500
#define RC_SERVO_MAX_PULSE 2400
 */
/*******************************************************************************
 * PUBLIC FUNCTIONS                                                           *
 ******************************************************************************/
static unsigned int tickPulse;
//static unsigned char* RC_PAYLOAD;
//static unsigned int countIt;
//static unsigned char SERVO_ID;
static unsigned int thisPulse;
static unsigned int BUSY;
static int flag = 1;

/**
 * @Function RCServo_Init(void)
 * @param None
 * @return SUCCESS or ERROR
 * @brief initializes hardware required and set it to the CENTER PULSE */
int RCServo_Init(void) {
    /* CONFIGURE TIMER3 */
    T3CON = 0x0; // clear control registers
    T3CONbits.TCKPS = 0x05; // set pre-scaler 1:32
    TMR3 = 0; // set current value to zero
    PR3 = 62500; // Set period register for 50ms roll-over

    /* set up timer3 interrupts */
    IPC3bits.T3IP = 3; // timer 3 interrupt priority
    IPC3bits.T3IS = 2; // T3 subpriority
    IFS0bits.T3IF = 0; // clear interrupt flag
    IEC0bits.T3IE = 1; // enable timer 3 interrupts

    /* CONFIGURE OUTPUT COMPARE MODULE */
    OC3CON = 0x0; // clear and disable control register
    OC3CONbits.OCTSEL = 1; // Output compare. This slaves timer3
    OC3CONbits.OCM0 = 1; // PWM mode with Fault pin disabled
    OC3CONbits.OCM = 5;
    OC3CONbits.OC32 = 0;
     OC_TRIS = 0;
    //OC3RS = 1249; // this sets a 2% duty cycle 
    OC3R = 1000; // this sets initial duty cycle
   // OC3RS = 1875;

//    IFS0bits.OC3IF = 0; // clear the interrupt flag
//    IPC3bits.OC3IP = 3;
//    IEC0bits.OC3IE = 1;


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
    if (RC_SERVO_MIN_PULSE <= inPulse <= RC_SERVO_MAX_PULSE) {
        thisPulse = ((inPulse * 125) / 100);
        BUSY = TRUE;
        return SUCCESS;
    } else {
        return ERROR;
    }
}

/**
 * @Function int RCServo_GetPulse(void)
 * @param None
 * @return Pulse in microseconds currently set */
unsigned int RCServo_GetPulse(void) {
    return ((thisPulse * 100) / 125);
}

/**
 * @Function int RCServo_GetRawTicks(void)
 * @param None
 * @return raw timer ticks required to generate current pulse. */
unsigned int RCServo_GetRawTicks(void) {
    return thisPulse;
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

    while (1) {
        //  countIt++;

        if (Protocol_IsMessageAvailable() == TRUE) {
            SERVO_ID = Protocol_ReadNextID();
            if (SERVO_ID == ID_COMMAND_SERVO_PULSE) {
                Protocol_GetPayload(&servoPayload);
                servoPayload = Protocol_IntEndednessConversion(servoPayload);
                prevPulse = thisPulse;

                RCServo_SetPulse(servoPayload);
                if ((prevPulse != thisPulse) && flag == 1) {
                    prevPulse = Protocol_IntEndednessConversion(servoPayload);
                    Protocol_SendMessage(4, ID_SERVO_RESPONSE, &servoPayload);
                    flag = 0;
                } else {
                    Protocol_GetPayload(&trash);
                }

                //                for (int j = 0; j < 400; j++) { // delay for at least 10us before resetting the interrupt flag
                //                    asm(" nop ");
                //                }
            }

            // LEDS_SET(0xf);
        }
        Protocol_IsError();
    }
    while (1);
    BOARD_End();
    return 0;
}
#endif

void __ISR(_OUTPUT_COMPARE_3_VECTOR) __OC3Interrupt(void) {
    // clear the flag so we are not stuck in the ISR
    // Since it runs in the background and make sure its not running infinte
    IFS0bits.OC3IF = 0;
}

void __ISR(_TIMER_3_VECTOR, ipl3auto) T3_IntHandler(void) {

    if (BUSY == TRUE) {
        BUSY = FALSE;
        OC3RS = RCServo_SetPulse(thisPulse);
        flag = 1;
    }
    IFS0bits.T3IF = 0;


}






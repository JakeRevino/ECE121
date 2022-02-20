#include <proc/p32mx340f512h.h>
#include "BOARD.h"
#include "MessageIDs.h"
#include "RCServo.h"
#include "Protocol.h"
#include <sys/attribs.h>




/*
 * THESE ARE ALREADY DEFINED IN .H
#define RC_SERVO_MIN_PULSE 600
#define RC_SERVO_CENTER_PULSE 1500
#define RC_SERVO_MAX_PULSE 2400
 */
/*******************************************************************************
 * PUBLIC FUNCTIONS                                                           *
 ******************************************************************************/

/**
 * @Function RCServo_Init(void)
 * @param None
 * @return SUCCESS or ERROR
 * @brief initializes hardware required and set it to the CENTER PULSE */
int RCServo_Init(void) {
    /* CONFIGURE TIMER3 */
    T3CON = 0x0; // clear control registers
    T3CONbits.TCKPS = 6; // set pre-scaler 1:64
    TMR3 = 0; // set current value to zero
    PR3 = 31249; // Set period register for 50ms roll-over

    /* set up timer3 interrupts */
    IPC3bits.T3IP = 5; // timer 3 interrupt priority
    IPC3bits.T3IS = 3; // T3 subpriority
    IFS0bits.T3IF = 0; // clear interrupt flag
    IEC0bits.T3IE = 1; // enable timer 3 interrupts
    
    /* CONFIGURE OUTPUT COMPARE MODULE */
    OC3CON = 0x0;
    OC3CONbits.OCTSEL = 0; // Output compare. This slaves timer2
    OC3CONbits.OCM = 6;
    OC3RS = 1249;   
    
    /* OUTPUT COMPARE INTERRUPTS */
    
    
    
     T3CONbits.ON = 1; // enable timer3
     OC3CONbits.ON = 1; // enable 

}

/**
 * @Function int RCServo_SetPulse(unsigned int inPulse)
 * @param inPulse, integer representing number of microseconds
 * @return SUCCESS or ERROR
 * @brief takes in microsecond count, converts to ticks and updates the internal variables
 * @warning This will update the timing for the next pulse, not the current one */
int RCServo_SetPulse(unsigned int inPulse) {

}

/**
 * @Function int RCServo_GetPulse(void)
 * @param None
 * @return Pulse in microseconds currently set */
unsigned int RCServo_GetPulse(void) {

}

/**
 * @Function int RCServo_GetRawTicks(void)
 * @param None
 * @return raw timer ticks required to generate current pulse. */
unsigned int RCServo_GetRawTicks(void) {


}

//int main(void) {
//
//
//    return 0;
//}

void __ISR(_OUTPUT_COMPARE_3_VECTOR)__OC3Interrupt(void) {

/******thtth*/


}




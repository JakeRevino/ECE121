#include <proc/p32mx340f512h.h>
#include "BOARD.h"
#include "MessageIDs.h"
#include "RCServo.h"
#include "Protocol.h"
#include <sys/attribs.h>

#define RC_SERVO_TEST




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
static unsigned char* RC_PAYLOAD;
static unsigned int countIt;

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
    PR3 = 31250; // Set period register for 50ms roll-over

    /* set up timer3 interrupts */
    IPC3bits.T3IP = 3; // timer 3 interrupt priority
    IPC3bits.T3IS = 3; // T3 subpriority
    IFS0bits.T3IF = 0; // clear interrupt flag
    IEC0bits.T3IE = 1; // enable timer 3 interrupts

    /* CONFIGURE OUTPUT COMPARE MODULE */
    OC3CON = 0x0; // clear and disable control register
    OC3CONbits.OCTSEL = 1; // Output compare. This slaves timer3
    OC3CONbits.OCM = 6; // PWM mode with Fault pin disabled
    //OC3RS = 1249; // this sets a 2% duty cycle 
    OC3R = RC_SERVO_CENTER_PULSE; // this sets initial duty cycle
    OC3RS = 600;

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
    unsigned int outPulse = 0;
    // unsigned int temp = inPulse;
    outPulse = (inPulse * 625000); // [us]*[625000 ticks/s]
    outPulse = (outPulse / 1000000); // divide by 1 million to get rid of seconds
    if (outPulse >= RC_SERVO_MAX_PULSE) {
        tickPulse = RC_SERVO_MAX_PULSE;
    } else if (outPulse <= RC_SERVO_MIN_PULSE) {
        tickPulse = RC_SERVO_MIN_PULSE;
    } else {
        tickPulse = outPulse;
    }
    return SUCCESS;

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

#ifdef RC_SERVO_TEST

int main(void) {
    RCServo_Init();
    LEDS_INIT();
    BOARD_Init();
    Protocol_Init();


    while (1) {
        countIt++;
        if (Protocol_IsMessageAvailable() == TRUE) {
            if (ID_COMMAND_SERVO_PULSE == Protocol_ReadNextID()) {
                Protocol_GetPayload(RC_PAYLOAD);
                RCServo_SetPulse((unsigned int) RC_PAYLOAD);
            }
            // LEDS_SET(0xf);
        }
    }

    return 0;
}
#endif

void __ISR(_TIMER_3_VECTOR, ipl3auto) T3_IntHandler(void) {


    if (countIt % 2 == 0) {
        OC3RS = OC3RS + 10;
        LEDS_SET(0b1001001);
    } else if (countIt % 2 == 1) {
        OC3RS = OC3RS - 10;
         LEDS_SET(0b1000001);
    }
   
    IFS0bits.T3IF = 0;

}




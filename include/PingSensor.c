#include <proc/p32mx340f512h.h>
#include <sys/attribs.h>
#include "PingSensor.h"
#include "Protocol.h"
#include "BOARD.h"
#include "MessageIDs.h"


//#define PING_SENSOR_TEST

static unsigned int IC_RisingEdge;
static unsigned int IC_FallingEdge;
static unsigned int dT;
static unsigned int distance;
unsigned short TEMP_distance;
static unsigned short thisDistance;

/* These are the states for determining delta t */
typedef enum {
    RISING, FALLING
} STATES;

/* Initializing the first state */
static STATES state = RISING;

//union myData {
//    unsigned short value;
//    char message[2];
//};

/**
 * @Function PingSensor_Init(void)
 * @param None
 * @return SUCCESS or ERROR
 * @brief initializes hardware for PingSensor with the needed interrupts */
int PingSensor_Init(void) {
    /* CONFIGURE TIMER4 */
    T4CON = 0x0; // clear control registers
    T4CONbits.TCKPS = 6; // set pre-scaler
    TMR4 = 0; // set current value to zero
    PR4 = 37500; // Set period register for 60ms roll-over

    /* set up timer4 interrupts */
    IPC4bits.T4IP = 5; // timer 4 interrupt priority
    IPC4bits.T4IS = 3; // T4 subpriority
    IFS0bits.T4IF = 0; // clear interrupt flag
    IEC0bits.T4IE = 1; // enable timer 4 interrupts
    //

    /* CONFIGURE TIMER2 */
    T2CON = 0x0; // clear control registers
    T2CONbits.TCKPS = 3; // set pre-scaler
    TMR2 = 0; // set current value to zeo
    PR2 = 0xFFFFFF; // Set period register


    /* CONFIGURE INPUT CAPTURE */
    IC3CONbits.ICTMR = 1; // timer 2 is counter source 
    IC3CONbits.ICM = 1; // Edge Detect mode ? every edge (rising and falling)
    IC3CONbits.FEDGE = 1; // Capture rising edge first
    IC3CONbits.ICI = 0; // Interrupt of every capture event
    // IC3CONbits.ON = 1; // Input Capture enabled

    /* CONFIGURE INPUT CAPTURE INTERRUPTS */
    IPC3bits.IC3IP = 6; // Input Capture interrupt priority
    IPC3bits.IC3IS = 3; // Input Capture interrupt sub-priority
    IFS0bits.IC3IF = 0; // Clear interrupt flag

    /* ENABLE */
    IEC0bits.IC3IE = 1; // Enable input capture interrupts
    T2CONbits.ON = 1; // enable timer2
    IC3CONbits.ON = 1; // Input Capture enabled
    T4CONbits.ON = 1; // enable timer4


    return SUCCESS;
}

/**
 * @Function int PingSensor_GetDistance(void)
 * @param None
 * @return Unsigned Short corresponding to distance in millimeters */
unsigned short PingSensor_GetDistance(void) {

    return distance;

}

#ifdef PING_SENSOR_TEST

int main(void) {
    PingSensor_Init();
    BOARD_Init();
    Protocol_Init();
    LEDS_INIT();
    TRISDbits.TRISD1 = 0; // Set Pin7 to output
    TRISDbits.TRISD0 = 0;
    TRISDbits.TRISD10 = 1;
    // inVariable
    char temp[4];

    union {
        unsigned int i;
        char word[5];
    } variable;
    //    
    //    temp[0] = word[3];
    //    temp[1] = word[2];
    //    temp[2] = word[1];
    //    temp[3] = word[0];

    while (1) {
        variable.i = distance;
        //char *message = "sup";
        // Protocol_SendDebugMessage(variable.word);
        variable.i = Protocol_IntEndednessConversion(variable.i);
        Protocol_SendMessage(0x05, ID_PING_DISTANCE, &(variable.word));
        for (int i = 0; i < 4000; i++) { // delay for at least 10us before resetting the interrupt flag
            asm(" nop ");
            ///    IFS0bits.T4IF = 0;

        }
        //  IFS0bits.IC3IF = 1;
        // theData.message[0] = PingSensor_GetDistance();
        //Protocol_SendMessage(0x02, ID_DEBUG, &distance);

    }
    return 0;

}
#endif

void __ISR(_TIMER_4_VECTOR)Timer4IntHandlr(void) {
    if (IFS0bits.T4IF == 1) {
        IFS0bits.T4IF = 0;
        LATDbits.LATD1 = 1; // turn on


        for (int i = 0; i < 400; i++) { // delay for at least 10us before resetting the interrupt flag
            asm(" nop ");
            IFS0bits.T4IF = 0;
        }
        LATDbits.LATD1 = 0; // turn off
    }




}

void __ISR(_INPUT_CAPTURE_3_VECTOR) __IC3Interrupt(void) {
    if (IFS0bits.IC3IF == 1) {
        IFS0bits.IC3IF = 0;
        LEDS_SET(0x01);



        switch (state) {
            case RISING:
                if (PORTDbits.RD10 == 1) {
                    LEDS_SET(0x11);
                    IC_RisingEdge = (0xFFFF & IC3BUF);
                    state = FALLING;
                } else {
                    LEDS_SET(0x10);
                    state = RISING;
                }

                break;

            case FALLING:
                if (PORTDbits.RD10 == 0) {
                    LEDS_SET(0x1111);
                    IC_FallingEdge = (0xFFFF & IC3BUF);
                    if (IC_FallingEdge < IC_RisingEdge) {
                        dT = 0xFFFF - IC_FallingEdge + IC_RisingEdge;
                    } else {
                        LEDS_SET(0x1001);
                        dT = IC_FallingEdge - IC_RisingEdge;
                    }

                    // distance = Protocol_IntEndednessConversion(TEMP_distance);

                    //Data.message[0] = PingSensor_GetDistance();
                    //  Protocol_SendDebugMessage((unsigned char*)distance);

                }
                distance = ((340 * dT) / (10000));
                state = RISING;
                break;

        }
    }

}
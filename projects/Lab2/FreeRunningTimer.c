#include <BOARD.h>
#include <xc.h>
#include <Protocol.h>
#include <sys/attribs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <MessageIDs.h>
#include <circleBuff.h>
#include <proc/p32mx340f512h.h>
#include "FreeRunningTimer.h"

//#define FRT_TEST


unsigned int TIME_MILLIS;
unsigned int TIME_MICRO;

/*
 *@Function FreeRunningTimer_Init(void)
 * @param none
 * @return None.
 * @brief Initializes the timer module */
void FreeRunningTimer_Init(void) {
    T5CON = 0x0; // clear control registers
    T5CONbits.TCKPS = 0b011; // set pre-scale for 1:8... Might have to change later
    TMR5 = 0; // set current value to zeo
    PR5 = 2500; // Set period register. 40 MHz operating f, but minus 1

    /* set up interrupts */
    IPC5bits.T5IP = 6; // timer 5 interrupt priority
    IPC5bits.T5IS = 3;
    IFS0bits.T5IF = 0; // clear interrupt flag
    IEC0bits.T5IE = 1; // enable timer 5 interrupts

    T5CONbits.ON = 1; // enable timer5
}

/*
 *Function : FreeRunningTimer_GetMilliSeconds
 * @param None
 * @return the current MilliSecond Count
 */
unsigned int FreeRunningTimer_GetMilliSeconds(void) {
    return TIME_MILLIS;
}

/**
 * Function: FreeRunningTimer_GetMicroSeconds
 * @param None
 * @return the current MicroSecond Count
 */
unsigned int FreeRunningTimer_GetMicroSeconds(void) {
    return TIME_MICRO;
}

#ifdef FRT_TEST
int main() {
    FreeRunningTimer_Init();
    BOARD_Init();
    Protocol_Init();
    LEDS_INIT();
    TRISDbits.TRISD0 = 0; // Configure as output
    TRISEbits.TRISE0 = 0;

//    char message[] = "HELLO WORLD";
//
//    unsigned int targetMilli;
//    unsigned int targetMicro;
//    targetMilli = (FreeRunningTimer_GetMilliSeconds() + (25000 / 1000));
//    targetMicro = (FreeRunningTimer_GetMicroSeconds() % 1000) + (25000 % 1000);
//    int counter;
//    //unsigned int 
//    while (1) {
//        //        printf("targetMilli: %d\r\n", TIME_MILLIS);
//        //        printf("targetMico: %d\r\n", TIME_MICRO);
//        printf("targetMilli: %d\r\n", targetMilli);
//        printf("targetMico: %d\r\n", targetMicro);
//
//        if ((FreeRunningTimer_GetMilliSeconds() >= targetMilli) && (FreeRunningTimer_GetMicroSeconds() % 1000 >= targetMicro)) {
//            counter++;
//            if (counter % 2 == 0) {
//                LATE = 1;
//            } else if (counter % 2 == 1) {
//                LATE = 0;
//            }
//            // Protocol_SendDebugMessage(message);
//
//
//            //            for (int i = 0; i < 24000; i++) {
//            //                asm(" nop ");
//            //            }
//            // LATE = !blink;
//        }
//        // do the thing
//        // char msg[] = "Work mother fucker";
//        // Protocol_SendDebugMessage(msg);
//        //Protocol_SendMessage(0x06, ID_DEBUG, "Hello");
//        // targetMilli = (FreeRunningTimer_GetMilliSeconds() + (2500 / 1000));
//        // targetMicro = (FreeRunningTimer_GetMicroSeconds() % 1000) + (2500 % 1000);
//    }
    // while (1);
   // BOARD_End();
    return 1;
}
#endif

void __ISR(_TIMER_5_VECTOR) Timer5IntHandler(void) {
    //   char *msg = "Work mother fucker\r\n";
    IFS0bits.T5IF = 0;

    TIME_MILLIS++;
    TIME_MICRO += 1000;
    LATDbits.LATD0 ^= 1;
}
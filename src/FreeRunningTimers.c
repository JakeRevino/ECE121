#include <BOARD.h>
#include <xc.h>
#include <Protocol.h>
#include <sys/attribs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <MessageIDs.h>
#include <circleBuff.h>

void __ISR(_TIMER_5_VECTOR, ipl3auto) Timer5IntHandler(void) {
    char msg[100];
    IFS0bits.T5IF = 0;
    sprintf(msg, "The count was %10u, or %10.8f seconds.\r\n", TMR5, TMR5 / 40000000.0);


}

/**
 * @Function FreeRunningTimer_Init(void)
 * @param none
 * @return None.
 * @brief  Initializes the timer module */
void FreeRunningTimer_Init(void) {
    T5CON = 0x0; // clear control registers
    T5CONbits.TCKPS = 0b011; // set pre-scale for 1:8 ... Might have to change later
    TMR5 = 0; // set current value to zero
    PR5 = 5000; // Set period register

    /* set up interrupts */
    IPC5bits.T5IP = 3; // timer 5 interrupt priority
    IFS0bits.T5IF = 0; // clear interrupt flag
    IEC0bits.T5IE = 1; // enable timer 5 interrupts
    
    T5CONbits.ON = 1; // enable timer5
}

/**
 * Function: FreeRunningTimer_GetMilliSeconds
 * @param None
 * @return the current MilliSecond Count
 */
unsigned int FreeRunningTimer_GetMilliSeconds(void) {
  //  unsigned int time_in_millis = FreeRunningTimer_Init()/1000;

}

/**
 * Function: FreeRunningTimer_GetMicroSeconds
 * @param None
 * @return the current MicroSecond Count
 */
unsigned int FreeRunningTimer_GetMicroSeconds(void) {


}

int main(void) {
    FreeRunningTimer_Init();
    BOARD_Init();
    IFS0bits.T5IF = 1;
    

    while (1);
    return 0;
}

#include <proc/PIC32MX/p32mx340f512h.h>
#include <sys/attribs.h>
#include <stdio.h>
#include "ADCFilter.h"
#include "MessageIDs.h"
#include "Protocol.h"
#include "BOARD.h"
#include "FreeRunningTimer.h"
#include "FrequencyGenerator.h"
#include "NonVolatileMemory.h"


/* Define variables */
#define NUMOFCHANNELS 4
#define NUMOFFILTERS 2
#define MAXPAGELENGTH 64
#define BIT_MAX 1023
#define BIT_FRACTION (BIT_MAX >> 3)
#define PIN0 100
#define PIN1 300
#define PIN2 500
#define PIN3 700
#define TRUE 0x01
#define FALSE 0x00

/* Set-up the dumbass switches */
#define SW1 PORTDbits.RD8
#define SW2 PORTDbits.RD9
#define SW3 PORTDbits.RD10
#define SW4 PORTDbits.RD11

/* Set up states */
#define Switch_Change() ((PORTD >> 8) & 0x07)
#define Switch_Change_Channel() ((PORTD >> 8) & 0x04)
#define Switch_Change_Filter() ((PORTD >> 8) & 0x03)

static short FilterArray[NUMOFCHANNELS][NUMOFFILTERS][FILTERLENGTH];

#ifdef LAB3_APPLICATION
int main(void) {
    BOARD_Init();
    Protocol_Init();
    ADCFilter_Init();
    FreeRunningTimer_Init();
    FrequencyGenerator_Init();
    NonVolatileMemory_Init();
    LEDS_INIT();
    
    unsigned char FilterFrequency[FILTERLENGTH];
    unsigned char pin;
    unsigned char filter;
    unsigned char change;
    unsigned char frequencyState;
    
    unsigned int pinAddress;
    
    uint8_t sw_STATUS = Switch_Change();
    uint8_t sw_CHANNEL = Switch_Change_Channel();
    uint8_t sw_FILTER = Switch_Change_Filter();

    char alive_message[MAXPAYLOADLENGTH];
    sprintf(alive_message, "Lab3 Application online at: %s %s.", __TIME__, __DATE__);
    Protocol_SendDebugMessage(alive_message);

    /* Initialize switches */
    TRISDbits.TRISD8 = 1;
    TRISDbits.TRISD9 = 1;
    TRISDbits.TRISD10 = 1;
    TRISDbits.TRISD11 = 1;
    
    /* Initial switch check */
    if (sw_CHANNEL == 3) {
        pin = 3;
        pinAddress = PIN3;
    } else if (sw_CHANNEL == 2) {
        pin = 2;
        pinAddress = PIN2;
    } else if (sw_CHANNEL == 1) {
        pin = 1;
        pinAddress = PIN1;
    } else if (sw_CHANNEL == 0) {
        pin = 0;
        pinAddress = PIN0;
    }


    return 0;
    while (1);
    BOARD_End();
}
#endif
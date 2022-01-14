/*
 * File:   HelloWorld_main.c
 * Author: jrev1
 *
 * Created on January 11, 2022, 5:34 PM
 */



#include <stdio.h> 
#include <BOARD.h>
#include "xc.h"

// define LED's
#define LED1 TRISEbits.RE7
#define LED2 TRISEbits.RE6
#define LED3 TRISEbits.RE5
#define LED4 TRISEbits.RE4
#define LED5 TRISEbits.RE3  
#define LED6 TRISEbits.RE2
#define LED7 TRISEbits.RE1
#define LED8 TRISEbits.RE0

// define buttons
#define BTN1 PORTFbits.RF1
#define BTN2 PORTDbits.RD5
#define BTN3 PORTDbits.RD6
#define BTN4 PORTDbits.RD7

int main(void) {
    BOARD_Init();
    TRISE = 0x00;            // setting TRISE as output
    LATE = 0b11111111;       // turns on LEDs
    
    PORTD = 0x11;            // setting PORTD as input
    while (BTN1) {
        PORTD = 0x00;
        LATE = 0b11111110;
    }
}

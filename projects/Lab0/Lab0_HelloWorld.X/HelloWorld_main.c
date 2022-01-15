/*
 * File:   HelloWorld_main.c
 * Author: jrev1
 *
 * Created on January 11, 2022, 5:34 PM
 */



#include <stdio.h> 
#include <stdlib.h>
#include <xc.h>
#include "BOARD.h"


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

#define PART1_buttons
#define PART2_leds

#define NOPS_FOR_5MS 5e-3

void main(void) {
    BOARD_Init();
    TRISE = 0x00; // setting port E as output
    TRISD = 0xff; // setting port D as input
    TRISF = 0xff; // setting port F as input

    int i;
    for (i = 0; i < NOPS_FOR_5MS; i++) {
        asm(" nop ");

    }
#ifdef PART1_buttons 
    while (1) {

        if ((BTN1 == 1) || (BTN2 == 1) || (BTN3 == 1) || BTN4 == 1) {
            // TRISF = 0x04;
            LATE = 0b10100101; // only 4 LEDs will turn on 
        } else
            LATE = 0b00000000; // else all LEDs are off
    }
#endif

#ifdef PART2_leds
  /*  while (1) {


    } */
#endif


    while (1);
    BOARD_End();
}

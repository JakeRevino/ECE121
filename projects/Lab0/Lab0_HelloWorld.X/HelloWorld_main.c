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
#define LED1 PORTEbits.RE7
#define LED2 PORTEbits.RE6
#define LED3 PORTEbits.RE5
#define LED4 PORTEbits.RE4
#define LED5 PORTEbits.RE3  
#define LED6 PORTEbits.RE2
#define LED7 PORTEbits.RE1
#define LED8 PORTEbits.RE0

// define buttons
#define BTN1 PORTFbits.RF1
#define BTN2 PORTDbits.RD5
#define BTN3 PORTDbits.RD6
#define BTN4 PORTDbits.RD7

//#define PART1_buttons
#define PART2_leds

#define NOPS_FOR_5MS 240000

void main(void) {
    BOARD_Init();
    TRISE = 0x00; // setting port E as output
    TRISD = 0xff; // setting port D as input
    TRISF = 0xff; // setting port F as input

#ifdef PART1_buttons 
    while (1) {
      //  for (int i = 0; i < NOPS_FOR_5MS; i++) {
        //    asm(" nop ");

        //}
        if ((BTN1 == 1) || (BTN2 == 1) || (BTN3 == 1) || BTN4 == 1) {
            // TRISF = 0x04;
            LATE = 0b10100101; // only 4 LEDs will turn on 
        } else
            LATE = 0b00000000; // else all LEDs are off
    }
#endif

#ifdef PART2_leds
    
    LED1 = 0;
    LED2 = 0;
    LED3 = 0;
    LED4 = 0;
    LED5 = 0;
    LED6 = 0;
    LED7 = 0;
    LED8 = 0;

    while (1) {
        for (int j = 0; j < NOPS_FOR_5MS; j++) {
            asm(" nop ");

        }
        if ((BTN1 == 1) || (BTN2 == 1) || (BTN3 == 1) || BTN4 == 1) {
            PORTE = 0x00;
        }
        if (PORTE > 0xff) {
            PORTE = 0x00;
        }
        PORTE++;
    }
#endif

    while (1);
    BOARD_End();
}

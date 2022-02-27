/* ************************************************************************** */
/** Descriptive File Name

  @Company
    Company Name

  @File Name
    filename.c

  @Summary
    Brief description of the file.

  @Description
    Describe the purpose of this file.
 */
/* ************************************************************************** */


#include <proc/p32mx340f512h.h>
#include "BOARD.h"
#include "MessageIDs.h"
#include "Protocol.h"
#include "FreeRunningTimer.h"
#include "RotaryEncoder.h"



#define CS_TRIS TRISDbits.TRISD7 // pin 37
#define CS_LAT LATDbits.LATD7
#define MOSI_TRIS TRISGbits.TRISG8 // pin 11
#define MOSI_LAT LATGbits.LATG8
#define MISO_TRIS TRISGbits.TRISG7 // pin 12
#define MISO_LAT LATGbits.LATG7
#define SCK_TRIS TRIGbits.TRISG6 // pin 13, Shift Clock
#define SCK_LAT LATGbits.LATG6

#define RotaryEncoder_TEST

int parityBit(int in);

int partityBit(int in) {
    int p = 0;
    int i;
    for (i = 0; i < 0x8000; i++) {
        if (in & i) {
            p = p + 1;
        }
        i = i << 1;
    }
    p = p % 2;
    return p;
    //    in = (in | (p << 15));
}

/* These are already defined
 * 
 * #define ENCODER_BLOCKING_MODE 0
 * #define ENCODER_INTERRUPT_MODE 1
 * 
 * */

/**
 * @Function RotaryEncoder_Init(char interfaceMode)
 * @param interfaceMode, one of the two #defines determining the interface
 * @return SUCCESS or ERROR
 * @brief initializes hardware in appropriate mode along with the needed interrupts */
int RotaryEncoder_Init(char interfaceMode) {
    if (interfaceMode == ENCODER_BLOCKING_MODE) {
        SPI2CON = 0; // clear EVERYTHING in control reg
        //SPI2STA = 0; // clear status register
        SPI2CONbits.MSTEN = 1; // Master Mode - Enable Bit
        SPI2CONbits.MODE32 = 0;
        SPI2CONbits.MODE16 = 1; // 16-bit mode
        // SPI2CONbits.SMP = 1; // Input data is sampled at end of data output time
        SPI2CONbits.CKP = 0; // Clock Polarity, Idle state for clock is low and active is high 
        SPI2CONbits.CKE = 1; // Clock Edge Select, Serial output data changes on transition from idle clock state to active clock state 
        SPI2BRG = 3;

        CS_TRIS = 0; // set slave select (SS) as output
        CS_LAT = 1; // SS set high
        SPI2CONbits.ON = 1; // Turn on SPI
        return SUCCESS;
    } else {
        return ERROR;
    }

}

/**
 * @Function int RotaryEncoder_ReadRawAngle(void)
 * @param None
 * @return 14-bit number representing the raw encoder angle (0-16384) */
unsigned short RotaryEncoder_ReadRawAngle(void) {
    unsigned short i;
    unsigned short data;
    CS_LAT = 0; // Write an output low to CS
    SPI2BUF = 0xFFFF; // Send SPI command: 0xFFFC

    for (i = 0; i < _350ns; i++) { // wait 350ns
        asm("nop");
    }

    CS_LAT = 1; // Write an output high to CS
    unsigned short trash = SPI2BUF; //Discard SPI data that came in
    CS_LAT = 0; // Write an output low to CS
    SPI2BUF = 0xC000; // Send NOP command on SPI

    for (i = 0; i < _350ns; i++) { // wait 350ns
        asm("nop");
    }

    CS_LAT = 1; // Write an output high to CS
    data = SPI2BUF; // Read ANGLECOM register
    return data; // 0x3FFF masks off the first 2 bits
}


#ifdef RotaryEncoder_TEST

int main(void) {
    BOARD_Init();
    Protocol_Init();
    RotaryEncoder_Init(ENCODER_BLOCKING_MODE);
    unsigned short data2send;
    unsigned int j;
    while (1) {
       
        data2send = RotaryEncoder_ReadRawAngle();
        data2send = (data2send & 0x3FFF);
        data2send = Protocol_ShortEndednessConversion(data2send);
        Protocol_SendMessage(2, ID_LAB2_ANGLE_REPORT, &(data2send));
         for (j = 0; j < 100000; j++) { // wait 350ns
            asm("nop");
        }
    }

    /* get fucked */
}
#endif

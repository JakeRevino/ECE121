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
    SPI2CON = 0; // clear EVERYTHING in control reg
    //SPI2STA = 0; // clear status register
    SPI2CONbits.MSTEN = 1; // Master Mode - Enable Bit
    SPI2CONbits.MODE32 = 0;
    SPI2CONbits.MODE16 = 1; // 16-bit mode
    SPI2CONbits.SMP = 1; // Input data is sampled at end of data output time
    SPI2CONbits.CKP = 0; // Clock Polarity, Idle state for clock is low and active is high 
    SPI2CONbits.CKE = 0; // Clock Edge Select, Serial output data changes on transition from idle clock state to active clock state 
    SPI2BRG = CLOCK_5MHZ;
    SPI2CONbits.ON = 1; // Turn on SPI
    TRISDbits.TRISD3 = 0; // set slave select (SS) as output
    LATDbits.LATD3 = 1; // SS set high
    /* Know MOST steps! */


}

/**
 * @Function int RotaryEncoder_ReadRawAngle(void)
 * @param None
 * @return 14-bit number representing the raw encoder angle (0-16384) */
unsigned short RotaryEncoder_ReadRawAngle(void) {



    /* Know no steps... */
}

//int main(void) {
///* get fucked */
//}
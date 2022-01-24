#include <BOARD.h>
#include <xc.h>
#include <Protocol.h>
#include <stdio.h>
#include <stdlib.h>

#define MAIN_TEST
#define Baud_Gen 21

int Protocol_Init(void) {
    // Special Function Registers (SFR)
    U1MODE = 0; // Enable UART for 8-bit data

    //U1BRG = Baud_Gen;
    IFS0bits.U1TXIF = 0; // clear flags for UART1
    //IFS1 = 0; // clear flags to be safe
    U1STA = 0; // clear control registers
    U1TXREG = 0; // init Tx reg
    U1RXREG = 0; // init Rx reg
    U1BRG = 21; // baud rate gen value of 21 gives Baud Rate = 115k
    // End SFR configuration

    // Configure to 8-N-1
    U1MODEbits.PDSEL = 00; // sets 8-data and no parity
    U1MODEbits.STSEL = 0; // sets 1 stop bit

    // Enable Tx, Rx, and UART1
    U1STAbits.UTXEN = 1; // enable Tx bits
    U1STAbits.URXEN = 1; // enable Rx bits
    U1MODEbits.ON = 1; // Turn UART on

    /*
     
     Will need to do more stuff with the interrupts
     
     */

}

#ifdef MAIN_TEST

void main(void) {
    BOARD_Init();
    Protocol_Init();
    U1TXREG = 'J';
    while(1);
    BOARD_End();
}

#endif
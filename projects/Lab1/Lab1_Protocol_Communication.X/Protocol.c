#include <BOARD.h>
#include <xc.h>
#include <Protocol.h>
#include <stdio.h>
#include <stdlib.h>

//#define MAIN_TEST
#define UART_TEST
#define BUFFER_TEST
#define Baud_Gen 21
#define MAX_BUFFER_LENGTH 16

int Protocol_Init(void) {

    //    //Configuring the Mode registers
    //    U1MODEbits.ON = 1; // UART is enabled and will be controlled by UEN & TXEN
    //    U1MODEbits.IREN = 1; // Encoder & Decoder Enabled. Set = to 0 to disable
    //    //U1MODEbits.FRZ = 1; // Freeze in debug mode enables. Set = 0 to disable
    //    U1MODEbits.SIDL = 0; // Continue in idle mode. Set = 1 to make UART stop when it enters idle
    //    U1MODEbits.RTSMD = 1; // UxRTS pin is in Simplex mode. Set = 0 for Flow Control Mode
    //    U1MODEbits.UEN = 0b10; // U1TX, U1RX, ~(U1CTS-Clear To Send), and ~(U1RTS-Ready to Send) are enabled and used
    //    U1MODEbits.WAKE = 1; // Wake Up is enables. Set = 0 to disable
    //    U1MODEbits.LPBACK = 1; // Loopback Mode is enabled/ Set = 0 to disable;
    //    U1MODEbits.ABAUD = 1; // Baud Rate measurement enabled. Set = 0 to disable
    //    U1MODEbits.RXINV = 1; // RX idle state is '0'. Set = 0 to make idle state '1'
    //    U1MODEbits.BRGH = 0; // Standard speed mode (16x Baud clk). Set = 1 for high speed mode (4x Baud clk enabled)
    //    U1MODEbits.PDSEL = 0b00; // 8-bit data, No parity
    //    U1MODEbits.STSEL = 0; // 1 Stop bit

    //    // Configuring the Status and Control Registers
    //    U1STA = 0; // Clear entire Status and Control Reg
    //    U1STAbits.ADM_EN = 1; // Auto Address Detect Mode enabled. Set = 0 to disable
    //    // ADDR - When ADM_EN = 1, this defines the address character to use for auto detect
    //    U1STAbits.UTXISEL = 0b01; // Interrupt is generated when all characters have been transmitted
    //    U1STAbits.UTXINV = 1; // Encoded UxTX idle state is '1'. This is tied to IREN!!!
    //    U1STAbits.URXEN = 1; // Receiver is enabled
    //    U1STAbits.UTXBRK = 1; // Send break on next TX, "Start-bit" followed by 12 zeros, followed by "stop-bit"
    //    U1STAbits.UTXEN = 1; // Transmitter is enabled
    //    U1STAbits.URXISEL = 0b10; // Interrupt occurs when buffer is 75% full
    //    U1STAbits.OERR = 1; // Receive buffer has overflowed
    //    
    /*
     Make sure to check: UTXBF reg status for state of TX buffer
     *                   TRMT reg to check if shift reg is empty
     *                   RIDLE - Receiver Idle bit
     *                   PERR - Parity error status
     *                   FERR - Framing error status
     *                   URXDA - RX buffer data status
     */

    //  U1BRG = 21; // baud rate gen value of 21 gives Baud Rate = 115k

    // INTERRUPTS */


    // Special Function Registers (SFR)
    U1MODE = 1; // Enable UART for 8-bit data

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

    // #define interrupt_flag IFS1.U1TXIF


    //  Will need to do more stuff with the interrupts



}

static struct circBuff{
    unsigned char buffer[MAX_BUFFER_LENGTH];
    int head;
    int tail;

};

void enqueue_CB(unsigned char input) {
    circBuff.head = 0;
    circBuff.tail = 0;
    circBuff.buffer[TAIL] = input;
    circBuff.tail = (circBuff.tail + 1) % MAX_BUFFER_LENGTH;

}

void main(void) {


#ifdef UART_TEST 
    BOARD_Init();
    Protocol_Init();
    while (1) {
        U1TXREG = 'J';
    }
    //while(1);
    BOARD_End();
#endif
    
#ifdef BUFFER_TEST
    unsigned char test_char = 'P';
    struct circBuff TX_Buff;
    
    //TX_Buff.buffer
  
    /*write a test for ur buffer*/
    
    
#endif
}
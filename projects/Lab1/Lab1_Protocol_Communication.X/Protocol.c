#include <BOARD.h>
#include <xc.h>
#include <Protocol.h>
#include <stdio.h>
#include <stdlib.h>

//#define MAIN_TEST
#define Fpb 40000000
#define desired_baud 115200
#define UART_TEST
#define BUFFER_TEST
#define PUTCHAR_TEST
#define MAX_BUFFER_LENGTH 128
char full_msg = 'F';

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

    U1MODE = 1; // Enable UART for 8-bit data
    //IPC6bits.U1IP = 4; // sets interrupt priority
    IFS0bits.U1TXIF = 0; // clear flags for UART1
    U1STA = 0; // clear control registers
    U1BRG = ((Fpb / desired_baud) / 16) - 1; // baud rate gen value of 21 gives Baud Rate = 115k


    IEC0bits.U1RXIE = 1; // enable RX interrupt
    IEC0bits.U1TXIE = 1; // enable TX interrupt

    //IFS1 = 0; // clear flags to be safe

    U1MODEbits.UEN = 2;
    U1STAbits.UTXISEL0 = 1;
    U1STAbits.UTXISEL1 = 0;

    // Configure to 8-N-1
    U1MODEbits.PDSEL = 00; // sets 8-data and no parity
    //U1MODEbits.PDSEL1 = 0;
    U1MODEbits.STSEL = 0; // sets 1 stop bit
    U1STAbits.UTXEN = 1; // enable Tx bits
    U1STAbits.URXEN = 1; // enable Rx bits
    U1MODEbits.ON = 1; // Turn UART on


}

static struct {
    unsigned char data[MAX_BUFFER_LENGTH];
    unsigned int head;
    unsigned int tail;

} CircleBuffer;

//CircleBuffer circleBuffer;

void init_buff(void) {
    //CircleBuffer = {.head = 0, .tail = 0};
    CircleBuffer.head = 0;
    CircleBuffer.tail = 0;
}

char Protocol_IsMessageAvailable(void) {
    if (CircleBuffer.head == CircleBuffer.tail) { // if head == tail then its empty
        return TRUE;
    } else
        return FALSE;
}

char Protocol_IsQueueFull(void) {
    if (CircleBuffer.head == ((CircleBuffer.tail + 1) % MAX_BUFFER_LENGTH)) {
        return TRUE;
    } else
        return FALSE;
}

void enqueue_CB(unsigned char input) {
    if (CircleBuffer.head == (CircleBuffer.tail + 1) % MAX_BUFFER_LENGTH) { // this is checking if its full
        return;
    } else {
        CircleBuffer.data[CircleBuffer.tail] = input; // this is writing the data to the tail
        CircleBuffer.tail = (CircleBuffer.tail + 1) % MAX_BUFFER_LENGTH; // this is incrementing 
    }
}

char dequeue_CB(void) {
    if (CircleBuffer.head == CircleBuffer.tail) {
        return full_msg;
    } else {
        char temp_placeHolder;
        temp_placeHolder = CircleBuffer.data[CircleBuffer.head];
        CircleBuffer.head = (CircleBuffer.head + 1) % MAX_BUFFER_LENGTH;
        return temp_placeHolder;
    }
}

/**
 * @Function char PutChar(char ch)
 * @param ch, new char to add to the circular buffer
 * @return SUCCESS or ERROR
 * @brief adds to circular buffer if space exists, if not returns ERROR
 * @author mdunne */
int PutChar(char ch) {
    if (Protocol_IsQueueFull() == TRUE) {
        return ERROR;
    } else {
        U1TXREG = ch;
        return SUCCESS;
    }

}

void __ISR(_UART1_VECTOR)IntUart1Handler(void);


#ifdef TESTHARNESS

int main(void) {
    char test_char[] = "JAKE";
    BOARD_Init();
    Protocol_Init();
    init_buff();

#ifdef PUTCHAR_TEST
    int i = 0;
    while (i <= sizeof (test_char)) {
        PutChar(test_char[i]);
        i++;
    }
    
#endif
    
    while (1);
    BOARD_End();
    return 0;
}
#endif
//#ifdef UART_TEST 
//    while (1) {
//    BOARD_Init();
//    Protocol_Init();
////    while (1) {
////        U1TXREG = 'J';
////    }
//    //while(1);
//    //BOARD_End();
////#endif
//    
//#ifdef BUFFER_TEST
//    unsigned char test_char[] = "PENIS ";
//    init_buff();
//    int length = sizeof(test_char);
//    int i = 0;
//    PutChar('H');
//   /* while (i < length -1) {
//        if (U1STAbits.TRMT) {
//            U1TXREG = PutChar(test_char[i]);
//            i++;
//        
//        
//        }
//    */ 
//    
//    
//    //for (int i = 0; test_char[i] != EOF; i++) {
//    //enqueue_CB(test_char[i]);
//    
//    
//    //char char_to_put = 'K';
//    
//   // U1TXREG = (test_char[i]);
//    
//    //U1TXREG = test_char[i];
//    }//dequeue_CB();
//    //enqueue_CB(test_char[2]);
//    dequeue_CB();
//    dequeue_CB();
//    //enqueue_CB(test_char[3]);
//   // enqueue_CB(test_char[4]);
//    //int full_or_nah = checkBuff_isFull();
//    
//#endif  
//    //TX_Buff.buffer
//  
//    /*write a test for ur buffer*/
//    
//    
//#endif
//        BOARD_End();
//
//    
//}




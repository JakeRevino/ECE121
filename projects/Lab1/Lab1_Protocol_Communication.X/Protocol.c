#include <BOARD.h>
#include <xc.h>
#include <Protocol.h>
#include <sys/attribs.h>
#include <stdio.h>
#include <stdlib.h>

//#define MAIN_TEST
#define Fpb 40e6 
#define desired_baud 115200
#define BaudRate ((Fpb / desired_baud) / 16) - 1; // baud rate gen value of 21 gives Baud Rate = 115k
#define UART_TEST
#define RXTEMP
#define ECHO_TEST
#define BUFFER_TEST
#define PUTCHAR_TEST
#define MAX_BUFFER_LENGTH 32
//#define TESTHARNESS
char RXVAR;

int Protocol_Init(void) {


    /*
     Make sure to check: UTXBF reg status for state of TX buffer
     *                   TRMT reg to check if shift reg is empty
     *                   RIDLE - Receiver Idle bit
     *                   PERR - Parity error status
     *                   FERR - Framing error status
     *                   URXDA - RX buffer data status
     */



    U1STACLR = 0xff;
    U1BRG = BaudRate;

    U1MODEbits.PDSEL = 0b00;
    // U1MODEbits.PDSEL1 = 0; // sets 8-data and no parity
    U1MODEbits.STSEL = 0; // sets 1 stop bit
    U1MODEbits.UEN = 2; // enable UART

    IPC6bits.U1IP = 4; // Interrupt protocol priority
    IPC6bits.U1IS = 0; // Interrupt sub-protocol priority

    IEC0bits.U1TXIE = 1;
    U1STAbits.UTXISEL = 0b10;
    // U1STAbits.UTXISEL1 = 1;
    U1STAbits.UTXEN = 1;

    IEC0bits.U1RXIE = 1;
    U1STAbits.URXISEL = 0b00;
    //  U1STAbits.URXISEL1 = 0;
    U1STAbits.URXEN = 1;

    U1MODEbits.ON = 1; // Turn UART on



}

static struct {
    char data[MAX_BUFFER_LENGTH];
    int head; // oldest data element
    int tail; // next available free space
    int size; // number of elements in buffer
    int putCharFlag;
    int collision;
    //char RXVAR;

} CircleBuffer;

/* These are some function prototypes */
void init_buff(void);
int check_EmptyBuff(void);
int check_FullBuff(void);
void enqueue_CB(char input);
char dequeue_CB(void);

void init_buff(void) { // init the buffer
    CircleBuffer.head = 0; // set head to 0
    CircleBuffer.tail = 0; // set tail to 0
    // CircleBuffer.size = 0; IDK if I need this
    CircleBuffer.putCharFlag = 0;
    //CircleBuffer.RXVAR = 0;

}

int check_EmptyBuff(void) {
    if (CircleBuffer.head == CircleBuffer.tail) { // if head == tail then its empty
        return 1;
    } else
        return 0;
}

int check_FullBuff(void) {
    if (CircleBuffer.head == ((CircleBuffer.tail + 1) % MAX_BUFFER_LENGTH)) {
        return 1;
    } else
        return 0;
}

void enqueue_CB(char input) { // write to CB
    if (!check_FullBuff()) { // check if there is space in the circle buffer
        CircleBuffer.data[CircleBuffer.tail] = input; // this is writing the data to the tail
        CircleBuffer.tail = (CircleBuffer.tail + 1) % MAX_BUFFER_LENGTH; // this is incrementing 


    }

}

char dequeue_CB(void) { // read from CB
    char temp;
    if (!check_EmptyBuff()) { // check buffer isn't empty, ie. if there is data to dequeue
        temp = CircleBuffer.data[CircleBuffer.head];
        CircleBuffer.head = (CircleBuffer.head + 1) % MAX_BUFFER_LENGTH;
        return temp;
    }

}

int PutChar(char ch) {
    if (!check_FullBuff()) { // check if the buffer is full
        CircleBuffer.putCharFlag = 1;

        enqueue_CB(ch); // put the char on the buffer

    }
    CircleBuffer.putCharFlag = 0;
    if ((U1STAbits.TRMT == 1) /*|| (CircleBuffer.collision == 1)*/) {
        //CircleBuffer.collision = 0;
        IFS0bits.U1TXIF = 1;
        return SUCCESS;
    }




    return ERROR;

}

void __ISR(_UART1_VECTOR)IntUart1Handler(void) {


    IFS0bits.U1RXIF = 0;



    if (IFS0bits.U1TXIF == 1) {

        if (CircleBuffer.putCharFlag == 0) {

            U1TXREG = dequeue_CB(); // value from CB goes into TX reg
            IFS0bits.U1TXIF = 0;
        }

    }


    //  IFS0bits.U1TXIF = 0; // clear interrupt flag

}

#ifdef TESTHARNESS

int main() {
    //while (1) {
    char test_char[] = "Below I type hello:\n ";

    //char test_RXcopy[] = test_char;
    BOARD_Init();
    Protocol_Init();
    init_buff();
#ifdef PUTCHAR_TEST
    int i = 0;
    // writeUART(test_char);
    while (i < (sizeof test_char) - 1) {
        //enqueue_CB(test_char[i]);

        if (U1STAbits.TRMT == 1) {
            PutChar(test_char[i]);
            i++;
        }
    }
#endif
    
#ifdef ECHO_TEST
    
    while (1) { // always running in background
        if (U1STAbits.URXDA == 1) { // buffer full whenever you type 
            PutChar(U1RXREG); // initiate the TX
        }
    }
#endif

#endif

    while (1);
    BOARD_End();
    //return 0;
}




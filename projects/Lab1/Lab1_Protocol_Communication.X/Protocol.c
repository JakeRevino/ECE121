#include <BOARD.h>
#include <xc.h>
#include <Protocol.h>
#include <sys/attribs.h>
#include <stdio.h>
#include <stdlib.h>

/* These are some function prototypes */
void init_buff(void);
int check_EmptyBuff(void);
int check_FullBuff(void);
void enqueue_CB(char input);
char dequeue_CB(void);
unsigned char checkSum(char * payload);

/* Here are some global definitions */
#define Fpb 40e6 
#define desired_baud 115200
#define BaudRate ((Fpb / desired_baud) / 16) - 1; // baud rate gen value of 21 gives Baud Rate = 115k
#define ECHO_TEST
#define BUFFER_TEST
#define PUTCHAR_TEST
#define MAX_BUFFER_LENGTH 32
//#define CHECKSUM_TEST
//#define TESTHARNESS
char RXVAR;
int collision;
int putCharFlag = 0;

/* Making structs for CB and Packet */
static struct {
    char data[MAX_BUFFER_LENGTH];
    int head; // oldest data element
    int tail; // next available free space
    int size; // number of elements in buffer
} CircleBuffer;

int Protocol_SendMessage(unsigned char len, unsigned char ID, void *Payload) {
    PutChar(HEAD);
    unsigned char length = len + 1;
    PutChar(length);
    unsigned char checksum = ID;
    PutChar(ID);
    unsigned char i;
    unsigned char *payload = (unsigned char*)Payload;
    
    for (i = 0; i <= length; ++i) {
        PutChar(payload[i]);
        checksum = checkSum(payload);
    }
    PutChar(TAIL);
    PutChar(checksum);
    PutChar('\r');
    PutChar('\n');
    
}

unsigned char checkSum(char * payload) {
    unsigned char checksum = 0;
    int length = (sizeof (payload)) / (sizeof (payload[0]));
    int i;
    for (i = 0; i < (length - 1); i++) {
        checksum = (checksum >> 1) | (checksum << 7);
        checksum += payload[i];
        checksum &= 0xff;
    }
    return checksum;
}

int Protocol_Init(void) {
    U1STACLR = 0xffff;
    U1BRG = BaudRate;

    U1MODEbits.PDSEL = 0b00; // sets 8-data and no parity
    U1MODEbits.STSEL = 0; // sets 1 stop bit
    U1MODEbits.UEN = 2; // enable UART

    IPC6bits.U1IP = 4; // Interrupt protocol priority
    IPC6bits.U1IS = 0; // Interrupt sub-protocol priority

    IEC0bits.U1TXIE = 1;
    U1STAbits.UTXISEL = 0b10;
    U1STAbits.UTXEN = 1;

    IEC0bits.U1RXIE = 1;
    U1STAbits.URXISEL = 0b00;
    U1STAbits.URXEN = 1;

    U1MODEbits.ON = 1; // Turn UART on

    /*
    Make sure to check: UTXBF reg status for state of TX buffer
     *                   TRMT reg to check if shift reg is empty
     *                   RIDLE - Receiver Idle bit
     *                   PERR - Parity error status
     *                   FERR - Framing error status
     *                   URXDA - RX buffer data status
     */
}

void init_buff(void) { // init the buffer
    CircleBuffer.head = 0; // set head to 0
    CircleBuffer.tail = 0; // set tail to 0
    // CircleBuffer.size = 0; IDK if I need this
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
        putCharFlag = 1;
        enqueue_CB(ch); // put the char on the buffer
    }
    putCharFlag = 0;
    if ((U1STAbits.TRMT == 1) || (collision == 1)) {
        collision = 0;
        IFS0bits.U1TXIF = 1;
        return SUCCESS;
    }
    return ERROR;
}

void __ISR(_UART1_VECTOR)IntUart1Handler(void) {
    if (IFS0bits.U1RXIF == 1) {
        IFS0bits.U1RXIF = 0;
        LEDS_SET(0b11111111);
    }
    if (IFS0bits.U1TXIF == 1) {
        if (putCharFlag == 0) {
            U1TXREG = dequeue_CB(); // value from CB goes into TX reg
            IFS0bits.U1TXIF = 0;
            LEDS_SET(0b11001001);
        } else {
            collision = 1;
        }
    }
}

#ifdef TESTHARNESS

int main() {
    //while (1) {
    char test_char[] = "TX once please....\n ";
    //char test_RXcopy[] = test_char;
    BOARD_Init();
    Protocol_Init();
    init_buff();
    LEDS_INIT();

#ifdef PUTCHAR_TEST
    int i = 0;
    while (i < (sizeof test_char) - 1) {
        if (U1STAbits.TRMT == 1) {
            PutChar(test_char[i]);
            i++;
        }
    }
#endif

#ifdef CHECKSUM_TEST
    char test1[] = "0x817F";
    unsigned char t1_sum = checkSum(test1);
    PutChar(t1_sum);
    //sprintf(t1_sum, "\n")
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




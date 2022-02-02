#include <BOARD.h>
#include <xc.h>
#include <Protocol.h>
#include <sys/attribs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <MessageIDs.h>
#include <circleBuff.h>

/* These are some function prototypes 
void init_buff(struct CircleBuffer *buff);
int check_EmptyBuff(struct CircleBuffer *buff);
int check_FullBuff(struct CircleBuffer *buff);
void enqueue_CB(unsigned char input, struct CircleBuffer *buff);
char dequeue_CB(struct CircleBuffer *buff);
//unsigned char checkSum(char * payload);
 * */

/* Here are some global definitions */
#define Fpb 40e6 
#define desired_baud 115200
#define BaudRate ((Fpb / desired_baud) / 16) - 1 // baud rate gen value of 21 gives Baud Rate = 115k
//#define ECHO_TEST
//#define BUFFER_TEST
#define PUTCHAR_TEST
//#define SENDMSG_TEST
//#define MAX_BUFFER_LENGTH 32
#define maxPAYLOADlength 128
//#define END1 '\r'
//#define END2 '\n'
//#define CHECKSUM_TEST
static int collision = 0;
static int putCharFlag = 0;

/* Making structs for CB and Packet 
struct {
    char data[MAX_BUFFER_LENGTH];
    static int head; // oldest data element
    static int tail; // next available free space
    static int size; // number of elements in buffer
} CircleBuffer;
 * */

static struct CircleBuffer TXCB = {};
static struct CircleBuffer RXCB = {};
//static struct {
//    char RXdata[MAX_BUFFER_LENGTH];
//    int RXhead;
//    int RXtail;
//    int RXsize;
//} RXCB;

static struct {
    unsigned char packHEAD;
    unsigned char packLENGTH;
    unsigned char packPAYLOAD;
    unsigned char packTAIL;
    unsigned char packCHECKSUM;
    unsigned char packEND1;
    unsigned char packEND2;
} PACKET;

int Protocol_Init(void) {
    U1STACLR = 0xff;
    U1BRG = BaudRate;

    U1MODEbits.PDSEL = 0b00; // sets 8-data and no parity
    U1MODEbits.STSEL = 0; // sets 1 stop bit
    U1MODEbits.UEN = 0; // enable UART

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

int Protocol_SendMessage(unsigned char len, unsigned char ID, void *Payload) {
    //char testHEAD = 'H';
    //PutChar(testHEAD);
    PutChar(HEAD);
    unsigned char length = len + 1;
    PutChar(length);
    unsigned char checksum = ID;
    PutChar(ID);
    unsigned char i;
    unsigned char *payload = (unsigned char*) Payload;

    for (i = 0; i <= length; i++) {
        PutChar(payload[i]);
        checksum = Protocol_CalcIterativeChecksum(payload[i], checksum);
    }
    PutChar(TAIL);
    PutChar(checksum);
    PutChar('\r');
    PutChar('\n');
    return SUCCESS;
}

int Protocol_SendDebugMessage(char *Message) {
    return Protocol_SendMessage(strlen(Message), ID_DEBUG, Message);
}

unsigned char Protocol_ReadNextID(void) {
    /* need to get the next ID */
}

int Protocol_GetPayload(void* payload) {
    unsigned char *Payload = (unsigned char*) payload;
    PACKET.packPAYLOAD = *Payload;
    return SUCCESS;
    /* takes in memory location of the payload */
}

char Protocol_IsMessageAvailable(void) {
    if (!check_EmptyBuff(&RXCB)) {
        return TRUE;
    }
}

char Protocol_IsQueueFull(void) {
    if (check_FullBuff(&TXCB)) {
        return TRUE;
    }
}

char Protocol_IsError(void) {
    if (U1STAbits.PERR | U1STAbits.FERR | U1STAbits.OERR) {
        return TRUE;
    }
}

unsigned short Protocol_ShortEndednessConversion(unsigned short inVariable) {
    /* 1 line */

}

unsigned int Protocol_IntEndednessConversion(unsigned int inVariable) {
    /* about 6 lines */

}

unsigned char Protocol_CalcIterativeChecksum(unsigned char charIn, unsigned char curChecksum) {
    curChecksum = (curChecksum >> 1) | (curChecksum << 7);
    curChecksum = curChecksum + charIn;
    return curChecksum;


}

void Protocol_RunReceiveStateMachine(unsigned char charIn) {
//    char MODE = 'WAIT_FOR_HEAD';
//    switch(MODE) {
//        case 'WAIT_FOR_HEAD' :
//            
//            if (charIn == HEAD){
//                
//                MODE = 'GET_HEAD';    
//            }
//            
//            break;
//            
//        case 'GET_HEAD':
//            PACKET.HEAD = charIn;
//            /* getting head */
//            break;
//            
//        case 'GET_LENGTH':
//            /*...*/
//            break;
//            
//        case 'ID':
//            /*....*/
//            break;
//    
//    
//    
//    
//    
//    }


}

/*
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
} */

/*
void init_buff(struct CircleBuffer *buff) { // init the buffer
    buff->head = 0; // set head to 0
    CircleBuffer.tail = 0; // set tail to 0

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
 * */

int PutChar(char ch) {
    if (check_FullBuff(&TXCB)) { // check if the buffer is full
        return ERROR;
    }
        putCharFlag = 1;
        enqueue_CB(ch, &TXCB); // put the char on the buffer
    
    putCharFlag = 0;
    if ((U1STAbits.TRMT == 1) | (collision == 1)) {
        collision = 0;
        IFS0bits.U1TXIF = 1;
       
    }
    return SUCCESS;
}

unsigned char GetChar(void) {

}

void __ISR(_UART1_VECTOR)IntUart1Handler(void) {
    if (IFS0bits.U1RXIF == 1) {

        IFS0bits.U1RXIF = 0;

    }
    if (IFS0bits.U1TXIF == 1) {
       
        if (putCharFlag == 0) {
            U1TXREG = dequeue_CB(&TXCB); // value from CB goes into TX reg
            // LEDS_SET(0b11110011);
             IFS0bits.U1TXIF = 0;
            


        } else {
            collision = 1;
        }
    }
}

#ifdef TESTHARNESS

int main() {
   // while (1) {
        char test_char[] = {'G', 'E', 'T', ' ', 'F', 'U', 'C', 'K', '3', 'D', '\n'};
        BOARD_Init();
        Protocol_Init();
        init_buff(&TXCB);
        init_buff(&RXCB);
       // LEDS_INIT();

#ifdef PUTCHAR_TEST
        int i = 0;
        while (test_char[i] != '\0') {
            while (!U1STAbits.TRMT);
                PutChar(test_char[i]);
                i++;
            
        }
#endif

#ifdef SENDMSG_TEST
        //char testPAYLOAD[] = {'1', '2', '3', '4'};
        Protocol_SendMessage('1', 'D', testPAYLOAD);
#endif

#ifdef CHECKSUM_TEST
        char test1[] = "0x817F";
        unsigned char t1_sum = checkSum(test1);
        PutChar(t1_sum);
        sprintf(t1_sum, "\n")
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





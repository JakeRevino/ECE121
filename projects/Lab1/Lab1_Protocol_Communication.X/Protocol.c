#include <BOARD.h>
#include <xc.h>
#include <Protocol.h>
#include <sys/attribs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <MessageIDs.h>
#include <circleBuff.h>

/*
//These are some function prototypes 
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
#define maxPAYLOADlength 128

/* HERE ARE SOME TEST SUITES */
//#define ECHO_TEST
//#define BUFFER_TEST
//#define PUTCHAR_TEST
//#define SENDMSG_TEST
//#define PACKET_TEST
//#define MAX_BUFFER_LENGTH 32
//#define CHECKSUM_TEST

static int collision = 0;
static int putCharFlag = 0;
static int counter = 0;

static struct CircleBuffer TXCB = {};
static struct CircleBuffer RXCB = {};

static struct {
    unsigned char packHEAD;
    unsigned char packLENGTH;
    unsigned char packID;
    unsigned char packLEDS;
    unsigned char packPAYLOAD;
    unsigned char packTAIL;
    unsigned char packCHECKSUM;
    unsigned char packEND1;
    unsigned char packEND2;
} PACKET;

typedef enum {
    WAIT_FOR_HEAD, GET_HEAD, GET_LENGTH, ID, GET_PAYLOAD, GET_TAIL, COMPARE_CHECKSUMS
} states;

static states MODE = WAIT_FOR_HEAD;

int Protocol_Init(void) {
    U1STACLR = 0xff;
    U1BRG = BaudRate;
    IFS0bits.U1RXIF = 0;
    IFS0bits.U1TXIF = 0;
    U1MODEbits.PDSEL = 0; // sets 8-data and no parity
    U1MODEbits.STSEL = 0; // sets 1 stop bit
    U1MODEbits.UEN = 0; // enable UART
    IPC6bits.U1IP = 3; // Interrupt protocol priority
    IPC6bits.U1IS = 1; // Interrupt sub-protocol priority
    IEC0bits.U1TXIE = 1;
    U1STAbits.UTXISEL = 0;
    U1STAbits.UTXEN = 1;
    IEC0bits.U1RXIE = 1;
    U1STAbits.URXISEL = 2;
    U1STAbits.URXEN = 1;
    U1MODEbits.ON = 1; // Turn UART on
}

int Protocol_SendMessage(unsigned char len, unsigned char ID, void *Payload) {
    PutChar(HEAD);
    //unsigned char length = len + 1;
    PutChar(len);
    unsigned char checksum = ID;
    PutChar(ID);
    unsigned char i;
    //unsigned char *payload = (unsigned char*) Payload;

    for (i = 0; i < len - 1; i++) {
        PutChar(((unsigned char*) Payload)[i]);
        checksum = Protocol_CalcIterativeChecksum(((unsigned char*) Payload)[i], checksum);
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
    if ((!check_EmptyBuff(&RXCB)) | (!check_EmptyBuff(&TXCB))) {
        return TRUE;
    }
}

char Protocol_IsQueueFull(void) {
    if (check_FullBuff(&TXCB) | check_FullBuff(&RXCB)) {
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
    char *checksumError = "Checksums didn't match\n";
    char *tailError = "No tail was given\n";

    switch (MODE) {
        case WAIT_FOR_HEAD:
            //LEDS_SET(0b11111111);
            PACKET.packHEAD = 0;
            PACKET.packLENGTH = 0;
            PACKET.packID = 0;
            PACKET.packPAYLOAD = 0;
            PACKET.packCHECKSUM = 0;
            if (charIn == HEAD) {
                MODE = GET_LENGTH;
                break;
            }

        case GET_LENGTH:
            //LEDS_SET(0b11110000);
            PACKET.packLENGTH = charIn;
            MODE = ID;
            break;

        case ID:
            //LEDS_SET(0b10101010);
            if (charIn == ID_LEDS_SET) {
                PACKET.packLEDS = 1;
                PACKET.packID = charIn;
                MODE = GET_PAYLOAD;
                break;
            } else {
                PACKET.packID = charIn;
                MODE = GET_PAYLOAD;
                break;
            }

        case GET_PAYLOAD:
            //LEDS_SET(0b100000000);
            PACKET.packPAYLOAD = charIn;
            PACKET.packCHECKSUM = Protocol_CalcIterativeChecksum(charIn, PACKET.packCHECKSUM);
            counter++;
            break;
            if (counter == PACKET.packLENGTH) {
                counter = 0;
                MODE = GET_TAIL;
                break;
            }



        case GET_TAIL:
            // LEDS_SET(0b00001000);
            if (charIn == TAIL) {
                MODE = COMPARE_CHECKSUMS;
                break;
            } else {
                Protocol_SendDebugMessage(tailError);
                MODE = WAIT_FOR_HEAD;
                break;
            }
            // break;

        case COMPARE_CHECKSUMS:
            if (PACKET.packCHECKSUM == charIn) {

                if (PACKET.packLEDS == 1) {
                    LEDS_SET(PACKET.packPAYLOAD);
                    PACKET.packLEDS = 0;
                    MODE = WAIT_FOR_HEAD;
                    break;
                }
                enqueue_CB(PACKET.packLENGTH, &RXCB);
                enqueue_CB(PACKET.packID, &RXCB);
                enqueue_CB(PACKET.packPAYLOAD, &RXCB);
                MODE = WAIT_FOR_HEAD;
                break;
            } else {
                Protocol_SendDebugMessage(checksumError);
                MODE = WAIT_FOR_HEAD;
                break;
            }
            //break;
    }
}

int PutChar(char ch) {
    if (check_FullBuff(&TXCB)) { // check if the buffer is full
        //  LEDS_SET(0xff);
        return ERROR;
    }
    putCharFlag = 1;
    enqueue_CB(ch, &TXCB); // put the char on the buffer

    putCharFlag = 0;

    while (!U1STAbits.TRMT);

    if ((U1STAbits.TRMT == 1) || (collision == 1)) {
        collision = 0;
        IFS0bits.U1TXIF = 1;
    }
    return SUCCESS;
}

void __ISR(_UART1_VECTOR)IntUart1Handler(void) {
    if (IFS0bits.U1RXIF == 1) {
        Protocol_RunReceiveStateMachine(U1RXREG);

        IFS0bits.U1RXIF = 0;

    }
    if (IFS0bits.U1TXIF == 1) {

        if (putCharFlag == 0) {
            U1TXREG = dequeue_CB(&TXCB); // value from CB goes into TX reg
            IFS0bits.U1TXIF = 0;

        } else {
            collision = 1;
        }
    }
}

#ifdef TESTHARNESS

int main() {

    char test_char[] = "Please work";
    BOARD_Init();
    char *message = "11111111";
    Protocol_Init();
    init_buff(&TXCB);
    init_buff(&RXCB);
    LEDS_INIT();
    while (1); // {
    // Protocol_SendMessage(0x08, ID_LEDS_SET, message);

    // while (1);
#ifdef PUTCHAR_TEST
    int i = 0;
    while (test_char[i] != '\0') {
        while (!U1STAbits.TRMT);
        PutChar(test_char[i]);
        i++;

    }
#endif

#ifdef SENDMSG_TEST
    char testPAYLOAD[] = {'1', '2', '3', '4'};
    Protocol_SendMessage(4, ID_LEDS_SET, testPAYLOAD);
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

#ifdef PACKET_TEST
    IFS0bits.U1RXIF = 1;

#endif



#endif
    //   }
    while (1);
    BOARD_End();
    //return 0;
}





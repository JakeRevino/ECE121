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

static struct CircleBuffer TXCB = {};
static struct CircleBuffer RXCB = {};

static struct {
    unsigned char packHEAD;
    unsigned char packLENGTH;
    unsigned char packID;
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
}

int Protocol_SendMessage(unsigned char len, unsigned char ID, void *Payload) {
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
            PACKET.packHEAD = 0;
            PACKET.packLENGTH = 0;
            PACKET.packID = 0;
            PACKET.packPAYLOAD = 0;
            PACKET.packCHECKSUM = 0;
            if (charIn == HEAD) {
                Protocol_SendDebugMessage(tailError);
                MODE = GET_HEAD;
            }
            break;

        case GET_HEAD:
            PACKET.packHEAD = charIn;
            MODE = GET_LENGTH;
            break;

        case GET_LENGTH:
            PACKET.packLENGTH = charIn;
            MODE = ID;
            break;

        case ID:
            PACKET.packID = charIn;
            MODE = GET_PAYLOAD;
            break;

        case GET_PAYLOAD:
            PACKET.packPAYLOAD = charIn;
            unsigned char counter = 0;
            while (counter <= PACKET.packLENGTH) {
                PACKET.packCHECKSUM = Protocol_CalcIterativeChecksum(charIn, PACKET.packCHECKSUM);
                counter++;
            }
            MODE = GET_TAIL;
            break;

        case GET_TAIL:
            if (charIn == TAIL) {
                MODE = COMPARE_CHECKSUMS;
            } else {
                Protocol_SendDebugMessage(tailError);
                MODE = WAIT_FOR_HEAD;
            }
            break;

        case COMPARE_CHECKSUMS:
            if (PACKET.packCHECKSUM == charIn) {
                enqueue_CB(PACKET.packLENGTH, &RXCB);
                enqueue_CB(PACKET.packID, &RXCB);
                enqueue_CB(PACKET.packPAYLOAD, &RXCB);
                MODE = WAIT_FOR_HEAD;
            } else {
                Protocol_SendDebugMessage(checksumError);
                MODE = WAIT_FOR_HEAD;
            }
            break;
    }
}

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

void __ISR(_UART1_VECTOR)IntUart1Handler(void) {
    if (IFS0bits.U1RXIF == 1) {
       // unsigned char rx = dequeue_CB(&RXCB);
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
   // while (1) {
        char test_char[] = {'G', 'E', 'T', ' ', 'F', 'U', 'C', 'K', '3', 'D', '\n'};
        BOARD_Init();
        Protocol_Init();
        init_buff(&TXCB);
        init_buff(&RXCB);
        LEDS_INIT();

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

#ifdef PACKET_TEST
            IFS0bits.U1RXIF = 1;
           
#endif



#endif
 //   }
    while (1);
    BOARD_End();
    //return 0;
}





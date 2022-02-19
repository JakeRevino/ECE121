#include "BOARD.h"
#include <xc.h>
#include "Protocol.h"
#include <sys/attribs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "MessageIDs.h"
#include "circleBuff.h"

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
//#define INT_ENDED_TEST


/* These are some static declarations used throughout the code*/
static int collision;
static int putCharFlag;
static int counter;
static int clear;
static unsigned char packHEAD;
static unsigned char packLENGTH;
static unsigned char packID;
static unsigned char packPAYLOAD[maxPAYLOADlength];
static unsigned char packCHECKSUM;
static unsigned char packLEDS;
static unsigned char ledValue;
static int pongFlag;

/* Initializing the 2 EMPTY TX & RX buffer */
static struct CircleBuffer TXCB = {};
static struct CircleBuffer RXCB = {};

/* These are the states in the RXSM */
typedef enum {
    WAIT_FOR_HEAD, GET_LENGTH, GET_ID, GET_PAYLOAD, GET_TAIL,
    COMPARE_CHECKSUMS, DETERMINE_RESPONSE, GET_END1, GET_END2
} states;

/* Initializing the first state */
static states MODE = WAIT_FOR_HEAD;

int Protocol_Init(void) {
    U1MODE = 0; // Clear mode register
    U1STA = 0; // clear status register
    U1BRG = BaudRate; // set the baud rate
    U1MODEbits.PDSEL = 00; // sets 8-data and no parity
    U1MODEbits.STSEL = 0; // sets 1 stop bit

    U1MODEbits.ON = 1; // Turn UART on
    U1MODEbits.UARTEN = 1; // enable UART

    U1STAbits.URXEN = 1; // enable RX
    U1STAbits.UTXEN = 1; // enable TX

    IEC0bits.U1TXIE = 1; // enable TX interrupts
    U1STAbits.UTXISEL = 0b10; // set interrupt to generate when TX buffer becomes empty
    IFS0bits.U1TXIF = 0; // clear TX interrupt flag

    IEC0bits.U1RXIE = 1; // enable RX interrupts
    U1STAbits.URXISEL = 0b00; // set RX interrupts to generate when the RX buffer has at least 1 item
    IFS0bits.U1RXIF = 0; // clear RX interrupt flag

    IPC6bits.U1IP = 7; // Interrupt protocol priority
    IPC6bits.U1IS = 0b010; // Interrupt sub-protocol priority
    return SUCCESS;
}

int Protocol_SendMessage(unsigned char len, unsigned char ID, void *Payload) {
    unsigned char SCORED; // this is being used as a check to make sure PutChar is successful
    SCORED = PutChar(HEAD);
   // printf("%X", HEAD);
    if (SCORED == SUCCESS) {
        SCORED = PutChar(len);
     //   printf("%X", len);
    }

    unsigned char checksum = 0;
    checksum = Protocol_CalcIterativeChecksum(ID, checksum);
    if (SCORED == SUCCESS) {
        SCORED = PutChar(ID);
       // printf("%X", ID);
    }
    unsigned char i;
    unsigned char * plchar = Payload;

    for (i = 0; i < len - 1; i++) { // len -1 because the lenght given includes the ID
        if (SCORED == SUCCESS) {
            SCORED = PutChar(*plchar);
         //   printf("%X", *plchar);
        }
        checksum = Protocol_CalcIterativeChecksum(*plchar, checksum);
        ++plchar;
    }
    if (SCORED == SUCCESS) {
        SCORED = PutChar(TAIL);
       // printf("%X", TAIL);
    }
    if (SCORED == SUCCESS) {
        SCORED = PutChar(checksum);
      //  printf("%X", checksum);
    }
    if (SCORED == SUCCESS) {
        SCORED = PutChar('\r');
        //printf("%X", '\r');
    }
    if (SCORED == SUCCESS) {
        SCORED = PutChar('\n');
       // printf("%X", '\n');
    }
    return SUCCESS;
}

int Protocol_SendDebugMessage(char *Message) {
    unsigned int x;
    unsigned char len = strlen(Message);
    for (x = 0; x <= len; x++) {
        packPAYLOAD[x] = Message[x];
    }
  //  printf("%c", packPAYLOAD[1]);
    Protocol_SendMessage(len + 1, ID_DEBUG, packPAYLOAD);
    return SUCCESS;
    // return Protocol_SendMessage(strlen(Message), ID_DEBUG, Message);
}

unsigned char Protocol_ReadNextID(void) {
    /* need to get the next ID */
}

int Protocol_GetPayload(void* payload) {
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
    if (U1STAbits.PERR | U1STAbits.FERR | U1STAbits.OERR) { // Checking for Parity Error, Framing Error, and Overflow Error
        return TRUE;
    }
}

unsigned short Protocol_ShortEndednessConversion(unsigned short inVariable) {

    /* 1 line */

}

unsigned int Protocol_IntEndednessConversion(unsigned int inVariable) {
    inVariable = ((inVariable >> 24) & 0xFF) | ((inVariable << 8) & 0xFF0000)
            | ((inVariable >> 8) & 0xFF00) | ((inVariable << 24) & 0xFF000000);
    return inVariable;

}

unsigned char Protocol_CalcIterativeChecksum(unsigned char charIn, unsigned char curChecksum) {
    curChecksum = (curChecksum >> 1) + (curChecksum << 7);
    curChecksum = curChecksum + charIn;
    return curChecksum;
}

void Configure_Pong(void* theArray) {
    unsigned char* PONG = (unsigned char*) theArray;
    unsigned int zero_Pos = ((unsigned int) PONG[0]) << 24; // Convert endedness
    unsigned int one_Pos = ((unsigned int) PONG[1]) << 16;
    unsigned int two_Pos = ((unsigned int) PONG[2]) << 8;
    unsigned int three_Pos = ((unsigned int) PONG[3]);
    unsigned int divide = (zero_Pos | one_Pos | two_Pos | three_Pos) >> 1; // divide by 2
    PONG[0] = (divide & 0xFF000000) >> 24; // convert endedness again
    PONG[1] = (divide & 0xFF0000) >> 16;
    PONG[2] = (divide & 0xFF00) >> 8;
    PONG[3] = (divide & 0xFF);
}

void Protocol_RunReceiveStateMachine(unsigned char charIn) {
    char *checksumError = "Checksums didn't match\n";
    char *tailError = "No tail was given\n";


    switch (MODE) {
        case WAIT_FOR_HEAD:
            if (charIn == HEAD) {
                packLENGTH = 0; // resetting the variables used
                packID = 0;
                packCHECKSUM = 0;
                counter = 0;
                ledValue = 0;
                packLEDS = 0;
                MODE = GET_LENGTH;
            }
            break;

        case GET_LENGTH:
            packLENGTH = charIn; // save the length
            MODE = GET_ID;
            break;

        case GET_ID:
            packID = charIn; // save the message ID
            packCHECKSUM = Protocol_CalcIterativeChecksum(charIn, packCHECKSUM);
            if (packID == ID_LEDS_GET) { // check for special case that has no payload
                MODE = GET_TAIL;
            } else {
                MODE = GET_PAYLOAD;
            }
            break;

        case GET_PAYLOAD:
            if ((packID == ID_LEDS_SET) && (counter == 0)) { // ID_LEDS_SET has only 1 item in payload
                packLEDS = charIn; // save the item to be used later
                packCHECKSUM = Protocol_CalcIterativeChecksum(charIn, packCHECKSUM);
                MODE = GET_TAIL;
            } else if ((packID == ID_PING) && (counter == 0)) { // check if the message is a Ping
                packPAYLOAD[counter] = charIn; // save charIn to the payload
                pongFlag = 1; // set flag for Pong
                packCHECKSUM = Protocol_CalcIterativeChecksum(charIn, packCHECKSUM);
                counter++;
            } else if (pongFlag == 1) {
                packPAYLOAD[counter] = charIn;
                packCHECKSUM = Protocol_CalcIterativeChecksum(charIn, packCHECKSUM);
                counter++;
                if (counter == 4) { // 4 = length of PING payload - 1
                    MODE = GET_TAIL;
                }
            } else { // if ID is not some special case, process payload 
                packPAYLOAD[counter] = charIn;
                packCHECKSUM = Protocol_CalcIterativeChecksum(charIn, packCHECKSUM);
                if (counter == packLENGTH - 1) { // packLENGTH - 1 because we already processed the ID
                    MODE = GET_TAIL;
                }
                counter++;
                MODE = GET_PAYLOAD; // re-process payload until 1 of the above cases are met
            }
            break;

        case GET_TAIL:
            if (charIn != TAIL) { // this is making sure we are getting a tail
                Protocol_SendDebugMessage(tailError);
                MODE = WAIT_FOR_HEAD;
            } else {
                MODE = COMPARE_CHECKSUMS; // we got a tail so now we need to make sure checksum is correct
            }
            break;

        case COMPARE_CHECKSUMS:
            if (packCHECKSUM != charIn) { // send an error message if checksums don't match
                Protocol_SendDebugMessage(checksumError);
                MODE = WAIT_FOR_HEAD;

            } else if (packID == ID_LEDS_SET) { // setting the LEDS
                LEDS_SET(packLEDS);
                // MODE = GET_END1;
                MODE = WAIT_FOR_HEAD;

            } else if (packID == ID_LEDS_GET) { // getting the status of the LEDS
                ledValue = LEDS_GET();
                Protocol_SendMessage(2, ID_LEDS_STATE, &ledValue);
                //MODE = GET_END1; 
                MODE = WAIT_FOR_HEAD; // we go back to head because thats the end of the packet

            } else if (pongFlag == 1) { // response to ID_PING
                Configure_Pong(packPAYLOAD); // convert --> shift --> convert
                Protocol_SendMessage(packLENGTH, ID_PONG, packPAYLOAD);
                pongFlag = 0;
                //MODE = GET_END1;
                MODE = WAIT_FOR_HEAD;

            } else {
                enqueue_CB(packLENGTH, &RXCB); // Putting the important parts of packet into RXCB
                enqueue_CB(packID, &RXCB);
                enqueue_CB(*packPAYLOAD, &RXCB);
                //MODE = GET_END1;
                MODE = WAIT_FOR_HEAD;
            }
            break;

            /*        case GET_END1:
                        MODE = GET_END2;
                        break;

                    case GET_END2:
                        MODE = WAIT_FOR_HEAD;
                        break; */
    }
}

int PutChar(char ch) {
    if (check_FullBuff(&TXCB)) { // check if the buffer is full
        clear = 0;
        return ERROR;
    }
    putCharFlag = 1;
    enqueue_CB(ch, &TXCB); // put the char on the buffer
    putCharFlag = 0;

    while (U1STAbits.TRMT == 0);

    if ((U1STAbits.TRMT == 1) | (collision == 1)) { // TX shift reg is empty (last TX is complete)
        collision = 0;
        IFS0bits.U1TXIF = 1;
        clear = 1;
    }
    return SUCCESS;
}

void __ISR(_UART1_VECTOR)IntUart1Handler(void) {
    if (IFS0bits.U1RXIF == 1) { // check if we're here because RX interrupt
        IFS0bits.U1RXIF = 0; // reset the flag
        // while (1) {
        Protocol_RunReceiveStateMachine(U1RXREG); // run RXSM
        // }
    }
    // IFS0bits.U1RXIF = 0;
    if (IFS0bits.U1TXIF == 1) { // check if we're here because TX interrupt
        IFS0bits.U1TXIF = 0; // reset the flag
        if (putCharFlag == 0) {
            if (!check_EmptyBuff(&TXCB)) {
            U1TXREG = dequeue_CB(&TXCB); // value from CB goes into TX reg
            }
            // IFS0bits.U1TXIF = 0;

        } else {
            collision = 1;
        }
    }
}

#ifdef PROTOCOL_TESTHARNESS

int main(void) {

    unsigned char test_char[] = "Please work";
    BOARD_Init();
    char *message = "11111111";
    Protocol_Init();
    init_buff(&TXCB);
    init_buff(&RXCB);
    LEDS_INIT();
    // while (1);
#ifdef PUTCHAR_TEST
    int i = 0;
    while (test_char[i] != '\0') {
        while (!U1STAbits.TRMT);
        PutChar(test_char[i]);
        i++;

    }
#endif

#ifdef INT_ENDED_TEST
    unsigned int theINT = 0xDEADBEEF;
    unsigned int reversedINT = Protocol_IntEndednessConversion(theINT);
    // Protocol_SendDebugMessage(reversedINT);


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
            // PutChar(U1RXREG); // initiate the TX
            IFS0bits.U1RXIF = 1;
        }
    }
#endif

#ifdef PACKET_TEST
    IFS0bits.U1RXIF = 1;

#endif




    //   }
    while (1);
    BOARD_End();
    //return 0;
}
#endif


// is this saving properly!!

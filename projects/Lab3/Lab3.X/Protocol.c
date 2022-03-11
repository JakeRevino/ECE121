#include "BOARD.h"
#include <xc.h>
#include "Protocol.h"
#include <sys/attribs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "MessageIDs.h"
#include "circleBuff.h"
#include "RCServo.h"

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
static unsigned char counter;
static unsigned char index;
static int clear;
static unsigned char packHEAD;
static unsigned char packLENGTH;
static unsigned char packID;
static unsigned char packPAYLOAD[maxPAYLOADlength];
static unsigned char packCHECKSUM;
static unsigned char packLEDS;
static unsigned char ledValue;
static int pongFlag;
static int servoFlag;
static int appFlag;

static int msgAvail;
unsigned char currentLENGTH;
unsigned char currentID;
char Lab3PL[MAXPAYLOADLENGTH];
unsigned char ii;
short weightsPL[MAXPAYLOADLENGTH];

unsigned int rc_PL1;
unsigned int rc_PL2;
unsigned int rc_PL3;
unsigned int rc_PL4;

unsigned int APP_PL;

static unsigned char destinationPL[maxPAYLOADlength];

/* Initializing the 2 EMPTY TX & RX buffer */
static struct CircleBuffer TXCB = {};
static struct CircleBuffer RXCB = {};
static struct CircleBuffer PLCB = {}; // We only need one of these

unsigned char dequeue_Payload(unsigned char *destination, struct CircleBuffer *buff);
void enqueue_Payload(unsigned char *input, unsigned char len, struct CircleBuffer *buff);
unsigned char returnID(struct CircleBuffer *buff);
int check_EmptyRX(struct CircleBuffer *buff);

/* These are the states in the RXSM */
typedef enum {
    WAIT_FOR_HEAD, GET_LENGTH, GET_ID, GET_PAYLOAD, GET_TAIL,
    COMPARE_CHECKSUMS, LAB3_STATE, GET_END1, GET_END2
} states;

/* Initializing the first state */
static states MODE = WAIT_FOR_HEAD;

int Protocol_Init(void) {
    init_buff(&TXCB);
    init_buff(&RXCB);
    init_buff(&PLCB);
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
    while (PutChar(HEAD) == ERROR);
    //        for (int x = 0; x < 4; x++) { // delay for at least 10us before resetting the interrupt flag
    //            asm(" nop ");
    //        }
    while (PutChar(len) == ERROR);
    //        for (int x = 0; x < 4; x++) { // delay for at least 10us before resetting the interrupt flag
    //            asm(" nop ");
    //        }
    unsigned char checksum = 0;
    checksum = Protocol_CalcIterativeChecksum(ID, checksum);
    while (PutChar(ID) == ERROR);
    //        for (int x = 0; x < 4; x++) { // delay for at least 10us before resetting the interrupt flag
    //            asm(" nop ");
    //        }

    unsigned char i;
    unsigned char * plchar = Payload;

    for (i = 0; i < (unsigned int) len - 1; i++) { // len -1 because the lenght given includes the ID
        while (PutChar(*plchar) == ERROR);
        //            for (int x = 0; x < 4; x++) { // delay for at least 10us before resetting the interrupt flag
        //                asm(" nop ");
        //            }
        checksum = Protocol_CalcIterativeChecksum(*plchar, checksum);
        ++plchar;
    }
    //    for (int x = 0; x < 10; x++) { // delay for at least 10us before resetting the interrupt flag
    //        asm(" nop ");
    //    }
    while (PutChar(TAIL) == ERROR);
    //        for (int x = 0; x < 10; x++) { // delay for at least 10us before resetting the interrupt flag
    //            asm(" nop ");
    //        }
    while (PutChar(checksum) == ERROR);
    //        for (int x = 0; x < 4; x++) { // delay for at least 10us before resetting the interrupt flag
    //            asm(" nop ");
    //        }
    while (PutChar('\r') == ERROR);
    //        for (int x = 0; x < 4; x++) { // delay for at least 10us before resetting the interrupt flag
    //            asm(" nop ");
    //        }
    while (PutChar('\n') == ERROR);
    //        for (int x = 0; x < 4; x++) { // delay for at least 10us before resetting the interrupt flag
    //            asm(" nop ");
    //        }
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
    return currentID;
    //    unsigned char thisID = returnID(&PLCB);
    //    return thisID;
}

int Protocol_GetPayload(void* payload) {
    // dequeue_Payload(payload, &packPAYLOAD);
    memcpy(payload, packPAYLOAD, sizeof (packPAYLOAD));
    return SUCCESS;
}

char Protocol_IsMessageAvailable(void) {
    //    if (check_EmptyRX(&PLCB) == 1) {
    if (msgAvail == 1) {
        msgAvail = 0;
        return TRUE;
    } else {
        return FALSE;
    }
    //    if (check_EmptyBuff(&RXCB) == 1) {
    //        return FALSE;
    //    } else {
    //        return TRUE;
    //    }
}

char Protocol_IsQueueFull(void) {
    if (check_FullBuff(&TXCB) || check_FullBuff(&RXCB)) {
        return TRUE;
    }
}

char Protocol_IsError(void) {
    if (U1STAbits.PERR | U1STAbits.FERR | U1STAbits.OERR) { // Checking for Parity Error, Framing Error, and Overflow Error
        return TRUE;
    }
}

unsigned short Protocol_ShortEndednessConversion(unsigned short inVariable) {
    unsigned char front = (inVariable & 0xFF00) >> 8;
    unsigned char back = inVariable & 0x00FF;
    unsigned int result = ((unsigned short) ((back << 8) + front));
    return result;

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
                ii = 0;
                currentLENGTH = 0;
                index = 0;
                ledValue = 0;
                packLEDS = 0;
                MODE = GET_LENGTH;
            }
            break;

        case GET_LENGTH:
            packLENGTH = charIn; // save the length
            currentLENGTH = charIn;
            MODE = GET_ID;
            break;

        case GET_ID:
            packID = charIn; // save the message ID
            packPAYLOAD[index] = charIn;
            index++;
            packCHECKSUM = Protocol_CalcIterativeChecksum(charIn, packCHECKSUM);
            if (packID == ID_LEDS_GET) { // check for special case that has no payload
                MODE = GET_TAIL;
            } else if ((packID == ID_NVM_WRITE_BYTE) || (packID == ID_NVM_READ_BYTE) || (packID == ID_NVM_WRITE_PAGE) || (packID == ID_NVM_READ_PAGE)
                    || (packID == ID_ADC_SELECT_CHANNEL) || (packID == ID_ADC_FILTER_VALUES)) {
                MODE = LAB3_STATE;

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
            } else if (packID == ID_COMMAND_SERVO_PULSE) {
                packPAYLOAD[index] = charIn; // save charIn to the payload
                servoFlag = 1; // set flag for servo
                packCHECKSUM = Protocol_CalcIterativeChecksum(charIn, packCHECKSUM);
                counter++;
                index++;
                if (counter == 4) {
                    MODE = GET_TAIL;
                }
            } else if (packID == ID_LAB2_INPUT_SELECT) {
                packPAYLOAD[index] = charIn;
                enqueue_CB(charIn, &RXCB);
                appFlag = 1;
                packCHECKSUM = Protocol_CalcIterativeChecksum(charIn, packCHECKSUM);
                counter++;
                index++;
                MODE = GET_TAIL;
            } else if (pongFlag == 1) {
                packPAYLOAD[counter] = charIn;
                enqueue_CB(charIn, &RXCB);
                packCHECKSUM = Protocol_CalcIterativeChecksum(charIn, packCHECKSUM);
                counter++;
                if (counter == 4) { // 4 = length of PING payload - 1
                    MODE = GET_TAIL;
                }
            } else { // if ID is not some special case, process payload 
                packPAYLOAD[index] = charIn;
                enqueue_CB(charIn, &RXCB);
                packCHECKSUM = Protocol_CalcIterativeChecksum(charIn, packCHECKSUM);
                if (counter == (packLENGTH - 2)) { // packLENGTH - 1 because we already processed the ID
                    MODE = GET_TAIL;
                } else {
                    counter++;
                    index++;
                    MODE = GET_PAYLOAD; // re-process payload until 1 of the above cases are met
                }
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

            } else if (servoFlag == 1) {
                rc_PL1 = packPAYLOAD[1];
                rc_PL2 = packPAYLOAD[2];
                rc_PL3 = packPAYLOAD[3];
                rc_PL4 = packPAYLOAD[4];
                LEDS_SET(0b110011);
                MODE = WAIT_FOR_HEAD;
            } else if (appFlag == 1) {
                APP_PL = packPAYLOAD[1];
                MODE = WAIT_FOR_HEAD;
            } else {
                msgAvail = 1;
                currentID = packID;
                //enqueue_Payload(packPAYLOAD, packLENGTH - 1, PLCB);
                //MODE = GET_END1;
                MODE = WAIT_FOR_HEAD;
            }
            break;

        case LAB3_STATE:

            if ((packID == ID_NVM_WRITE_BYTE) || (packID == ID_NVM_READ_BYTE) || (packID == ID_NVM_WRITE_PAGE) || (packID == ID_NVM_READ_PAGE) || (packID == ID_ADC_SELECT_CHANNEL) || (packID == ID_ADC_FILTER_VALUES)) {
                Lab3PL[ii] = charIn;
                weightsPL[ii] = charIn;
                packCHECKSUM = Protocol_CalcIterativeChecksum(charIn, packCHECKSUM);
                if (ii < packLENGTH - 2) {
                    MODE = LAB3_STATE;
                    ii++;

                } else {

                    MODE = GET_TAIL;
                }
            }
            //            if (packID == ID_NVM_READ_PAGE) {
            //                Lab3PL[ii] = charIn;
            //                packCHECKSUM = Protocol_CalcIterativeChecksum(charIn, packCHECKSUM);
            //                if (ii < packLENGTH - 2) {
            //                    MODE = LAB3_STATE;
            //                    ii++;
            //
            //                } else {
            //                    MODE = GET_TAIL;
            //                }
            //            }
            //            if (packID == ID_NVM_WRITE_PAGE) {
            //                Lab3PL[ii] = charIn;
            //                packCHECKSUM = Protocol_CalcIterativeChecksum(charIn, packCHECKSUM);
            //                if (ii < packLENGTH - 2) {
            //                    MODE = LAB3_STATE;
            //                    ii++;
            //
            //                } else {
            //                    MODE = GET_TAIL;
            //                }
            //            } else if (packID == ID_NVM_READ_BYTE) {
            //                Lab3PL[ii] = charIn;
            //                packCHECKSUM = Protocol_CalcIterativeChecksum(charIn, packCHECKSUM);
            //                if (ii < packLENGTH - 2) {
            //                    MODE = LAB3_STATE;
            //                    ii++;
            //
            //                } else {
            //                    MODE = GET_TAIL;
            //                }
            //            }
            break;


    }
}

char PutChar(char ch) {
    //    if (check_FullBuff(&TXCB) == 1) { // check if the buffer is full
    //        clear = 0;
    //        return ERROR;
    //    }
    //    putCharFlag = 1;
    //    enqueue_CB(ch, &TXCB); // put the char on the buffer
    //    putCharFlag = 0;
    //
    //    while (U1STAbits.TRMT == 0);
    //
    //    if ((U1STAbits.TRMT == 1) | (collision == 1)) { // TX shift reg is empty (last TX is complete)
    //        collision = 0;
    //        IFS0bits.U1TXIF = 1;
    //        clear = 1;
    //    } 
    //    return SUCCESS;

    if ((check_FullBuff(&TXCB) == 0) | (collision == 1)) { // TX shift reg is empty (last TX is complete)
        putCharFlag = 1;
        enqueue_CB(ch, &TXCB); // put the char on the buffer
        putCharFlag = 0;
        collision = 0;
        IFS0bits.U1TXIF = 1;
        clear = 1;
        return SUCCESS;
    } else {

        return ERROR;
    }
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
            while (check_EmptyBuff(&TXCB) == 0) {
                if (U1STAbits.UTXBF == 0) {
                    unsigned char txin = dequeue_CB(&TXCB); // value from CB goes into TX reg
                    U1TXREG = txin;
                } else {
                    asm("nop");
                    asm("nop");
                    asm("nop");
                    asm("nop");
                    asm("nop");
                    asm("nop");
                }
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

//#define TESTTEST
#ifdef TESTTEST

int main(void) {
    BOARD_Init();
    Protocol_Init();
    unsigned char str[5] = {0, 0, 2, 88};
    Protocol_SendMessage(5, ID_DEBUG, &str);
    while (1);
}
#endif





// is this saving properly!!


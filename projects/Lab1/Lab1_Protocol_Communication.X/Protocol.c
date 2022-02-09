#include <BOARD.h>
#include <xc.h>
#include <Protocol.h>
#include <sys/attribs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <MessageIDs.h>
#include <circleBuff.h>

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

static int collision = 0;
static int putCharFlag = 0;
static int counter = 0;
static int LEDSflag = 0;
static int clear = 0;
static unsigned char packHEAD = 0;
static unsigned char packLENGTH = 0;
static unsigned char packID = 0;
static unsigned char packPAYLOAD = 0;
static unsigned char packCHECKSUM = 0;
static unsigned char packLEDS = 0;
static unsigned int ledsVal[2];
static unsigned char ledValue = 0;

static struct CircleBuffer TXCB = {};
static struct CircleBuffer RXCB = {};

/*
static struct {
    unsigned char packHEAD;
    unsigned char packLENGTH;
    unsigned char packID;
    unsigned char packLEDS;
    unsigned char packPAYLOAD;
    unsigned char tempPAYLOAD;
    unsigned char packTAIL;
    unsigned char packCHECKSUM;
    unsigned char packEND1;
    unsigned char packEND2;
} PACKET; */

typedef enum {
    WAIT_FOR_HEAD, GET_LENGTH, GET_ID, GET_PAYLOAD, GET_TAIL,
    COMPARE_CHECKSUMS, DETERMINE_RESPONSE, GET_END1, GET_END2
} states;

static states MODE = WAIT_FOR_HEAD;

int Protocol_Init(void) {
    //U1STACLR = 0xff;
    U1MODE = 0;
    U1STA = 0;
    U1BRG = BaudRate;
    U1MODEbits.PDSEL = 00; // sets 8-data and no parity
    U1MODEbits.STSEL = 0; // sets 1 stop bit

    U1MODEbits.ON = 1; // Turn UART on
    U1MODEbits.UARTEN = 1; // enable UART

    U1STAbits.URXEN = 1;
    U1STAbits.UTXEN = 1;

    IEC0bits.U1TXIE = 1;
    U1STAbits.UTXISEL = 0b10;
    IFS0bits.U1TXIF = 0;

    IEC0bits.U1RXIE = 1;
    U1STAbits.URXISEL = 0b00;
    IFS0bits.U1RXIF = 0;

    IPC6bits.U1IP = 0b011; // Interrupt protocol priority
    IPC6bits.U1IS = 0b01; // Interrupt sub-protocol priority
    return SUCCESS;
}

int Protocol_SendMessage(unsigned char len, unsigned char ID, void *Payload) {
    unsigned char SCORED = 0;
    SCORED = PutChar(HEAD);
    // unsigned char length = len + 1;
    if (SCORED == SUCCESS) {
        SCORED = PutChar(len);
    }

    unsigned char checksum = Protocol_CalcIterativeChecksum(ID, checksum);
    if (SCORED == SUCCESS) {
        SCORED = PutChar(ID);
    }
    unsigned char i;
    //unsigned char *payload = (unsigned char*) Payload;

    for (i = 0; i < len - 1; i++) {
        if (SCORED == SUCCESS) {
            SCORED = PutChar(((unsigned char*) Payload)[i]);
        }
        checksum = Protocol_CalcIterativeChecksum(((unsigned char*) Payload)[i], checksum);
    }
    if (SCORED == SUCCESS) {
        SCORED = PutChar(TAIL);
    }
    if (SCORED == SUCCESS) {
        SCORED = PutChar(checksum);
    }
    if (SCORED == SUCCESS) {
        SCORED = PutChar('\r');
    }
    if (SCORED == SUCCESS) {
        SCORED = PutChar('\n');
    }
    return SUCCESS;
}

int Protocol_SendDebugMessage(char *Message) {
    return Protocol_SendMessage(strlen(Message), ID_DEBUG, Message);
}

unsigned char Protocol_ReadNextID(void) {
    /* need to get the next ID */
}

int Protocol_GetPayload(void* payload) {
    //    unsigned char *Payload = (unsigned char*) payload;
    //    PACKET.packPAYLOAD = *Payload;
    //    return SUCCESS;
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
    inVariable = ((inVariable >> 24) & 0xFF) | ((inVariable << 8) & 0xFF0000)
            | ((inVariable >> 8) & 0xFF00) | ((inVariable << 24) & 0xFF000000);
    return inVariable;

}

unsigned char Protocol_CalcIterativeChecksum(unsigned char charIn, unsigned char curChecksum) {
    curChecksum = (curChecksum >> 1) + (curChecksum << 7);
    curChecksum = curChecksum + charIn;
    return curChecksum;


}

void Protocol_RunReceiveStateMachine(unsigned char charIn) {
    char *checksumError = "Checksums didn't match\n";
    char *tailError = "No tail was given\n";


    switch (MODE) {
        case WAIT_FOR_HEAD:
            packHEAD = 0;
            packLENGTH = 0;
            packID = 0;
            packPAYLOAD = 0;
            packCHECKSUM = 0;
            counter = 0;
            ledValue = 0;
            packLEDS = 0;
            if (charIn == HEAD) {
                MODE = GET_LENGTH;
            } else {
                MODE = WAIT_FOR_HEAD;
            }
            break;

        case GET_LENGTH:
            packLENGTH = charIn;
            MODE = GET_ID;
            break;

        case GET_ID:
            //            if (counter == 0) { // when counter == 0, we get the ID
            packID = charIn;
            packCHECKSUM = Protocol_CalcIterativeChecksum(charIn, packCHECKSUM);
            if (packID == ID_LEDS_GET) {
                MODE = GET_TAIL;
            } else {
                MODE = GET_PAYLOAD;
            }
            break;

        case GET_PAYLOAD:
            if ((packID == ID_LEDS_SET) && (counter == 0)) {
                packLEDS = charIn;
                packCHECKSUM = Protocol_CalcIterativeChecksum(charIn, packCHECKSUM);
                MODE = GET_TAIL;
            } else {
                packPAYLOAD = charIn;
                packCHECKSUM = Protocol_CalcIterativeChecksum(charIn, packCHECKSUM);
                if (counter == packLENGTH + 1) {
                    MODE = GET_TAIL;
                }
                counter++;
                MODE = GET_PAYLOAD;
            }
            break;

        case GET_TAIL:
            if (charIn != TAIL) {
                Protocol_SendDebugMessage(tailError);
                MODE = WAIT_FOR_HEAD;
                //break;

            } else {
                // PACKET.packTAIL = charIn;
                MODE = COMPARE_CHECKSUMS;
            }
            break;

        case COMPARE_CHECKSUMS:
            if (packCHECKSUM != charIn) {
                Protocol_SendDebugMessage(checksumError);
                MODE = WAIT_FOR_HEAD;

            } else if (packID == ID_LEDS_SET) {
                LEDS_SET(packLEDS);
                MODE = GET_END1;

            } else if (packID == ID_LEDS_GET) {
                //                    //ledsVal[0] = PORTE;
                ledValue = PORTE;
                Protocol_SendMessage(0x02, ID_LEDS_STATE, &ledValue);
                MODE = GET_END1; // we go back to head because thats the end of the packet

            } else {
                enqueue_CB(packLENGTH, &RXCB);
                enqueue_CB(packID, &RXCB);
                enqueue_CB(packPAYLOAD, &RXCB);
                MODE = GET_END1;
            }
            break;

        case GET_END1:
            MODE = GET_END2;
            break;

        case GET_END2:
            MODE = WAIT_FOR_HEAD;
            break;
            /*                            counter++; // incrementing so we don't grab ID again
                                        MODE = GET_PAYLOAD;
                                        // break;
                                    } else if (counter == 1) { // this will be where the payload is that LEDS_SET needs
                                        if (packID == ID_LEDS_SET) { // if the ID is ID_LEDS_SET
                                            packLEDS = charIn; // we want to store the values to set
                                            packCHECKSUM = Protocol_CalcIterativeChecksum(charIn, packCHECKSUM);
                                            MODE = GET_TAIL;
            
                                        } else if (packID == ID_LEDS_GET) { // checking if the ID is to GET LED state ... Maybe could do this right when we get the ID
                                            // if (packCHECKSUM == 0x83) {
                                            //ledsVal[0] = PORTE;
                                            unsigned char ledValue = PORTE;
                                            Protocol_SendMessage(0x02, ID_LEDS_STATE, &ledValue);
                                            MODE = WAIT_FOR_HEAD; // we go back to head because thats the end of the packet
                                            // }
                                        }
            
                                    } else {
                                        packPAYLOAD = charIn;
                                        packCHECKSUM = Protocol_CalcIterativeChecksum(charIn, packCHECKSUM);
                                        MODE = GET_PAYLOAD;
                                        if (counter == (packLENGTH - 1)) {
                                            MODE = GET_TAIL;
                                            //counter++;
                                        } //break;
                                        counter++;
            
                                    }
                                    break;
            
                                case GET_TAIL:
                                    if (charIn != TAIL) {
                                        Protocol_SendDebugMessage(tailError);
                                        MODE = WAIT_FOR_HEAD;
                                        //break;
            
                                    } else {
                                        // PACKET.packTAIL = charIn;
                                        MODE = COMPARE_CHECKSUMS;
                                    }
                                    break;
            
                                case COMPARE_CHECKSUMS:
                                    if (packCHECKSUM != charIn) {
                                        Protocol_SendDebugMessage(checksumError);
                                        MODE = WAIT_FOR_HEAD;
            
                                    } else if (packID == ID_LEDS_SET) {
                                        LEDS_SET(packLEDS);
                                        MODE = WAIT_FOR_HEAD;
                                        //break;
            
                                    }//            else if (packID == ID_LEDS_GET) {
                                        //                //                LEDSflag = 1;
                                        //                //                // LEDS_SET(0xff);
                                        //                ledsVal = LEDS_GET();
                                        //                //                //packPAYLOAD = LEDS_GET();
                                        //                //               //Protocol_SendDebugMessage(LEDS_GET());
                                        //                Protocol_SendMessage(0x01, ID_LEDS_STATE, &ledsVal);
                                        //                MODE = WAIT_FOR_HEAD;
                                        //                //                //    break;
            
                                    else {
                                        enqueue_CB(packLENGTH, &RXCB);
                                        enqueue_CB(packID, &RXCB);
                                        enqueue_CB(packPAYLOAD, &RXCB);
                                        MODE = WAIT_FOR_HEAD;
            
                                    }
                                    break; */

    }

}

int PutChar(char ch) {
    if (check_FullBuff(&TXCB)) { // check if the buffer is full
        //  LEDS_SET(0xff);
        clear = 0;
        return ERROR;
    }
    putCharFlag = 1;
    enqueue_CB(ch, &TXCB); // put the char on the buffer

    putCharFlag = 0;

    while (!U1STAbits.TRMT);

    if ((U1STAbits.TRMT == 1) | (collision == 1)) {
        collision = 0;
        IFS0bits.U1TXIF = 1;
        clear = 1;

    }

    return SUCCESS;
}

void __ISR(_UART1_VECTOR)IntUart1Handler(void) {
    if (IFS0bits.U1RXIF == 1) {
        // while (U1STAbits.URXDA == 0);
        //    
        IFS0bits.U1RXIF = 0;
        // while (1) {
        Protocol_RunReceiveStateMachine(U1RXREG);
        //  }

    }
    // IFS0bits.U1RXIF = 0;


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

    unsigned char test_char[] = "Please work";
    BOARD_Init();
    char *message = "11111111";
    Protocol_Init();
    init_buff(&TXCB);
    init_buff(&RXCB);
    LEDS_INIT();
    // while (1); // {
    //Protocol_SendDebugMessage(test_char);

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



#endif
    //   }
    while (1);
    BOARD_End();
    //return 0;
}



// is this saving properly!!

//#include <stdio.h> 
//#include <xc.h>
//#include <stdlib.h>
//#include <string.h>
//#include <sys/attribs.h>
//#include "BOARD.h"
//#include "Protocol.h"
//#include <proc/PIC32MX/p32mx340f512h.h>
//#include "MessageIDS.h"
//// #include "CircleBuffer.h"
//
//
//
//// #define PROTOCOL_TESTHARNESS
//
//// #define MAX_BUFFER_LENGTH 512
//// #define BUFF_MAX_LEN    (MAXPAYLOADLENGTH*PACKETBUFFERSIZE)
//#define BRG_FOR_IDEAL_RATE 21
//#define MAX_BUFFER_LENGTH  (MAXPAYLOADLENGTH * PACKETBUFFERSIZE)
//
//
//#define CIRCLE_BUFFER_TX 0
//#define CIRCLE_BUFFER_RX 1 
//#define CIRCLE_BUFFER_MSG 2
//
//
//#define TX_IS_TALKING 1
//#define RX_IS_TALKING 2
//#define NOBODY_IS_TALKING 3
//
//typedef enum {
//    CHILL, RECIEVED_HEAD, RECIEVED_LENGTH, RECIEVED_ID,
//    IN_PAYLOAD, RECIEVED_PAYLOAD, RECIEVED_TAIL, RECIEVED_CHECKSUM,
//    FINAL, AFTER_VALID_MESSAGE_LED
//} rx_state;
//
//static char Semaphore = NOBODY_IS_TALKING;
//static rx_state rxState;
//
//void CircleBuffer_Init(char type_of_cb);
//void CircleBuffer_add(char type_of_cb, unsigned char item);
//char CircleBuffer_remove(char type_of_cb);
//char CircleBuffer_isempty(char type_of_cb);
//char CircleBuffer_isfull(char type_of_cb);
//unsigned short CircleBuffer_length(char type_of_cb);
//
//static struct {
//    unsigned short head; // need to be short because index can go up to 640
//    unsigned short tail;
//    unsigned short num_of_elements;
//    unsigned char data [MAX_BUFFER_LENGTH];
//} circleBuffer_tx, circleBuffer_rx;
//
//static struct {
//    unsigned short head; 
//    unsigned short tail;
//    unsigned short num_of_elements;
//    unsigned char data [PACKETBUFFERSIZE];
//} circleBuffer_msg;
//
//static struct {
//    unsigned char current_message_index;
//    unsigned char current_message_checksum;
//    unsigned char current_message_id;
//    unsigned char current_message_length;
//    unsigned char current_message_final;
//
//    unsigned char ID_LEDS_STATE;
//    unsigned char RECIEVED_LED_MODE;
//
//    unsigned char * current_message_payload;
//    // 0 = default, 1 = GET, 2 = SET
//} current_rx_message;
//// circleBuffer_msg.data[circleBuffer_msg.head]
//
//static char error_flag = 0;
//
///**
// * @Function Protocol_Init(void)
// * @param None
// * @return SUCCESS or ERROR
// */
//int Protocol_Init(void) {
//
//    // Prelab Part 3
//    U1MODE = 0;
//    U1BRG = 0;
//    IFS0 = 0; // clear all of the flags for UART1
//    IFS1 = 0; // clear all of the flags just in case
//    U1STA = 0; // clear control registers
//    U1TXREG = 0;
//    U1RXREG = 0;
//    U1BRG = BRG_FOR_IDEAL_RATE; // Baud rate gen = 21 gives baud rate of 115k
//
//    CircleBuffer_Init(CIRCLE_BUFFER_TX);
//    CircleBuffer_Init(CIRCLE_BUFFER_RX);
//    CircleBuffer_Init(CIRCLE_BUFFER_MSG);
//
//    U1STAbits.URXEN = 1; // Enable receiption bit
//    U1STAbits.UTXEN = 1; // Enable transmission bit
//    U1MODEbits.ON = 1; // UART on
//
//    // IE = interrupt enable
//    // Configure UART interrupt for both RX and TX
//    U1STAbits.UTXISEL = 2; // 2 (10 in binary) makes it so it get a TXIF set high 
//    // everytime transmit buffer is empty
//    U1STAbits.URXISEL = 0; // setting up U1 status register
//    // RXIF set high everytime the recieve buffer is NOT empty
//    IPC6bits.U1IP = 4; //set UART interrupt priority to 3
//    IPC6bits.U1IS = 1; //set interrupt subpriority to 1
//    IEC0bits.U1RXIE = 1; //enable RX interrupt
//    IEC0bits.U1TXIE = 1; //enable TX interrupt
//
//}
//
///**
// * @Function int Protocol_SendMessage(unsigned char len, void *Payload)
// * @param len, length of full <b>Payload</b> variable
// * @param Payload, pointer to data, will be copied in during the function
// * @return SUCCESS or ERROR
// */
//int Protocol_SendMessage(unsigned char len, unsigned char ID, void *Payload) {
//    if (Protocol_IsQueueFull()) {
//        error_flag = 3;
//        current_rx_message.current_message_id = 0;
//        Semaphore = NOBODY_IS_TALKING;
//        return ERROR;
//    }
//    char * message = (char *) Payload;
//    char cksum = Protocol_CalcIterativeChecksum(ID, 0);
//    int i;
//    PutChar(HEAD);
//    PutChar(len + 1);
//    PutChar(ID);
//    for (i = 0; i < len; i++) {
//        cksum = Protocol_CalcIterativeChecksum(message[i], cksum);
//        PutChar(message[i]);
//    }
//    PutChar(TAIL);
//    PutChar(cksum);
//    PutChar('\r'); // add pound define later
//    PutChar('\n'); // add pound define later     
//    return SUCCESS;
//
//}
//
///**
// * @Function int Protocol_SendMessage(unsigned char len, void *Payload)
// * @param len, length of full <b>Payload</b> variable
// * @param Payload, pointer to data, will be copied in during the function
// * @return SUCCESS or ERROR
// */
//int Protocol_SendDebugMessage(char *Message) {
//    return Protocol_SendMessage(strlen(Message), ID_DEBUG, Message);
//    //return SUCCESS;
//}
//
///**
// * @Function unsigned char Protocol_ReadNextID(void)
// * @param None
// * @return Reads ID of next Packet
// * @brief Returns ID_INVALID if no packets are available
// * @author mdunne */
//unsigned char Protocol_ReadNextID(void) {
//    if (CircleBuffer_isempty(CIRCLE_BUFFER_RX)) {
//        return ID_INVALID;
//    } else {
//        // return CircleBuffer_remove(CIRCLE_BUFFER_RX);
//        return circleBuffer_rx.data[circleBuffer_rx.head];
//    }
//}
//
///**
// * @Function int Protocol_GetPayload(void* payload)
// * @param payload, Memory location to put payload
// * @return SUCCESS or ERROR
// * @brief 
// * @author mdunne */
//int Protocol_GetPayload(void* payload) {
//    // circleBuffer_msg
//    if (CircleBuffer_isempty(CIRCLE_BUFFER_MSG) || CircleBuffer_isempty(CIRCLE_BUFFER_RX)) {
//        return ERROR;
//    }
//    unsigned char length = CircleBuffer_remove(CIRCLE_BUFFER_MSG) - 1;
//    unsigned char temp_data[length];
//    // 0x0400000
//    // char * message = (char *) payload;
//    //char message[length];
//    /*
//    if (CircleBuffer_isempty(CIRCLE_BUFFER_RX)) {
//        return ERROR;
//    } else {
//    */
//        CircleBuffer_remove(CIRCLE_BUFFER_RX); // To discard the ID
//        int i;
//        for (i = 0; i < length; i++) { 
//            // message[i] = CircleBuffer_remove(CIRCLE_BUFFER_RX);
//            temp_data[i] = CircleBuffer_remove(CIRCLE_BUFFER_RX);
//        }
//        memcpy(payload, temp_data, length);
//        return SUCCESS;
//    // }
//
//}
//
///**
// * @Function char Protocol_IsMessageAvailable(void)
// * @param None
// * @return TRUE if Queue is not Empty
// * @brief 
// * @author mdunne */
//char Protocol_IsMessageAvailable(void) {
//    // without this conditonal, it will break
//    if (CircleBuffer_isempty(CIRCLE_BUFFER_RX) || CircleBuffer_isempty(CIRCLE_BUFFER_MSG)) {
//        return FALSE;
//    } else if (circleBuffer_msg.data[circleBuffer_msg.head] > circleBuffer_rx.num_of_elements) {
//        // return TRUE;
//        return FALSE;
//    } else {
//        return TRUE;
//        //return TRUE;
//    }
//}
//
//char Protocol_IsQueueFull(void) {
//    // if circlebuffer TX is full
//    if (CircleBuffer_isfull(CIRCLE_BUFFER_TX)) {
//        return TRUE;
//    } else {
//        return FALSE;
//    }
//}
//
///**
// * @Function char Protocol_IsError(void)
// * @param None
// * @return TRUE if error
// * @brief Returns if error has occurred in processing, clears on read
// * @author mdunne */
//char Protocol_IsError(void) {
//    unsigned char system_error = 0;
//    if(U1STAbits.FERR == 1 || U1STAbits.PERR == 1 || U1STAbits.OERR == 1)
//    {
//         rxState = CHILL;
//         Protocol_Init();
//         system_error = 1;
//    }
//    if (error_flag != 0) {
//        if (error_flag == 1) {
//            Protocol_SendDebugMessage("Length is too large");
//        } else if (error_flag == 2) {
//            Protocol_SendDebugMessage("BSD checksum did not equal");
//        } else if (error_flag == 3) {
//            Protocol_SendDebugMessage("TX Message queue is full");
//        }
//        error_flag = 0;
//        rxState = CHILL;
//        if(system_error != 1)
//        {
//            Protocol_Init();
//        }
//        return TRUE;
//    } else {
//        return FALSE;
//    }
//    
//}
//
//unsigned short Protocol_ShortEndednessConversion(unsigned short inVariable) {
//    // inVariable << 8; 
//    // int can work 
//    unsigned char most_sig = (inVariable & 0xFF00) >> 8;
//    unsigned char least_sig = inVariable & 0x00FF;
//    unsigned int results = ((unsigned short) ((least_sig << 8) + most_sig));
//    // 
//    return results;
//}
//
//unsigned int Protocol_IntEndednessConversion(unsigned int inVariable) {
//    unsigned short most_sig = (inVariable & 0xFFFF0000) >> 16;
//    unsigned short least_sig = inVariable & 0x0000FFFF;
//    unsigned int results_one = ((Protocol_ShortEndednessConversion(least_sig) << 16));
//    unsigned int results_two = (Protocol_ShortEndednessConversion(most_sig));
//    // return ((unsigned int) ((least_sig << 16) + most_sig));   
//    return results_two + results_one;
//}
//
//unsigned char Protocol_CalcIterativeChecksum(unsigned char charIn, unsigned char curChecksum) {
//    curChecksum = (curChecksum >> 1) + (curChecksum << 7);
//    curChecksum += charIn;
//    return curChecksum;
//}
//
//void Protocol_RunReceiveStateMachine(unsigned char charIn) {
//    // switch is much more effecient in assembly than bunch of if else statements
//    switch (rxState) {
//        case CHILL:
//            /* Obviously, the first thing to do is to wait for the head byte. Until that happens, all incoming data should be
//            ignored. Once the head has been received, a new message is coming in and needs to be stored somewhere.
//             */
//            if (charIn == HEAD) {
//                current_rx_message.current_message_index = 0;
//                current_rx_message.current_message_checksum = 0;
//                current_rx_message.current_message_length = 0;
//                current_rx_message.current_message_id = 0;
//                current_rx_message.ID_LEDS_STATE = 0;
//                current_rx_message.RECIEVED_LED_MODE = 0;
//                rxState = RECIEVED_HEAD;
//                // Semaphore = RX_IS_TALKING;
//            }
//            break;
//        case RECIEVED_HEAD:
//            // Waiting for length
//            if (charIn <= MAXPAYLOADLENGTH) {
//                // Once we recieved the length
//                rxState = RECIEVED_LENGTH;
//                current_rx_message.current_message_length = charIn;
//                // The stored length is stored + 1 as it includes the ID and the payload
//            } else {
//                error_flag = 1;
//                // return ERROR;
//                // add the ERROR function from protocol later
//            }
//            break;
//        case RECIEVED_LENGTH:
//            // Waiting for ID
//            // then we recieved the id
//            current_rx_message.current_message_id = charIn;
//            if (charIn == ID_LEDS_GET) {
//                rxState = RECIEVED_PAYLOAD;
//            } else if (charIn == ID_LEDS_SET) {
//                rxState = RECIEVED_ID;
//            } else {
//                CircleBuffer_add(CIRCLE_BUFFER_RX, charIn);
//                CircleBuffer_add(CIRCLE_BUFFER_MSG, current_rx_message.current_message_length);
//                // first thing we adding in the circle buffer rx is the id
//                // By now, our RX circle buffer looks like this
//                // rx_buffer = [ID]
//                rxState = RECIEVED_ID;
//            }
//
//            current_rx_message.current_message_index += 1;
//            current_rx_message.current_message_checksum = Protocol_CalcIterativeChecksum(charIn, current_rx_message.current_message_checksum);
//
//            break;
//        case RECIEVED_ID:
//            // Waiting for the start of the payload
//            // Charin will be the first character of the payload
//            if (current_rx_message.current_message_id == ID_LEDS_SET) {
//                current_rx_message.ID_LEDS_STATE = charIn;
//                rxState = RECIEVED_PAYLOAD;
//            } else {
//                rxState = IN_PAYLOAD;
//                CircleBuffer_add(CIRCLE_BUFFER_RX, charIn);
//                // Here, we adding each character from the payload.
//                // By now, our RX circle buffer looks like this
//                // rx_buffer = [ID, Payload[0]]
//            }
//            current_rx_message.current_message_checksum = Protocol_CalcIterativeChecksum(charIn, current_rx_message.current_message_checksum);
//            current_rx_message.current_message_index += 1;
//            break;
//        case IN_PAYLOAD:
//            // Remianing characters of the payload
//            CircleBuffer_add(CIRCLE_BUFFER_RX, charIn);
//            current_rx_message.current_message_checksum = Protocol_CalcIterativeChecksum(charIn, current_rx_message.current_message_checksum);
//            current_rx_message.current_message_index += 1;
//            if (current_rx_message.current_message_index == current_rx_message.current_message_length) {
//                rxState = RECIEVED_PAYLOAD;
//            }
//            break;
//            // By now, our RX circle buffer looks like this
//            // rx_buffer = [ID, Payload[0], Payload[1], ...]
//            // msg_buffer = [len(message), ...]
//        case RECIEVED_PAYLOAD:
//            // After we
//            // CircleBuffer_rx must contain the id + entire payload, thats it
//            if (charIn == TAIL) {
//                rxState = RECIEVED_TAIL;
//            }
//            break;
//        case RECIEVED_TAIL:
//            // we need to check the checksum from the client
//            if (charIn == current_rx_message.current_message_checksum) {
//                rxState = FINAL;
//
//            } else {
//                error_flag = 2;
//                // return error
//            }
//            current_rx_message.current_message_final = 0;
//            break;
//        case FINAL:
//            if (current_rx_message.current_message_final == 0) {
//                if (charIn == '\r') {
//                    current_rx_message.current_message_final = 1;
//                }
//            } else if (current_rx_message.current_message_final == 1) {
//                if (charIn == '\n') {
//
//                    if (current_rx_message.current_message_id == ID_LEDS_SET) {
//                        LEDS_SET(current_rx_message.ID_LEDS_STATE);
//                    } else if (current_rx_message.current_message_id == ID_LEDS_GET) {
//                        current_rx_message.ID_LEDS_STATE = LEDS_GET();
//                        Protocol_SendMessage(1, ID_LEDS_STATE, &current_rx_message.ID_LEDS_STATE);
//                    }
//
//
//                    current_rx_message.current_message_id = 0;
//                    current_rx_message.ID_LEDS_STATE = 0;
//                    current_rx_message.RECIEVED_LED_MODE = 0;
//
//                    Semaphore = NOBODY_IS_TALKING;
//                    rxState = CHILL;
//                    // COMPLETE!!
//                }
//            }
//            break;
//
//    }
//
//
//
//}
//
///*
// The first step to testing your serial communication is to send a single character to your Putty serial terminal
//    on the PC. In order to do this, you need to define a function, PutChar, which will put a single character in the
//    UART transmit buffer which enables the transmission. The function prototype is already in Protocol.h.
// */
//
///**
// * @Function char PutChar(char ch)
// * @param ch, new char to add to the circular buffer
// * @return SUCCESS or ERROR
// * @brief adds to circular buffer if space exists, if not returns ERROR
// * @author mdunne */
//char PutChar(char ch) {
//
//    /*
//     * U1STAbits.UTXBF referring to the actual transmit buffer the part UART1 system
//     * Shift register
//     */
//    // if not full
//    if (CircleBuffer_isfull(CIRCLE_BUFFER_TX) == 0) {
//        CircleBuffer_add(CIRCLE_BUFFER_TX, ch);
//        IFS0bits.U1TXIF = 1; // setting the interruption flag
//        // Specific for the UAR1T system
//        return SUCCESS;
//    } else {
//        return ERROR;
//    }
//
//}
//
//// easy way to verify if you are correctly calculating it
//
//unsigned char BSDchecksum(char * str) {
//    unsigned char cksum = 0;
//    for (int i = 0; i < strlen(str); i++) {
//        cksum = (cksum >> 1) + (cksum << 7);
//        cksum += str[i];
//    }
//    return cksum;
//}
//
//// THis function runs continousely 24/7
//// Specifcally when there is an UART1 flag is set, this function will run.
//
//void __ISR(_UART1_VECTOR) IntUart1Handler(void) {
//    // Goes high when recieve buffer recieves a single byte
//    if (IFS0bits.U1RXIF && (Semaphore != TX_IS_TALKING)) {
//        /*
//        PutChar(U1RXREG + 1);
//         Just for debugging
//         */
//        IFS0bits.U1RXIF = 0;
//        if (error_flag == 0) {
//            Semaphore = RX_IS_TALKING;
//            // CircleBuffer_add(CIRCLE_BUFFER_RX, U1RXREG);
//            // U1RXREG is the char that just came in
//            Protocol_RunReceiveStateMachine(U1RXREG);
//            Semaphore = NOBODY_IS_TALKING;
//        } else {
//            Protocol_IsError();
//        }
//        
//
//    }
//    if (IFS0bits.U1TXIF && (Semaphore != RX_IS_TALKING)) {
//
//        // if hardware buffer is not full and the circlebuffer is not empty
//        if (!U1STAbits.UTXBF && !CircleBuffer_isempty(CIRCLE_BUFFER_TX)) // Interrupt Driven Transmission
//        {
//            Semaphore = TX_IS_TALKING;
//            U1TXREG = CircleBuffer_remove(CIRCLE_BUFFER_TX);
//        }
//        // 1 = Transmit shift register is empty and transmit buffer is empty (the last transmission has completed
//        if (U1STAbits.TRMT) {
//            IFS0bits.U1TXIF = 0; // clearing the interrupt flag
//            Semaphore = NOBODY_IS_TALKING;
//        }
//
//    }
//
//
//
//}
//
//void CircleBuffer_Init(char type_of_cb) {
//    if (type_of_cb == CIRCLE_BUFFER_TX) {
//        circleBuffer_tx.head = 0;
//        circleBuffer_tx.tail = 0;
//        circleBuffer_tx.num_of_elements = 0;
//    } else if (type_of_cb == CIRCLE_BUFFER_RX) {
//        circleBuffer_rx.head = 0;
//        circleBuffer_rx.tail = 0;
//        circleBuffer_rx.num_of_elements = 0;
//    } else if (type_of_cb == CIRCLE_BUFFER_MSG) {
//        circleBuffer_msg.head = 0;
//        circleBuffer_msg.tail = 0;
//        circleBuffer_msg.num_of_elements = 0;
//    }
//
//}
//
//void CircleBuffer_add(char type_of_cb, unsigned char item) {
//    if(!CircleBuffer_isfull(type_of_cb))
//    {
//        if (type_of_cb == CIRCLE_BUFFER_TX) {
//           circleBuffer_tx.data[circleBuffer_tx.tail] = item;
//           circleBuffer_tx.tail = (circleBuffer_tx.tail + 1) % MAX_BUFFER_LENGTH;
//           circleBuffer_tx.num_of_elements++;
//       } else if (type_of_cb == CIRCLE_BUFFER_RX) {
//           circleBuffer_rx.data[circleBuffer_rx.tail] = item;
//           circleBuffer_rx.tail = (circleBuffer_rx.tail + 1) % MAX_BUFFER_LENGTH;
//           circleBuffer_rx.num_of_elements++;
//       } else if (type_of_cb == CIRCLE_BUFFER_MSG) {
//           circleBuffer_msg.data[circleBuffer_msg.tail] = item;
//           circleBuffer_msg.tail = (circleBuffer_msg.tail + 1) % PACKETBUFFERSIZE;
//           circleBuffer_msg.num_of_elements++;
//       }       
//    }
//
//}
//
//char CircleBuffer_remove(char type_of_cb) {
//    if (type_of_cb == CIRCLE_BUFFER_TX) {
//        char item_will_removed = (circleBuffer_tx.data)[circleBuffer_tx.head];
//        circleBuffer_tx.head = (circleBuffer_tx.head + 1) % MAX_BUFFER_LENGTH;
//        circleBuffer_tx.num_of_elements--;
//        return item_will_removed;
//    } else if (type_of_cb == CIRCLE_BUFFER_RX) {
//        char item_will_removed = (circleBuffer_rx.data)[circleBuffer_rx.head];
//        circleBuffer_rx.head = (circleBuffer_rx.head + 1) % MAX_BUFFER_LENGTH;
//        circleBuffer_rx.num_of_elements--;
//        return item_will_removed;
//    } else if (type_of_cb == CIRCLE_BUFFER_MSG) {
//        char item_will_removed = (circleBuffer_msg.data)[circleBuffer_msg.head];
//        circleBuffer_msg.head = (circleBuffer_msg.head + 1) % PACKETBUFFERSIZE;
//        circleBuffer_msg.num_of_elements--;
//        return item_will_removed;
//    }
//}
//
//char CircleBuffer_isempty(char type_of_cb) {
//    if (type_of_cb == CIRCLE_BUFFER_TX) {
//        return circleBuffer_tx.num_of_elements == 0;
//    } else if (type_of_cb == CIRCLE_BUFFER_RX) {
//        return circleBuffer_rx.num_of_elements == 0;
//    } else if (type_of_cb == CIRCLE_BUFFER_MSG) {
//        return circleBuffer_msg.num_of_elements == 0;
//    }
//}
//
//char CircleBuffer_isfull(char type_of_cb) {
//    if (type_of_cb == CIRCLE_BUFFER_TX) {
//        return circleBuffer_tx.num_of_elements >= MAX_BUFFER_LENGTH;
//    } else if (type_of_cb == CIRCLE_BUFFER_RX) {
//        return circleBuffer_rx.num_of_elements >= MAX_BUFFER_LENGTH;
//    } else if (type_of_cb == CIRCLE_BUFFER_MSG) {
//        return circleBuffer_msg.num_of_elements >= PACKETBUFFERSIZE;
//    }
//}
//
//unsigned short CircleBuffer_length(char type_of_cb) {
//    if (type_of_cb == CIRCLE_BUFFER_TX) {
//        return circleBuffer_tx.num_of_elements;
//    } else if (type_of_cb == CIRCLE_BUFFER_RX) {
//        return circleBuffer_rx.num_of_elements;
//    } else if (type_of_cb == CIRCLE_BUFFER_MSG) {
//        return circleBuffer_msg.num_of_elements;
//    }
//}
//
//
//#ifdef PROTOCOL_TESTHARNESS
//
//int main() {
//    BOARD_Init();
//    Protocol_Init();
//    LEDS_INIT();
//
//    char test_message[MAXPAYLOADLENGTH];
//    sprintf(test_message, "Protocol test message time at %s %s", __TIME__, __DATE__);
//    Protocol_SendDebugMessage(test_message);
//    Protocol_SendMessage(5, ID_DEBUG, "hello");
//
//
//    // unsigned int payload;
//    unsigned char saved_next_id;
//    unsigned int payload = 0;
//    while (1) {
//        if (Protocol_IsMessageAvailable()) {
//            saved_next_id = Protocol_ReadNextID();
//            if (saved_next_id == ID_PING) {
//                Protocol_GetPayload(&payload);
//                payload = Protocol_IntEndednessConversion(payload);
//                payload = payload >> 1;
//                payload = Protocol_IntEndednessConversion(payload);
//                Protocol_SendMessage(4, ID_PONG, &payload);
//            }
//            // Protocol_IsError();
//        }
//
//
//    }
//    while (1);
//    BOARD_End();
//}
//
//#endif


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
    COMPARE_CHECKSUMS, DETERMINE_RESPONSE, GET_END1, GET_END2
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
    unsigned char thisID = returnID(&PLCB);
    return thisID;
}

int Protocol_GetPayload(void* payload) {
    dequeue_Payload(payload, &PLCB);
    return SUCCESS;
}

char Protocol_IsMessageAvailable(void) {
    //    if (check_EmptyRX(&PLCB) == 1) {
    //    if (msgAvail == 1) {
    //        msgAvail = 0;
    //        return TRUE;
    //    } else { 
    //        return FALSE;
    //    }
    if (check_EmptyBuff(&RXCB) == 0) {
        return FALSE;
    } else {
        return TRUE;
    }
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
                index = 0;
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
            packPAYLOAD[index] = charIn;
            index++;
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
                //enqueue_Payload(packPAYLOAD, packLENGTH - 1, PLCB);
                //MODE = GET_END1;
                MODE = WAIT_FOR_HEAD;
            }
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





//#include "BOARD.h"
//#include <xc.h>
//#include "Protocol.h"
//#include <sys/attribs.h>
//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//#include "MessageIDs.h"
//#include "circleBuff.h"
//#include "RCServo.h"
//
///* Here are some global definitions */
//#define Fpb 40e6 
//#define desired_baud 115200
//#define BaudRate ((Fpb / desired_baud) / 16) - 1 // baud rate gen value of 21 gives Baud Rate = 115k
//#define maxPAYLOADlength 128
//
///* HERE ARE SOME TEST SUITES */
////#define ECHO_TEST
////#define BUFFER_TEST
////#define PUTCHAR_TEST
////#define SENDMSG_TEST
////#define PACKET_TEST
////#define MAX_BUFFER_LENGTH 32
////#define CHECKSUM_TEST
////#define INT_ENDED_TEST
//
//
///* These are some static declarations used throughout the code*/
//static int collision;
//static int putCharFlag;
//static int counter;
//static int clear;
//static unsigned char packHEAD;
//static unsigned char packLENGTH;
//static unsigned char packID;
//static unsigned char packPAYLOAD[maxPAYLOADlength];
//static unsigned char packCHECKSUM;
//static unsigned char packLEDS;
//static unsigned char ledValue;
//static int pongFlag;
//static char errorFlag = 0;
//static unsigned int servoFlag;
//
//unsigned char rc_PL1;
//unsigned char rc_PL2;
//unsigned char rc_PL3;
//unsigned char rc_PL4;
//
//
///* Initializing the 2 EMPTY TX & RX buffer */
//static struct CircleBuffer TXCB = {};
//static struct CircleBuffer RXCB = {};
//
///* These are the states in the RXSM */
//typedef enum {
//    WAIT_FOR_HEAD, GET_LENGTH, GET_ID, GET_PAYLOAD, GET_TAIL,
//    COMPARE_CHECKSUMS, DETERMINE_RESPONSE, GET_END1, GET_END2
//} states;
//
///* Initializing the first state */
//static states MODE = WAIT_FOR_HEAD;
//
//int Protocol_Init(void) {
//    init_buff(&RXCB);
//    init_buff(&TXCB);
//    BOARD_Init();
//    LEDS_INIT();
//    U1MODE = 0; // Clear mode register
//    U1STA = 0; // clear status register
//    U1BRG = BaudRate; // set the baud rate
//    U1MODEbits.PDSEL = 00; // sets 8-data and no parity
//    U1MODEbits.STSEL = 0; // sets 1 stop bit
//
//    U1MODEbits.ON = 1; // Turn UART on
//    U1MODEbits.UARTEN = 1; // enable UART
//
//    U1STAbits.URXEN = 1; // enable RX
//    U1STAbits.UTXEN = 1; // enable TX
//
//    IEC0bits.U1TXIE = 1; // enable TX interrupts
//    U1STAbits.UTXISEL = 0b10; // set interrupt to generate when TX buffer becomes empty
//    IFS0bits.U1TXIF = 0; // clear TX interrupt flag
//
//    IEC0bits.U1RXIE = 1; // enable RX interrupts
//    U1STAbits.URXISEL = 0b00; // set RX interrupts to generate when the RX buffer has at least 1 item
//    IFS0bits.U1RXIF = 0; // clear RX interrupt flag
//
//    IPC6bits.U1IP = 7; // Interrupt protocol priority
//    IPC6bits.U1IS = 3; // Interrupt sub-protocol priority
//    return SUCCESS;
//}
//
//int Protocol_SendMessage(unsigned char len, unsigned char ID, void *Payload) {
//
//    char * message = (char *) Payload;
//    char cksum = Protocol_CalcIterativeChecksum(ID, 0);
//    int i;
//    PutChar(HEAD);
//    PutChar(len);
//    PutChar(ID);
//    for (i = 0; i < (unsigned int) len - 1; i++) {
//        cksum = Protocol_CalcIterativeChecksum(message[i], cksum);
//        PutChar(message[i]);
//    }
//    PutChar(TAIL);
//    PutChar(cksum);
//    PutChar('\r'); // add pound define later
//    PutChar('\n'); // add pound define later     
//    return SUCCESS;
//
//    //    while (PutChar(HEAD) == ERROR)
//    //        for (int x = 0; x < 4; x++) { // delay for at least 10us before resetting the interrupt flag
//    //            asm(" nop ");
//    //        }
//    //    while (PutChar(len) == ERROR)
//    //        for (int x = 0; x < 4; x++) { // delay for at least 10us before resetting the interrupt flag
//    //            asm(" nop ");
//    //        }
//    //    unsigned char checksum = 0;
//    //    checksum = Protocol_CalcIterativeChecksum(ID, checksum);
//    //    while (PutChar(ID) == ERROR)
//    //        for (int x = 0; x < 4; x++) { // delay for at least 10us before resetting the interrupt flag
//    //            asm(" nop ");
//    //        }
//    //
//    //    unsigned int i;
//    //    unsigned char * plchar = Payload;
//    //
//    //    for (i = 0; i < (unsigned int) len - 1; i++) { // len -1 because the lenght given includes the ID
//    //        while (PutChar(*plchar) == ERROR)
//    //            for (int x = 0; x < 4; x++) { // delay for at least 10us before resetting the interrupt flag
//    //                asm(" nop ");
//    //            }
//    //        checksum = Protocol_CalcIterativeChecksum(*plchar, checksum);
//    //        ++plchar;
//    //    }
//    //    for (int x = 0; x < 10; x++) { // delay for at least 10us before resetting the interrupt flag
//    //        asm(" nop ");
//    //    }
//    //    while (PutChar(TAIL) == ERROR)
//    //        for (int x = 0; x < 10; x++) { // delay for at least 10us before resetting the interrupt flag
//    //            asm(" nop ");
//    //        }
//    //    while (PutChar(checksum) == ERROR)
//    //        for (int x = 0; x < 4; x++) { // delay for at least 10us before resetting the interrupt flag
//    //            asm(" nop ");
//    //        }
//    //    while (PutChar('\r') == ERROR)
//    //        for (int x = 0; x < 4; x++) { // delay for at least 10us before resetting the interrupt flag
//    //            asm(" nop ");
//    //        }
//    //    while (PutChar('\n') == ERROR)
//    //        for (int x = 0; x < 4; x++) { // delay for at least 10us before resetting the interrupt flag
//    //            asm(" nop ");
//    //        }
//    //    return SUCCESS;
//
//}
//
//int Protocol_SendDebugMessage(char *Message) {
//    unsigned int x;
//    unsigned char len = strlen(Message);
//    for (x = 0; x <= len; x++) {
//        packPAYLOAD[x] = Message[x];
//    }
//    //  printf("%c", packPAYLOAD[1]);
//    Protocol_SendMessage(len + 1, ID_DEBUG, packPAYLOAD);
//    return SUCCESS;
//    // return Protocol_SendMessage(strlen(Message), ID_DEBUG, Message);
//}
//
//unsigned char Protocol_ReadNextID(void) {
//    //unsigned char thisID = dequeue_CB(&RXCB);
//    return packID;
//}
//
//int Protocol_GetPayload(void* payload) {
//    if ((!check_EmptyBuff(&RXCB))) {
//        return ERROR;
//    }
//    unsigned char tempLength = dequeue_CB(&RXCB);
//    unsigned char i;
//    unsigned char temp[tempLength];
//    // dequeue_CB(&RXCB); // this gets rid of the ID
//    for (i = 0; i < tempLength - 1; i++) {
//        if (check_EmptyBuff(&RXCB) == 0) {
//            ((unsigned char*) payload)[i] = dequeue_CB(&RXCB);
//            // }
//            // ((unsigned char*) payload)[i] = packPAYLOAD[i + 1];
//        }
//    }
//    memcpy(payload, temp, tempLength);
//    // init_buff(&RXCB);
//    return SUCCESS;
//}
//
//char Protocol_IsMessageAvailable(void) {
//    if (check_EmptyBuff(&RXCB) == 1) {
//        return FALSE;
//        //    } else if (RXCB.data[RXCB.head] > RXCB.size) {
//        //        return FALSE;
//    } else {
//        return TRUE;
//    }
//}
//
//char Protocol_IsQueueFull(void) {
//    if (check_FullBuff(&TXCB) || check_FullBuff(&RXCB)) {
//        return TRUE;
//    }
//}
//
//char Protocol_IsError(void) {
//    unsigned char systERROR = 0;
//    if (U1STAbits.PERR | U1STAbits.FERR | U1STAbits.OERR) { // Checking for Parity Error, Framing Error, and Overflow Error
//        MODE = WAIT_FOR_HEAD;
//        Protocol_Init();
//        systERROR = 0;
//    }
//    if (errorFlag != 0) {
//        if (errorFlag == 1) {
//            Protocol_SendDebugMessage("Length too long!");
//        } else if (errorFlag == 2) {
//            Protocol_SendDebugMessage("Wrong checksum!");
//        } else if (errorFlag == 3) {
//            Protocol_SendDebugMessage("TX is full!");
//        }
//        errorFlag = 0;
//        MODE = WAIT_FOR_HEAD;
//        if (systERROR != 1) {
//            Protocol_Init();
//        }
//        return TRUE;
//    } else {
//        return FALSE;
//    }
//}
//
//unsigned short Protocol_ShortEndednessConversion(unsigned short inVariable) {
//    unsigned char front = (inVariable & 0xFF00) >> 8;
//    unsigned char back = inVariable & 0x00FF;
//    unsigned int result = ((unsigned short) ((back << 8) + front));
//    // 
//    return result;
//
//}
//
//unsigned int Protocol_IntEndednessConversion(unsigned int inVariable) {
//    inVariable = ((inVariable >> 24) & 0xFF) | ((inVariable << 8) & 0xFF0000)
//            | ((inVariable >> 8) & 0xFF00) | ((inVariable << 24) & 0xFF000000);
//    return inVariable;
//
//}
//
//unsigned char Protocol_CalcIterativeChecksum(unsigned char charIn, unsigned char curChecksum) {
//    curChecksum = (curChecksum >> 1) + (curChecksum << 7);
//    curChecksum = curChecksum + charIn;
//    return curChecksum;
//}
//
//void Configure_Pong(void* theArray) {
//    unsigned char* PONG = (unsigned char*) theArray;
//    unsigned int zero_Pos = ((unsigned int) PONG[0]) << 24; // Convert endedness
//    unsigned int one_Pos = ((unsigned int) PONG[1]) << 16;
//    unsigned int two_Pos = ((unsigned int) PONG[2]) << 8;
//    unsigned int three_Pos = ((unsigned int) PONG[3]);
//    unsigned int divide = (zero_Pos | one_Pos | two_Pos | three_Pos) >> 1; // divide by 2
//    PONG[0] = (divide & 0xFF000000) >> 24; // convert endedness again
//    PONG[1] = (divide & 0xFF0000) >> 16;
//    PONG[2] = (divide & 0xFF00) >> 8;
//    PONG[3] = (divide & 0xFF);
//}
//
//void Protocol_RunReceiveStateMachine(unsigned char charIn) {
//    char *checksumError = "Checksums didn't match\n";
//    char *tailError = "No tail was given\n";
//
////    switch (MODE) {
////        case WAIT_FOR_HEAD:
////            if (charIn == HEAD) {
////                packLENGTH = 0; // resetting the variables used
////                packID = 0;
////                packCHECKSUM = 0;
////                counter = 0;
////                ledValue = 0;
////                packLEDS = 0;
////                MODE = GET_LENGTH;
////            }
////            break;
////
////        case GET_LENGTH:
////            if (charIn <= MAXPAYLOADLENGTH) {
////                packLENGTH = charIn; // save the length
////                MODE = GET_ID;
////            } else {
////                errorFlag = 1;
////            }
////            break;
////
////        case GET_ID:
////            packID = charIn; // save the message ID
////            packCHECKSUM = Protocol_CalcIterativeChecksum(charIn, packCHECKSUM);
////            MODE = GET_PAYLOAD;
////            break;
////
////        case GET_PAYLOAD:
////            if ((packID == ID_LEDS_SET) && (counter == 0)) { // ID_LEDS_SET has only 1 item in payload
////                packLEDS = charIn; // save the item to be used later
////                packCHECKSUM = Protocol_CalcIterativeChecksum(charIn, packCHECKSUM);
////                MODE = GET_TAIL;
////            }                //            else if ((packID == ID_PING) && (counter == 0)) { // check if the message is a Ping
////                //                packPAYLOAD[counter] = charIn; // save charIn to the payload
////                //                pongFlag = 1; // set flag for Pong
////                //                packCHECKSUM = Protocol_CalcIterativeChecksum(charIn, packCHECKSUM);
////                //                counter++;
////                //            } else if (packID == ID_COMMAND_SERVO_PULSE) {
////                //                packPAYLOAD[counter] = charIn;
////                //                servoFlag = 1;
////                //                packCHECKSUM = Protocol_CalcIterativeChecksum(charIn, packCHECKSUM);
////                //            } else if (pongFlag == 1) {
////                //                packPAYLOAD[counter] = charIn;
////                //                packCHECKSUM = Protocol_CalcIterativeChecksum(charIn, packCHECKSUM);
////                //                counter++;
////                //                if (counter == 4) { // 4 = length of PING payload - 1
////                //                    MODE = GET_TAIL;
////                //                }
////                //            }
////            else { // if ID is not some special case, process payload 
////                packPAYLOAD[counter] = charIn;
////                packCHECKSUM = Protocol_CalcIterativeChecksum(charIn, packCHECKSUM);
////                if (counter == (packLENGTH - 1)) { // packLENGTH - 1 because we already processed the ID
////                    MODE = GET_TAIL;
////                }
////                counter++;
////                MODE = GET_PAYLOAD; // re-process payload until 1 of the above cases are met
////                //                for (int i = 0; i < 50; i++) {
////                //                    asm("nop");
////                //                }
////            }
////            break;
////
////        case GET_TAIL:
////            if (charIn != TAIL) { // this is making sure we are getting a tail
////                Protocol_SendDebugMessage(tailError);
////                MODE = WAIT_FOR_HEAD;
////            } else {
////                MODE = COMPARE_CHECKSUMS; // we got a tail so now we need to make sure checksum is correct
////            }
////            break;
////
////        case COMPARE_CHECKSUMS:
////            if (packCHECKSUM != charIn) { // send an error message if checksums don't match
////                Protocol_SendDebugMessage(checksumError);
////                MODE = WAIT_FOR_HEAD;
////
////            } else if (packID == ID_LEDS_SET) { // setting the LEDS
////                LEDS_SET(packLEDS);
////                // MODE = GET_END1;
////                MODE = WAIT_FOR_HEAD;
////
////            } else if (packID == ID_LEDS_GET) { // getting the status of the LEDS
////                ledValue = LEDS_GET();
////                Protocol_SendMessage(2, ID_LEDS_STATE, &ledValue);
////                //MODE = GET_END1; 
////                MODE = WAIT_FOR_HEAD; // we go back to head because thats the end of the packet
////
////            } else if (packID == ID_PING) { // response to ID_PING
////                Configure_Pong(packPAYLOAD); // convert --> shift --> convert
////                Protocol_SendMessage(packLENGTH, ID_PONG, packPAYLOAD);
////                pongFlag = 0;
////                //MODE = GET_END1;
////                MODE = WAIT_FOR_HEAD;
////
////            } else if (packID == ID_COMMAND_SERVO_PULSE) {
////                Configure_Servo(packPAYLOAD);
////                Protocol_SendMessage(4, ID_SERVO_RESPONSE, packPAYLOAD);
////                //                rc_PL1 = packPAYLOAD[0];
////                //                rc_PL2 = packPAYLOAD[1];
////                //                rc_PL3 = packPAYLOAD[2];
////                //                rc_PL4 = packPAYLOAD[3];
////                LEDS_SET(0b11);
////                MODE = WAIT_FOR_HEAD;
////            } else {
////                //                thisLen packLENGTH;
////                //                thisID packID;
////                //                thisPayload packPAYLOAD;
////                enqueue_CB(packLENGTH, &RXCB); // Putting the important parts of packet into RXCB
////                enqueue_CB(packID, &RXCB);
////                enqueue_CB(*packPAYLOAD, &RXCB);
////                //MODE = GET_END1;
////                MODE = WAIT_FOR_HEAD;
////            }
////            break;
////
////
////    }
//
//
//        switch (MODE) {
//            case WAIT_FOR_HEAD:
//                if (charIn == HEAD) {
//                    packLENGTH = 0; // resetting the variables used
//                    packID = 0;
//                    packCHECKSUM = 0;
//                    counter = 0;
//                    ledValue = 0;
//                    packLEDS = 0;
//                    MODE = GET_LENGTH;
//                }
//                break;
//    
//            case GET_LENGTH:
//                if (charIn <= MAXPAYLOADLENGTH) {
//                    packLENGTH = charIn; // save the length
//                    MODE = GET_ID;
//                } else {
//                    errorFlag = 1;
//                }
//                break;
//    
//            case GET_ID:
//                packID = charIn; // save the message ID
//                packCHECKSUM = Protocol_CalcIterativeChecksum(charIn, packCHECKSUM);
//                MODE = GET_PAYLOAD;
//                break;
//    
//            case GET_PAYLOAD:
//                if ((packID == ID_LEDS_SET) && (counter == 0)) { // ID_LEDS_SET has only 1 item in payload
//                    packLEDS = charIn; // save the item to be used later
//                    packCHECKSUM = Protocol_CalcIterativeChecksum(charIn, packCHECKSUM);
//                    MODE = GET_TAIL;
//                } else if ((packID == ID_PING) && (counter == 0)) { // check if the message is a Ping
//                    packPAYLOAD[counter] = charIn; // save charIn to the payload
//                    pongFlag = 1; // set flag for Pong
//                    packCHECKSUM = Protocol_CalcIterativeChecksum(charIn, packCHECKSUM);
//                    counter++;
//                } else if (packID == ID_COMMAND_SERVO_PULSE) {
//                    packPAYLOAD[counter] = charIn;
//                    servoFlag = 1;
//                    packCHECKSUM = Protocol_CalcIterativeChecksum(charIn, packCHECKSUM);
//                } else if (pongFlag == 1) {
//                    packPAYLOAD[counter] = charIn;
//                    packCHECKSUM = Protocol_CalcIterativeChecksum(charIn, packCHECKSUM);
//                    counter++;
//                    if (counter == 4) { // 4 = length of PING payload - 1
//                        MODE = GET_TAIL;
//                    }
//                } else { // if ID is not some special case, process payload 
//                    packPAYLOAD[counter] = charIn;
//                    packCHECKSUM = Protocol_CalcIterativeChecksum(charIn, packCHECKSUM);
//                    if (counter == (packLENGTH - 1)) { // packLENGTH - 1 because we already processed the ID
//                        MODE = GET_TAIL;
//                    }
//                    counter++;
//                    MODE = GET_PAYLOAD; // re-process payload until 1 of the above cases are met
//                    //                for (int i = 0; i < 50; i++) {
//                    //                    asm("nop");
//                    //                }
//                }
//                break;
//    
//            case GET_TAIL:
//                if (charIn != TAIL) { // this is making sure we are getting a tail
//                    Protocol_SendDebugMessage(tailError);
//                    MODE = WAIT_FOR_HEAD;
//                } else {
//                    MODE = COMPARE_CHECKSUMS; // we got a tail so now we need to make sure checksum is correct
//                }
//                break;
//    
//            case COMPARE_CHECKSUMS:
//                if (packCHECKSUM != charIn) { // send an error message if checksums don't match
//                    Protocol_SendDebugMessage(checksumError);
//                    MODE = WAIT_FOR_HEAD;
//    
//                } else if (packID == ID_LEDS_SET) { // setting the LEDS
//                    LEDS_SET(packLEDS);
//                    // MODE = GET_END1;
//                    MODE = WAIT_FOR_HEAD;
//    
//                } else if (packID == ID_LEDS_GET) { // getting the status of the LEDS
//                    ledValue = LEDS_GET();
//                    Protocol_SendMessage(2, ID_LEDS_STATE, &ledValue);
//                    //MODE = GET_END1; 
//                    MODE = WAIT_FOR_HEAD; // we go back to head because thats the end of the packet
//    
//                } else if (pongFlag == 1) { // response to ID_PING
//                    Configure_Pong(packPAYLOAD); // convert --> shift --> convert
//                    Protocol_SendMessage(packLENGTH, ID_PONG, packPAYLOAD);
//                    pongFlag = 0;
//                    //MODE = GET_END1;
//                    MODE = WAIT_FOR_HEAD;
//    
//                }
////                else if (servoFlag == 1) {
////                    rc_PL1 = packPAYLOAD[0];
////                    rc_PL2 = packPAYLOAD[1];
////                    rc_PL3 = packPAYLOAD[2];
////                    rc_PL4 = packPAYLOAD[3];
////                    servoFlag = 0;
////                    LEDS_SET(0b11);
////                    MODE = WAIT_FOR_HEAD;
////                }
//                else {
//                    //                thisLen packLENGTH;
//                    //                thisID packID;
//                    //                thisPayload packPAYLOAD;
//                    enqueue_CB(packLENGTH, &RXCB); // Putting the important parts of packet into RXCB
//                    enqueue_CB(packID, &RXCB);
//                    enqueue_CB(*packPAYLOAD, &RXCB);
//                    //MODE = GET_END1;
//                    MODE = WAIT_FOR_HEAD;
//                }
//                break;
//    
//    
//        }
//}
//
//char PutChar(char ch) {
//    //    if (check_FullBuff(&TXCB) == 1) { // check if the buffer is full
//    //        clear = 0;
//    //        return ERROR;
//    //    }
//    //    putCharFlag = 1;
//    //    enqueue_CB(ch, &TXCB); // put the char on the buffer
//    //    putCharFlag = 0;
//    //
//    //    while (U1STAbits.TRMT == 0);
//    //
//    //    if ((U1STAbits.TRMT == 1) | (collision == 1)) { // TX shift reg is empty (last TX is complete)
//    //        collision = 0;
//    //        IFS0bits.U1TXIF = 1;
//    //        clear = 1;
//    //    } 
//    //    return SUCCESS;
//
//    if ((check_FullBuff(&TXCB) == 0) | (collision == 1)) { // TX shift reg is empty (last TX is complete)
//        putCharFlag = 1;
//        enqueue_CB(ch, &TXCB); // put the char on the buffer
//        putCharFlag = 0;
//        collision = 0;
//        IFS0bits.U1TXIF = 1;
//        clear = 1;
//        return SUCCESS;
//    } else {
//        return ERROR;
//    }
//}
//
//void __ISR(_UART1_VECTOR)IntUart1Handler(void) {
//    if (IFS0bits.U1RXIF == 1) { // check if we're here because RX interrupt
//        IFS0bits.U1RXIF = 0; // reset the flag
//        // while (1) {
//        Protocol_RunReceiveStateMachine(U1RXREG); // run RXSM
//        // }
//    }
//    // IFS0bits.U1RXIF = 0;
//    if (IFS0bits.U1TXIF == 1) { // check if we're here because TX interrupt
//        IFS0bits.U1TXIF = 0; // reset the flag
//        if (putCharFlag == 0) {
//            while (check_EmptyBuff(&TXCB) == 0) {
//                if (U1STAbits.UTXBF == 0) {
//                    unsigned char txin = dequeue_CB(&TXCB); // value from CB goes into TX reg
//                    U1TXREG = txin;
//                } else {
//                    asm("nop");
//                    asm("nop");
//                    asm("nop");
//                    asm("nop");
//                    asm("nop");
//                    asm("nop");
//                }
//            }
//            // IFS0bits.U1TXIF = 0;
//
//        } else {
//            collision = 1;
//        }
//    }
//}
//
//#ifdef PROTOCOL_TESTHARNESS
//
//int main(void) {
//
//    unsigned char test_char[] = "Please work";
//    BOARD_Init();
//    char *message = "11111111";
//    Protocol_Init();
//    init_buff(&TXCB);
//    init_buff(&RXCB);
//    LEDS_INIT();
//    // while (1);
//#ifdef PUTCHAR_TEST
//    int i = 0;
//    while (test_char[i] != '\0') {
//        while (!U1STAbits.TRMT);
//        PutChar(test_char[i]);
//        i++;
//
//    }
//#endif
//
//#ifdef INT_ENDED_TEST
//    unsigned int theINT = 0xDEADBEEF;
//    unsigned int reversedINT = Protocol_IntEndednessConversion(theINT);
//    // Protocol_SendDebugMessage(reversedINT);
//
//
//#endif
//
//#ifdef SENDMSG_TEST
//    char testPAYLOAD[] = {'1', '2', '3', '4'};
//    Protocol_SendMessage(4, ID_LEDS_SET, testPAYLOAD);
//#endif
//
//#ifdef CHECKSUM_TEST
//    char test1[] = "0x817F";
//    unsigned char t1_sum = checkSum(test1);
//    PutChar(t1_sum);
//    sprintf(t1_sum, "\n")
//#endif
//
//#ifdef ECHO_TEST
//            while (1) { // always running in background
//        if (U1STAbits.URXDA == 1) { // buffer full whenever you type 
//            // PutChar(U1RXREG); // initiate the TX
//            IFS0bits.U1RXIF = 1;
//        }
//    }
//#endif
//
//#ifdef PACKET_TEST
//    IFS0bits.U1RXIF = 1;
//
//#endif
//
//
//
//
//    //   }
//    while (1);
//    BOARD_End();
//    //return 0;
//}
//#endif
//
////#define TESTTEST
//#ifdef TESTTEST
//
//int main(void) {
//    BOARD_Init();
//    Protocol_Init();
//    unsigned char str[5] = {0, 0, 2, 88};
//    Protocol_SendMessage(5, ID_DEBUG, &str);
//    while (1);
//}
//#endif
//
//
//// is this saving properly!!













//#include "BOARD.h"
//#include <xc.h>
//#include "Protocol.h"
//#include <sys/attribs.h>
//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//#include "MessageIDs.h"
//#include "circleBuff.h"
//#include "RCServo.h"
//
///* Here are some global definitions */
//#define Fpb 40e6 
//#define desired_baud 115200
//#define BaudRate ((Fpb / desired_baud) / 16) - 1 // baud rate gen value of 21 gives Baud Rate = 115k
//#define maxPAYLOADlength 128
//
///* HERE ARE SOME TEST SUITES */
////#define ECHO_TEST
////#define BUFFER_TEST
////#define PUTCHAR_TEST
////#define SENDMSG_TEST
////#define PACKET_TEST
////#define MAX_BUFFER_LENGTH 32
////#define CHECKSUM_TEST
////#define INT_ENDED_TEST
//
//
///* These are some static declarations used throughout the code*/
//static int collision;
//static int putCharFlag;
//static int counter;
//static int clear;
//static unsigned char packHEAD;
//static unsigned char packLENGTH;
//static unsigned char packID;
//static unsigned char packPAYLOAD[maxPAYLOADlength];
//static unsigned char packCHECKSUM;
//static unsigned char packLEDS;
//static unsigned char ledValue;
//static int pongFlag;
//
///* Initializing the 2 EMPTY TX & RX buffer */
//static struct CircleBuffer TXCB = {};
//static struct CircleBuffer RXCB = {};
//
///* These are the states in the RXSM */
//typedef enum {
//    WAIT_FOR_HEAD, GET_LENGTH, GET_ID, GET_PAYLOAD, GET_TAIL,
//    COMPARE_CHECKSUMS, DETERMINE_RESPONSE, GET_END1, GET_END2
//} states;
//
///* Initializing the first state */
//static states MODE = WAIT_FOR_HEAD;
//
//int Protocol_Init(void) {
//    U1MODE = 0; // Clear mode register
//    U1STA = 0; // clear status register
//    U1BRG = BaudRate; // set the baud rate
//    U1MODEbits.PDSEL = 00; // sets 8-data and no parity
//    U1MODEbits.STSEL = 0; // sets 1 stop bit
//
//    U1MODEbits.ON = 1; // Turn UART on
//    U1MODEbits.UARTEN = 1; // enable UART
//
//    U1STAbits.URXEN = 1; // enable RX
//    U1STAbits.UTXEN = 1; // enable TX
//
//    IEC0bits.U1TXIE = 1; // enable TX interrupts
//    U1STAbits.UTXISEL = 0b10; // set interrupt to generate when TX buffer becomes empty
//    IFS0bits.U1TXIF = 0; // clear TX interrupt flag
//
//    IEC0bits.U1RXIE = 1; // enable RX interrupts
//    U1STAbits.URXISEL = 0b00; // set RX interrupts to generate when the RX buffer has at least 1 item
//    IFS0bits.U1RXIF = 0; // clear RX interrupt flag
//
//    IPC6bits.U1IP = 7; // Interrupt protocol priority
//    IPC6bits.U1IS = 0b010; // Interrupt sub-protocol priority
//    return SUCCESS;
//}
//
//int Protocol_SendMessage(unsigned char len, unsigned char ID, void *Payload) {
//    unsigned char SCORED; // this is being used as a check to make sure PutChar is successful
//    SCORED = PutChar(HEAD);
//    // printf("%X", HEAD);
//    if (SCORED == SUCCESS) {
//        SCORED = PutChar(len);
//        //   printf("%X", len);
//    }
//
//    unsigned char checksum = 0;
//    checksum = Protocol_CalcIterativeChecksum(ID, checksum);
//    if (SCORED == SUCCESS) {
//        SCORED = PutChar(ID);
//        // printf("%X", ID);
//    }
//    unsigned char i;
//    unsigned char * plchar = Payload;
//
//    for (i = 0; i < len - 1; i++) { // len -1 because the lenght given includes the ID
//        if (SCORED == SUCCESS) {
//            SCORED = PutChar(*plchar);
//            //   printf("%X", *plchar);
//        }
//        checksum = Protocol_CalcIterativeChecksum(*plchar, checksum);
//        ++plchar;
//    }
//    if (SCORED == SUCCESS) {
//        SCORED = PutChar(TAIL);
//        // printf("%X", TAIL);
//    }
//    if (SCORED == SUCCESS) {
//        SCORED = PutChar(checksum);
//        //  printf("%X", checksum);
//    }
//    if (SCORED == SUCCESS) {
//        SCORED = PutChar('\r');
//        //printf("%X", '\r');
//    }
//    if (SCORED == SUCCESS) {
//        SCORED = PutChar('\n');
//        // printf("%X", '\n');
//    }
//    return SUCCESS;
//}
//
//int Protocol_SendDebugMessage(char *Message) {
//    unsigned int x;
//    unsigned char len = strlen(Message);
//    for (x = 0; x <= len; x++) {
//        packPAYLOAD[x] = Message[x];
//    }
//    //  printf("%c", packPAYLOAD[1]);
//    Protocol_SendMessage(len + 1, ID_DEBUG, packPAYLOAD);
//    return SUCCESS;
//    // return Protocol_SendMessage(strlen(Message), ID_DEBUG, Message);
//}
//
//unsigned char Protocol_ReadNextID(void) {
//    //unsigned char thisID = dequeue_CB(&RXCB);
//    if (check_EmptyBuff(&RXCB) == 1) {
//        return ID_INVALID;
//    } else {
//        return RXCB.data[RXCB.head];
//    }
//}
//
//int Protocol_GetPayload(void* payload) {
//    if ((check_EmptyBuff(&RXCB) == 1) || (check_EmptyBuff(&TXCB) == 1)) {
//        return ERROR;
//    }
//    unsigned char tempLength = dequeue_CB(&RXCB) - 1;
//    unsigned char temp[tempLength];
//    dequeue_CB(&RXCB); // this gets rid of the ID
//    unsigned char i;
//    for (i = 0; i < tempLength; i++) {
//        temp[i] = dequeue_CB(&RXCB);
//
//        //((unsigned char*) payload)[i] = packPAYLOAD[i+1];
//    }
//    memcpy(payload, temp, tempLength);
//    return SUCCESS;
//}
//
//char Protocol_IsMessageAvailable(void) {
//    if ((!check_EmptyBuff(&RXCB)) || (!check_EmptyBuff(&TXCB))) {
//        return TRUE;
//    }
//}
//
//char Protocol_IsQueueFull(void) {
//    if (check_FullBuff(&TXCB) || check_FullBuff(&RXCB)) {
//        return TRUE;
//    }
//}
//
//char Protocol_IsError(void) {
//    if (U1STAbits.PERR | U1STAbits.FERR | U1STAbits.OERR) { // Checking for Parity Error, Framing Error, and Overflow Error
//        return TRUE;
//    }
//}
//
//unsigned short Protocol_ShortEndednessConversion(unsigned short inVariable) {
//    unsigned char front = (inVariable & 0xFF00) >> 8;
//    unsigned char back = inVariable & 0x00FF;
//    unsigned int result = ((unsigned short) ((back << 8) + front));
//    return result;
//
//}
//
//unsigned int Protocol_IntEndednessConversion(unsigned int inVariable) {
//    inVariable = ((inVariable >> 24) & 0xFF) | ((inVariable << 8) & 0xFF0000)
//            | ((inVariable >> 8) & 0xFF00) | ((inVariable << 24) & 0xFF000000);
//    return inVariable;
//
//}
//
//unsigned char Protocol_CalcIterativeChecksum(unsigned char charIn, unsigned char curChecksum) {
//    curChecksum = (curChecksum >> 1) + (curChecksum << 7);
//    curChecksum = curChecksum + charIn;
//    return curChecksum;
//}
//
//void Configure_Pong(void* theArray) {
//    unsigned char* PONG = (unsigned char*) theArray;
//    unsigned int zero_Pos = ((unsigned int) PONG[0]) << 24; // Convert endedness
//    unsigned int one_Pos = ((unsigned int) PONG[1]) << 16;
//    unsigned int two_Pos = ((unsigned int) PONG[2]) << 8;
//    unsigned int three_Pos = ((unsigned int) PONG[3]);
//    unsigned int divide = (zero_Pos | one_Pos | two_Pos | three_Pos) >> 1; // divide by 2
//    PONG[0] = (divide & 0xFF000000) >> 24; // convert endedness again
//    PONG[1] = (divide & 0xFF0000) >> 16;
//    PONG[2] = (divide & 0xFF00) >> 8;
//    PONG[3] = (divide & 0xFF);
//}
//
//void Protocol_RunReceiveStateMachine(unsigned char charIn) {
//    char *checksumError = "Checksums didn't match\n";
//    char *tailError = "No tail was given\n";
//
//
//    switch (MODE) {
//        case WAIT_FOR_HEAD:
//            if (charIn == HEAD) {
//                //   packLENGTH = 0; // resetting the variables used
//                //   packID = 0;
//                packCHECKSUM = 0;
//                counter = 0;
//                ledValue = 0;
//                packLEDS = 0;
//                MODE = GET_LENGTH;
//            }
//            break;
//
//        case GET_LENGTH:
//            packLENGTH = charIn; // save the length
//            MODE = GET_ID;
//            break;
//
//        case GET_ID:
//            packID = charIn; // save the message ID
//            packCHECKSUM = Protocol_CalcIterativeChecksum(charIn, packCHECKSUM);
//            if (packID == ID_LEDS_GET) { // check for special case that has no payload
//                MODE = GET_TAIL;
//            } else {
//                MODE = GET_PAYLOAD;
//            }
//            break;
//
//        case GET_PAYLOAD:
//            if ((packID == ID_LEDS_SET) && (counter == 0)) { // ID_LEDS_SET has only 1 item in payload
//                packLEDS = charIn; // save the item to be used later
//                packCHECKSUM = Protocol_CalcIterativeChecksum(charIn, packCHECKSUM);
//                MODE = GET_TAIL;
//            } else if ((packID == ID_PING) && (counter == 0)) { // check if the message is a Ping
//                packPAYLOAD[counter] = charIn; // save charIn to the payload
//
//                pongFlag = 1; // set flag for Pong
//                packCHECKSUM = Protocol_CalcIterativeChecksum(charIn, packCHECKSUM);
//                counter++;
//            } else if (pongFlag == 1) {
//                packPAYLOAD[counter] = charIn;
//                packCHECKSUM = Protocol_CalcIterativeChecksum(charIn, packCHECKSUM);
//                counter++;
//                if (counter == 4) { // 4 = length of PING payload - 1
//                    MODE = GET_TAIL;
//                }
//            } else { // if ID is not some special case, process payload 
//                packPAYLOAD[counter] = charIn;
//                packCHECKSUM = Protocol_CalcIterativeChecksum(charIn, packCHECKSUM);
//                if (counter == packLENGTH - 1) { // packLENGTH - 1 because we already processed the ID
//                    MODE = GET_TAIL;
//                }
//                counter++;
//                MODE = GET_PAYLOAD; // re-process payload until 1 of the above cases are met
//            }
//            break;
//
//        case GET_TAIL:
//            if (charIn != TAIL) { // this is making sure we are getting a tail
//                Protocol_SendDebugMessage(tailError);
//                MODE = WAIT_FOR_HEAD;
//            } else {
//                MODE = COMPARE_CHECKSUMS; // we got a tail so now we need to make sure checksum is correct
//            }
//            break;
//
//        case COMPARE_CHECKSUMS:
//            if (packCHECKSUM != charIn) { // send an error message if checksums don't match
//                Protocol_SendDebugMessage(checksumError);
//                MODE = WAIT_FOR_HEAD;
//
//            } else if (packID == ID_LEDS_SET) { // setting the LEDS
//                LEDS_SET(packLEDS);
//                // MODE = GET_END1;
//                MODE = WAIT_FOR_HEAD;
//
//            } else if (packID == ID_LEDS_GET) { // getting the status of the LEDS
//                ledValue = LEDS_GET();
//                Protocol_SendMessage(2, ID_LEDS_STATE, &ledValue);
//                //MODE = GET_END1; 
//                MODE = WAIT_FOR_HEAD; // we go back to head because thats the end of the packet
//
//
//            } else if (pongFlag == 1) { // response to ID_PING
//                Configure_Pong(packPAYLOAD); // convert --> shift --> convert
//                Protocol_SendMessage(packLENGTH, ID_PONG, packPAYLOAD);
//                pongFlag = 0;
//                //MODE = GET_END1;
//                MODE = WAIT_FOR_HEAD;
//
//            }
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////            else if (packID == ID_COMMAND_SERVO_PULSE) {
////                memcpy(servoPAYLOAD, packPAYLOAD, sizeof (packPAYLOAD));
////                Protocol_SendMessage(packLENGTH, ID_SERVO_RESPONSE, &servoPAYLOAD);
////                MODE = WAIT_FOR_HEAD;
////
////            }
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//            else {
//                enqueue_CB(packLENGTH, &RXCB); // Putting the important parts of packet into RXCB
//                enqueue_CB(packID, &RXCB);
//                enqueue_CB(*packPAYLOAD, &RXCB);
//                //MODE = GET_END1;
//                MODE = WAIT_FOR_HEAD;
//            }
//            break;
//
//
//    }
//}
//
//char PutChar(char ch) {
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
//}
//
//void __ISR(_UART1_VECTOR)IntUart1Handler(void) {
//    if (IFS0bits.U1RXIF == 1) { // check if we're here because RX interrupt
//        IFS0bits.U1RXIF = 0; // reset the flag
//        // while (1) {
//        Protocol_RunReceiveStateMachine(U1RXREG); // run RXSM
//        // }
//    }
//    // IFS0bits.U1RXIF = 0;
//    if (IFS0bits.U1TXIF == 1) { // check if we're here because TX interrupt
//        IFS0bits.U1TXIF = 0; // reset the flag
//        if (putCharFlag == 0) {
//            if (check_EmptyBuff(&TXCB) == 0) {
//                U1TXREG = dequeue_CB(&TXCB); // value from CB goes into TX reg
//            }
//            // IFS0bits.U1TXIF = 0;
//
//        } else {
//            collision = 1;
//        }
//    }
//}
//
//#ifdef PROTOCOL_TESTHARNESS
//
//int main(void) {
//
//    unsigned char test_char[] = "Please work";
//    BOARD_Init();
//    char *message = "11111111";
//    Protocol_Init();
//    init_buff(&TXCB);
//    init_buff(&RXCB);
//    LEDS_INIT();
//    // while (1);
//#ifdef PUTCHAR_TEST
//    int i = 0;
//    while (test_char[i] != '\0') {
//        while (!U1STAbits.TRMT);
//        PutChar(test_char[i]);
//        i++;
//
//    }
//#endif
//
//#ifdef INT_ENDED_TEST
//    unsigned int theINT = 0xDEADBEEF;
//    unsigned int reversedINT = Protocol_IntEndednessConversion(theINT);
//    // Protocol_SendDebugMessage(reversedINT);
//
//
//#endif
//
//#ifdef SENDMSG_TEST
//    char testPAYLOAD[] = {'1', '2', '3', '4'};
//    Protocol_SendMessage(4, ID_LEDS_SET, testPAYLOAD);
//#endif
//
//#ifdef CHECKSUM_TEST
//    char test1[] = "0x817F";
//    unsigned char t1_sum = checkSum(test1);
//    PutChar(t1_sum);
//    sprintf(t1_sum, "\n")
//#endif
//
//#ifdef ECHO_TEST
//            while (1) { // always running in background
//        if (U1STAbits.URXDA == 1) { // buffer full whenever you type 
//            // PutChar(U1RXREG); // initiate the TX
//            IFS0bits.U1RXIF = 1;
//        }
//    }
//#endif
//
//#ifdef PACKET_TEST
//    IFS0bits.U1RXIF = 1;
//
//#endif
//
//
//
//
//    //   }
//    while (1);
//    BOARD_End();
//    //return 0;
//}
//#endif
//
//
//// is this saving properly!!

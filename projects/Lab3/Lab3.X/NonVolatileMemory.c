#include <proc/p32mx340f512h.h>
#include "BOARD.h"
#include "NonVolatileMemory.h"
#include "Protocol.h"
#include "MessageIDs.h"

#define SlaveAddress 0x1010000
#define SlaveRead 0x10100001
#define SlaveWrite 0x10100000

//#define NVM_TEST

/**
 * @Function NonVolatileMemory_Init(void)
 * @param None
 * @return SUCCESS or ERROR
 * @brief initializes I2C for usage */
int NonVolatileMemory_Init(void) {
    I2C1CON = 0; // clear and disable I2C while initializing
    I2C1BRG = 0x0C6; // 198. Sets clock frequency to 100kHz

    I2C1CONbits.ON = 1;

    //    I2C1CONbits.A10M = 0; // 7-bit slave address mode
    //    I2C1CONbits.ACKEN = 0; // ack sequence is idle 
    //    I2C1CONbits.RCEN = 1; // enable RX
    //    I2C1CONbits.PEN = 0; // Stop condition idle
    //    I2C1CONbits.RSEN = 0; // Restart condition idle
    //    I2C1CONbits.SEN = 1; // initiate start condition

    return SUCCESS;

}

/**
 * @Function NonVolatileMemory_ReadByte(int address)
 * @param address, device address to read from
 * @return value at said address
 * @brief reads one byte from device
 * @warning Default value for this EEPROM is 0xFF */
unsigned char NonVolatileMemory_ReadByte(int address) {
    unsigned char theData;
    I2C1CONbits.SEN = 1; // send START
    while (I2C1CONbits.SEN == 1); // wait for start to finish
    // while (I2C1STATbits.);
    LEDS_SET(0b10000000);

    I2C1TRN = 0b10100000; // send 7-bit slave addy and R/W = 0
    while (I2C1STATbits.TRSTAT == 1); // wait for TX to finish
    //   I2C1CONbits.ACKEN = 0; // initiate ACK sequence
    //   while (I2C1STATbits.ACKSTAT == 1); // wait for ACK to be received
    LEDS_SET(0b10000001);

    I2C1TRN = address >> 8; // send memory address high byte
    while (I2C1STATbits.TRSTAT == 1); // wait for TX to finish
    //    while (I2C1STATbits.ACKSTAT == 1); // wait for ACK to be received
    LEDS_SET(0b10001111);

    I2C1TRN = address; // send memory address low byte
    while (I2C1STATbits.TRSTAT == 1); // wait for TX to finish
    //  while (I2C1STATbits.ACKSTAT == 1); // wait for ACK to be received
    LEDS_SET(0b00011000);

    I2C1CONbits.RSEN = 1; // SEND REPEATED START
    while (I2C1CONbits.RSEN == 1);
    I2C1TRN = 0b10100001; // send 7-bit slave addy and R/W = 1
    while (I2C1STATbits.TRSTAT == 1); // wait for TX to finish
    //  while (I2C1STATbits.ACKSTAT == 1); // wait for ACK to be received
    LEDS_SET(0b11111111);

    I2C1CONbits.RCEN = 1; // enable Rx bit
    while (I2C1CONbits.RCEN == 1); // wait for Rx to finish
    LEDS_SET(0b00011111);

    theData = I2C1RCV; // Receive data from device
    while (I2C1STATbits.TRSTAT == 1); // wait for TX to finish
    
    I2C1CONbits.ACKDT = 1; // initiate NACK
    I2C1CONbits.ACKEN = 1; // initiate ACK sequence
    //  while (I2C1STATbits.ACKSTAT == 1); // wait for RX to complete
    LEDS_SET(0b00011000);

    I2C1CONbits.PEN = 1; // initiate stop condition. NACK
    while (I2C1CONbits.PEN == 1); // wait for this to complete

    return theData;
}

/**
 * @Function char NonVolatileMemory_WriteByte(int address, unsigned char data)
 * @param address, device address to write to
 * @param data, value to write at said address
 * @return SUCCESS or ERROR
 * @brief writes one byte to device */
char NonVolatileMemory_WriteByte(int address, unsigned char data) {
    I2C1CONbits.SEN = 1; // send START
    while (I2C1CONbits.SEN == 1); // wait for start to finish
    // while (I2C1STATbits.);
    LEDS_SET(0b10000000);

    I2C1TRN = 0b10100000; // send 7-bit slave addy and R/W = 0
    while (I2C1STATbits.TRSTAT == 1); // wait for TX to finish
    //   I2C1CONbits.ACKEN = 0; // initiate ACK sequence
    //   while (I2C1STATbits.ACKSTAT == 1); // wait for ACK to be received
    LEDS_SET(0b10000001);

    I2C1TRN = address >> 8; // send memory address high byte
    while (I2C1STATbits.TRSTAT == 1); // wait for TX to finish
    //    while (I2C1STATbits.ACKSTAT == 1); // wait for ACK to be received
    LEDS_SET(0b10001111);

    I2C1TRN = address; // send memory address low byte
    while (I2C1STATbits.TRSTAT == 1); // wait for TX to finish
    //  while (I2C1STATbits.ACKSTAT == 1); // wait for ACK to be received
    LEDS_SET(0b00011000);

    I2C1TRN = data; // send data to be stored
    while (I2C1STATbits.TRSTAT == 1); // wait for TX to finish
   // while (I2C1STATbits.ACKSTAT == 1); // wait for ACK to be received
    LEDS_SET(0b00111100);

    I2C1CONbits.PEN = 1; // initiate stop condition. NACK
    while (I2C1CONbits.PEN == 1); // wait for this to complete
     LEDS_SET(0b11111111);

    return SUCCESS;

}

/**
 * @Function int NonVolatileMemory_ReadPage(int page, char length, unsigned char data[])
 * @param page, page value to read from
 * @param length, value between 1 and 64 bytes to read
 * @param data, array to store values into
 * @return SUCCESS or ERROR
 * @brief reads bytes in page mode, up to 64 at once
 * @warning Default value for this EEPROM is 0xFF */
int NonVolatileMemory_ReadPage(int page, char length, unsigned char data[]) {
    /* IDK do some shit */
    return SUCCESS;
}

/**
 * @Function char int NonVolatileMemory_WritePage(int page, char length, unsigned char data[])
 * @param address, device address to write to
 * @param data, value to write at said address
 * @return SUCCESS or ERROR
 * @brief writes one byte to device */
int NonVolatileMemory_WritePage(int page, char length, unsigned char data[]) {
    /* Do some more shit. Who tf cares */
    return SUCCESS;
}


#ifdef NVM_TEST

int main(void) {
    BOARD_Init();
    NonVolatileMemory_Init();
    Protocol_Init();
    LEDS_INIT();
    unsigned char bitchassData = 0b1111;
    int theaddy = 0x2345;
    int dataOut;
    // ID_NVM_WRITE_BYTE	0X CC 06 98 00 00 00 00 00 B9 C4 0D 0A
    // ID_NVM_READ_BYTE	    0X CC 05 96 00 00 00 00 B9 69 0D 0A
    unsigned char NVM_ID;
    int i;

    while (1) {
        //        if (Protocol_IsMessageAvailable() == TRUE) {
        //            NVM_ID = Protocol_ReadNextID();
        //            if (NVM_ID == ID_NVM_READ_BYTE) {
        //    LEDS_SET(0b1001);
       // dataOut = NonVolatileMemory_ReadByte(theaddy);
        //            } else if (NVM_ID == ID_NVM_WRITE_BYTE) {
        //                LEDS_SET(0b1111);
        //            }
        //        }
//        for (i = 0; i < 10000; i++) {
//            asm("nop");
//        }
         NonVolatileMemory_WriteByte(dataOut, bitchassData);
         for (i = 0; i < 10000; i++) {
            asm("nop");
        }
    }

    while (1);
    BOARD_End();
    return 0;
}
#endif
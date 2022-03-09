#include <proc/p32mx340f512h.h>
#include <stdio.h>
#include "BOARD.h"
#include "NonVolatileMemory.h"
#include "Protocol.h"
#include "MessageIDs.h"

#define SlaveAddress 0x1010000
#define SlaveRead 0x10100001
#define SlaveWrite 0x10100000

extern unsigned char currentID;
extern unsigned char currentLENGTH;
extern unsigned char Lab3PL[MAXPAYLOADLENGTH];

// CC 06 98 00010BC656 B9 E0 0D0A
//          00010BC656

//#define NVM_TEST

struct {
    int address;
    char payload;
    int data;
    unsigned char valAtAdrs;
} WriteByte, ReadByte;

struct {
    int page;
    char length;
    unsigned int address[4];
    unsigned char data[64];
} WritePage, ReadPage;

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

    I2C1TRN = 0b10100000; // send 7-bit slave addy and R/W = 0
    while (I2C1STATbits.TRSTAT == 1); // wait for TX to finish
    //   I2C1CONbits.ACKEN = 0; // initiate ACK sequence
    //   while (I2C1STATbits.ACKSTAT == 1); // wait for ACK to be received
    // LEDS_SET(0b10000001);

    I2C1TRN = address >> 8; // send memory address high byte
    while (I2C1STATbits.TRSTAT == 1); // wait for TX to finish
    //    while (I2C1STATbits.ACKSTAT == 1); // wait for ACK to be received
    // LEDS_SET(0b10001111);

    I2C1TRN = address; // send memory address low byte
    while (I2C1STATbits.TRSTAT == 1); // wait for TX to finish
    //  while (I2C1STATbits.ACKSTAT == 1); // wait for ACK to be received
    //LEDS_SET(0b00011000);

    I2C1CONbits.RSEN = 1; // SEND REPEATED START
    while (I2C1CONbits.RSEN == 1);
    I2C1TRN = 0b10100001; // send 7-bit slave addy and R/W = 1
    while (I2C1STATbits.TRSTAT == 1); // wait for TX to finish
    //  while (I2C1STATbits.ACKSTAT == 1); // wait for ACK to be received
    // LEDS_SET(0b11111111);

    I2C1CONbits.RCEN = 1; // enable Rx bit
    while (I2C1CONbits.RCEN == 1); // wait for Rx to finish
    // LEDS_SET(0b00011111);

    theData = I2C1RCV; // Receive data from device
    while (I2C1STATbits.TRSTAT == 1); // wait for TX to finish

    I2C1CONbits.ACKDT = 1; // initiate NACK
    I2C1CONbits.ACKEN = 1; // initiate ACK sequence
    //  while (I2C1STATbits.ACKSTAT == 1); // wait for RX to complete


    I2C1CONbits.PEN = 1; // initiate stop condition. NACK
    while (I2C1CONbits.PEN == 1); // wait for this to complete
  //  LEDS_SET(0b10101010);
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
    // LEDS_SET(0b10000000);

    I2C1TRN = 0b10100000; // send 7-bit slave addy and R/W = 0
    while (I2C1STATbits.TRSTAT == 1); // wait for TX to finish
    //   I2C1CONbits.ACKEN = 0; // initiate ACK sequence
    //   while (I2C1STATbits.ACKSTAT == 1); // wait for ACK to be received
    // LEDS_SET(0b10000001);

    I2C1TRN = address >> 8; // send memory address high byte
    while (I2C1STATbits.TRSTAT == 1); // wait for TX to finish
    //    while (I2C1STATbits.ACKSTAT == 1); // wait for ACK to be received
    // LEDS_SET(0b10001111);

    I2C1TRN = address; // send memory address low byte
    while (I2C1STATbits.TRSTAT == 1); // wait for TX to finish
    //  while (I2C1STATbits.ACKSTAT == 1); // wait for ACK to be received
    //LEDS_SET(0b00011000);

    I2C1TRN = data; // send data to be stored
    while (I2C1STATbits.TRSTAT == 1); // wait for TX to finish
    // while (I2C1STATbits.ACKSTAT == 1); // wait for ACK to be received
  //  LEDS_SET(0b00111100);


    I2C1CONbits.PEN = 1; // initiate stop condition. NACK
    while (I2C1CONbits.PEN == 1); // wait for this to complete
    //LEDS_SET(0b11101111);

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
    //unsigned char theData;
    int address = page * 64;
    I2C1CONbits.SEN = 1; // send START
    while (I2C1CONbits.SEN == 1); // wait for start to finish
    LEDS_SET(0b10000000);

    I2C1TRN = 0b10100000; // send 7-bit slave addy and R/W = 0
    while ((I2C1STATbits.TRSTAT == 1) || (I2C1STATbits.TBF == 1)); // wait for TX to finish
    while (I2C1STATbits.ACKSTAT == 1); // wait for ACK to be received
    LEDS_SET(0b01000000);
    
    unsigned char addressHIGH = ((address >> 8) & 0x7F);
    I2C1TRN = addressHIGH; // send memory address high byte
    while ((I2C1STATbits.TRSTAT == 1) || (I2C1STATbits.TBF == 1)); // wait for TX to finish
    while (I2C1STATbits.ACKSTAT == 1); // wait for ACK to be received
    LEDS_SET(0b00100000);

    unsigned char addressLOW = address & 0xFF;
    I2C1TRN = addressLOW; // send memory address low byte
    while ((I2C1STATbits.TRSTAT == 1) || (I2C1STATbits.TBF == 1)); // wait for TX to finish
    while (I2C1STATbits.ACKSTAT == 1); // wait for ACK to be received
    LEDS_SET(0b00010000);

    I2C1CONbits.SEN = 1; // SEND REPEATED START
    while (I2C1CONbits.SEN == 1);
    I2C1TRN = 0b10100001; // send 7-bit slave addy and R/W = 1
    while ((I2C1STATbits.TRSTAT == 1) || (I2C1STATbits.TBF == 1)); // wait for TX to finish
    while (I2C1STATbits.ACKSTAT == 1); // wait for ACK to be received
    LEDS_SET(0b00001000);

    for (int i = 0; i < length; i++) {
        I2C1CONbits.RCEN = 1; // enable Rx bit
        while (I2C1CONbits.RCEN == 1); // wait for Rx to finish
        data[i] = I2C1RCV; // Receive data from device
        while (I2C1CONbits.RCEN == 1); // wait for Rx to finish
        //        while (I2C1STATbits.TRSTAT == 1); // wait for TX to finish

        if (i < length - 1) {
            I2C1CONbits.ACKDT = 0; // initiate ACK
            I2C1CONbits.ACKEN = 1; // initiate ACK sequence
            while (I2C1CONbits.ACKEN == 1); // wait for Rx to finish
        } else {
            I2C1CONbits.ACKDT = 1; // initiate NACK
            I2C1CONbits.ACKEN = 1; // initiate ACK sequence
            while (I2C1CONbits.ACKEN == 1); // wait for Rx to finish
            //  while (I2C1STATbits.ACKSTAT == 1); // wait for RX to complete
            //LEDS_SET(0b00011000);
        }
    }

    I2C1CONbits.PEN = 1; // initiate stop condition. NACK
    while (I2C1CONbits.PEN == 1); // wait for this to complete
    LEDS_SET(0b11111111);
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
    int address = page * 64;
    I2C1CONbits.SEN = 1; // send START
    while (I2C1CONbits.SEN == 1); // wait for start to finish
    // while (I2C1STATbits.);
     LEDS_SET(0b10000000);

    I2C1TRN = 0b10100000; // send 7-bit slave addy and R/W = 0
    while ((I2C1STATbits.TRSTAT == 1) || (I2C1STATbits.TBF == 1)); // wait for TX to finish
    //   I2C1CONbits.ACKEN = 0; // initiate ACK sequence
    while (I2C1STATbits.ACKSTAT == 1); // wait for ACK to be received
     LEDS_SET(0b10000001);

    unsigned char addressHIGH = ((address >> 8) & 0x7F); // get rid of top bits
    I2C1TRN = addressHIGH; // send memory address high byte
    while ((I2C1STATbits.TRSTAT == 1) || (I2C1STATbits.TBF == 1)); // wait for TX to finish
    while (I2C1STATbits.ACKSTAT == 1); // wait for ACK to be received
    LEDS_SET(0b00000011);

    unsigned char addressLOW = address & 0xFF;
    I2C1TRN = addressLOW; // send memory address low byte
    while ((I2C1STATbits.TRSTAT == 1) || (I2C1STATbits.TBF == 1)); // wait for TX to finish
    while (I2C1STATbits.ACKSTAT == 1); // wait for ACK to be received
    LEDS_SET(0b000000111);
    
    for (int i = 0; i < length;) {
        I2C1TRN = data[i]; // send data to be stored
        while ((I2C1STATbits.TRSTAT == 1) || (I2C1STATbits.TBF == 1)); // wait for TX to finish
        while (I2C1STATbits.ACKSTAT == 1); // wait for ACK to be received
        i++;
        //LEDS_SET(0b00111100);
    }

    I2C1CONbits.PEN = 1; // initiate stop condition. NACK
    while (I2C1CONbits.PEN == 1); // wait for this to complete
    LEDS_SET(0b11101111);

    return SUCCESS;

}


#ifdef NVM_TEST

int main(void) {
    BOARD_Init();
    NonVolatileMemory_Init();
    Protocol_Init();
    LEDS_INIT();
    int theaddy;
    char *addressPayload;
    unsigned char *msg = "Successful write";
    int dataOut;
    // ID_NVM_WRITE_BYTE	0XCC 06 98 00010BC6 56 B9 E0 0D0A
    //ID_NVM_WRITE_BYTE_ACK	0X         00010BC6 56
    //0XCC459C00000503CB05C41737831D5B99204EE435F24D12A6F5C288A994C2EC58A11D4CC9C5D598F0F160AD39709F9FAF27B4997D4317C77EAA971643D52D289BE0444D347C0903B9130D0A

    unsigned char NVM_ID;
    int i;

    while (1) {

        if (Protocol_IsMessageAvailable() == TRUE) {
            // LEDS_SET(0b11111111);
            // NVM_ID = Protocol_ReadNextID();
            // 0000AE1C41
// 000004C1 17D84026F2C20D5358F57C57A55A84649180403BC17D181920D132F4E1FE879BB9D0858FAEF2BC40CC69D5A88D1CC70A83740D5509628BEBA443AE
            if (currentID == ID_NVM_READ_BYTE) {
                ReadByte.data = Protocol_IntEndednessConversion((int) Lab3PL);
                unsigned char thedata = NonVolatileMemory_ReadByte(ReadByte.data);
                Protocol_SendMessage(2, ID_NVM_READ_BYTE_RESP, &thedata);
                
            } else if (currentID == ID_NVM_WRITE_BYTE) {
                // WriteByte.payload = Lab3PL;
                WriteByte.address = Protocol_IntEndednessConversion((int) Lab3PL);
                WriteByte.valAtAdrs = Lab3PL[4];
                NonVolatileMemory_WriteByte(WriteByte.address, WriteByte.valAtAdrs);
                Protocol_SendMessage(2, ID_NVM_WRITE_BYTE_ACK, "ACK");
                
            } else if (currentID == ID_NVM_READ_PAGE) {
                ReadPage.page = Protocol_IntEndednessConversion((int) Lab3PL);
                NonVolatileMemory_ReadPage(ReadPage.page, 64, WritePage.data);
                Protocol_SendMessage(65, ID_NVM_READ_PAGE_RESP, WritePage.data);
               // Protocol_SendMessage(64, ID_NVM_READ_PAGE_RESP, &Lab3PL);
                
            } else if (currentID == ID_NVM_WRITE_PAGE) {
                for (int i = 0; i < 68; i++) {
                    if (i <= 4) {
                        WritePage.address[i] = Lab3PL[i];
                    } else {
                        WritePage.data[i - 4] = Lab3PL[i];
                    }
                }
                WritePage.page = Protocol_IntEndednessConversion((int)WritePage.address);
                NonVolatileMemory_WritePage(WritePage.page, 64, WritePage.data);
                Protocol_SendMessage(2, ID_NVM_WRITE_PAGE_ACK, "ACK");
            }
        }
        for (i = 0; i < 10000; i++) {
            asm("nop");
        }
    }

    while (1);
    BOARD_End();
    return 0;
}
#endif
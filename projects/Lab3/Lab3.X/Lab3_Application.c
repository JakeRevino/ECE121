#include <proc/PIC32MX/p32mx340f512h.h>
#include <sys/attribs.h>
#include <stdio.h>
#include <stdlib.h>
#include "ADCFilter.h"
#include "MessageIDs.h"
#include "Protocol.h"
#include "BOARD.h"
#include "FreeRunningTimer.h"
#include "FrequencyGenerator.h"
#include "NonVolatileMemory.h"

// handy dandy externs
extern char Lab3PL[65];
extern short weightsPL[32];

// Define variables
#define NUMOFCHANNELS 4
#define NUMOFFILTERS 2
#define MAXPAGELENGTH 64
#define BIT_MAX 1023
#define BIT_FRACTION (BIT_MAX >> 3)
#define PIN0 100
#define PIN1 300
#define PIN2 500
#define PIN3 700
#define TRUE 0x01
#define FALSE 0x00

// Set-up the switches 
#define SW1 PORTDbits.RD8
#define SW2 PORTDbits.RD9
#define SW3 PORTDbits.RD10
#define SW4 PORTDbits.RD11

// Set up switch change-states 
#define Switch_Change() ((PORTD >> 8) & 0x07)
#define Switch_Change_Channel() ((PORTD >> 8) & 0x03)
#define Switch_Change_Filter() ((PORTD >> 8) & 0x04)

void NOP_Delay(unsigned int delayCount);

void NOP_Delay(unsigned int delayCount) {
    for (int i = 0; i < delayCount; i++) {
        asm("nop");
    }
}

static short LowPass[32] = {-54, -64, -82, -97, -93, -47, 66, 266, 562, 951, 1412, 1909, 2396, 2821, 3136, 3304, 3304, 3136, 2821, 2396, 1909, 1412, 951, 562, 266, 66, -47, -93, -97, -82, -64, -54};
static short HighPass[32] = {0, 0, 39, -91, 139, -129, 0, 271, -609, 832, -696, 0, 1297, -3011, 4755, -6059, 6542, -6059, 4755, -3011, 1297, 0, -696, 832, -609, 271, 0, -129, 139, -91, 39, 0};
static short FilterArray[NUMOFCHANNELS][NUMOFFILTERS][FILTERLENGTH];
//                         channel#       filter#          32 

#ifdef LAB3_APPLICATION

int main(void) {
    BOARD_Init();
    Protocol_Init();
    ADCFilter_Init();
    FreeRunningTimer_Init();
    FrequencyGenerator_Init();
    NonVolatileMemory_Init();
    LEDS_INIT();

    // Send Debug Alive message
    char alive_message[MAXPAYLOADLENGTH];
    sprintf(alive_message, "Lab3 Application online at: %s %s", __TIME__, __DATE__);
    Protocol_SendDebugMessage(alive_message);

    // Initialize switches as input
    TRISDbits.TRISD8 = 1;
    TRISDbits.TRISD9 = 1;
    TRISDbits.TRISD10 = 1;
    TRISDbits.TRISD11 = 1;

    unsigned char pin;
    unsigned char filter;
    unsigned char change;

    short filterWeights[FILTERLENGTH];
    short ReadingBuffer[2];
    short minimum;
    short maximum;
    short RawReading;
    short FilteredReading;

    unsigned int pinAddress;
    unsigned int NVM_Confirm;
    unsigned int x;
    unsigned int y;
    unsigned int P2PorABS;
    unsigned int freq2set;

    uint8_t swStatus = Switch_Change();
    uint8_t swChannel = Switch_Change_Channel();
    uint8_t swFilter = Switch_Change_Filter();

    // Setup switch channel
    if (swChannel == 3) {
        pin = 3;
        pinAddress = PIN3;
    } else if (swChannel == 2) {
        pin = 2;
        pinAddress = PIN2;
    } else if (swChannel == 1) {
        pin = 1;
        pinAddress = PIN1;
    } else if (swChannel == 0) {
        pin = 0;
        pinAddress = PIN0;
    }

    filter = SW3;
    change = (pin << 4) | (filter);
    Protocol_SendMessage(2, ID_LAB3_CHANNEL_FILTER, &change);
    NOP_Delay(1600);
    NVM_Confirm = NonVolatileMemory_ReadPage(pinAddress, MAXPAGELENGTH, (unsigned char *) FilterArray[pin][filter]);

    for (x = 0; x < FILTERLENGTH; x++) {
        filterWeights[x] = FilterArray[pin][filter][x];
    }
    ADCFilter_SetWeights((short) pin, filterWeights);

    if (SW4 == 1) {
        minimum = filterWeights[0];
        maximum = filterWeights[0];
        for (x = 0; x < FILTERLENGTH; x++) {
            if (filterWeights[x] > maximum) {
                maximum = filterWeights[x];
            }
            if (filterWeights[x] < minimum) {
                minimum = filterWeights[x];
            }
        }
        P2PorABS = abs(maximum - minimum); // Peak-to-peak 
    } else {
        P2PorABS = abs(RawReading); // Absolute value
    }

    unsigned int currentMilli = FreeRunningTimer_GetMilliSeconds();
    unsigned int previousMilli = currentMilli;

    while (1) {
        if (Protocol_IsMessageAvailable() == TRUE) {

            if (Protocol_ReadNextID() == ID_LAB3_FREQUENCY_ONOFF) {
                if (Lab3PL[0] == 1) {
                    FrequencyGenerator_On();
                    // LEDS_SET(0b11011);
                } else {
                    FrequencyGenerator_Off();
                    // LEDS_SET(0b01);
                }

            } else if (Protocol_ReadNextID() == ID_LAB3_SET_FREQUENCY) {
                freq2set = Lab3PL[0] | Lab3PL[1] << 8; // this moves Lab3PL[1] to the zero position
                freq2set = Protocol_ShortEndednessConversion(freq2set);
                FrequencyGenerator_SetFrequency(freq2set);
                // Protocol_SendMessage(sizeof (freq2set) + 1, ID_ADC_FILTER_VALUES_RESP, &freq2set);

            } else if (Protocol_ReadNextID() == ID_ADC_SELECT_CHANNEL) {
                pinAddress = Lab3PL[0];
                Protocol_SendMessage(2, ID_ADC_SELECT_CHANNEL_RESP, &pinAddress);



            } else if (Protocol_ReadNextID() == ID_ADC_FILTER_VALUES) {
                if (Lab3PL[0] == -1) { // this is the first char of the low pass filter 
                    for (x = 0; x < FILTERLENGTH; x++) { // loop for endedness
                        filterWeights[x] = Protocol_ShortEndednessConversion(LowPass[x]);
                    }
                    ADCFilter_SetWeights((short) pin, LowPass);
                    Protocol_SendMessage(2, ID_ADC_FILTER_VALUES_RESP, &pin);
                    for (x = 0; x < FILTERLENGTH; x++) { // loop to populate NVM
                        FilterArray[pin][filter][x] = filterWeights[x];
                    }
                } else if (Lab3PL[0] == 0) { // this is the first char of the high pass filter
                    for (x = 0; x < FILTERLENGTH; x++) { // loop for endedness
                        filterWeights[x] = Protocol_ShortEndednessConversion(HighPass[x]);
                    }
                    ADCFilter_SetWeights((short) pin, HighPass);
                    Protocol_SendMessage(2, ID_ADC_FILTER_VALUES_RESP, &pin);
                    for (x = 0; x < FILTERLENGTH; x++) { // loop to populate NVM
                        FilterArray[pin][filter][x] = filterWeights[x];
                    }
                }
                NVM_Confirm = NonVolatileMemory_WritePage(pin, MAXPAGELENGTH, (unsigned char *) FilterArray[pin][filter]);
                if (NVM_Confirm == ERROR) {
                    NonVolatileMemory_Init();
                } else {
                    Protocol_SendMessage(1, ID_NVM_WRITE_BYTE_ACK, 0);
                }
            }
        }


        currentMilli = FreeRunningTimer_GetMilliSeconds();
        if ((currentMilli - previousMilli) >= 10) {

            RawReading = ADCFilter_RawReading((short) pin); // take a raw reading
            ReadingBuffer[0] = Protocol_ShortEndednessConversion(RawReading); // convert raw reading to be sent to 121-Interface
            FilteredReading = ADCFilter_FilteredReading((short) pin); // take filtered reading
            ReadingBuffer[1] = Protocol_ShortEndednessConversion(FilteredReading); // convert filtered reading to be sent to 121-Interface
            Protocol_SendMessage(5, ID_ADC_READING, &ReadingBuffer);
            previousMilli = currentMilli;


            // Now must do switch checking...
            if (Switch_Change() != swStatus) {
                if (Switch_Change_Channel() != swChannel) { // check the channels
                    if ((SW2 == 1) && (SW1 == 1)) {
                        pin = 3;
                        pinAddress = PIN3;
                    } else if ((SW2 == 1) && (SW1 == 0)) {
                        pin = 2;
                        pinAddress = PIN2;
                    } else if ((SW2 == 0) && (SW1 == 1)) {
                        pin = 1;
                        pinAddress = PIN1;
                    } else if ((SW2 == 0) && (SW1 == 0)) {
                        pin = 0;
                        pinAddress = PIN0;
                    }
                }
                swChannel = Switch_Change_Channel(); // update the channels

                if (Switch_Change_Filter() != swFilter) { // check the filter
                    filter = SW3;
                }
                swFilter = Switch_Change_Filter(); // update the filter

                change = (pin << 4) | (filter); // prepare notification of filter on pin
                Protocol_SendMessage(2, ID_LAB3_CHANNEL_FILTER, &change);

                NVM_Confirm = NonVolatileMemory_ReadPage(pinAddress, MAXPAGELENGTH, (unsigned char *) FilterArray[pin][filter]);
                for (x = 0; x < FILTERLENGTH; x++) {
                    filterWeights[x] = FilterArray[pin][filter][x];
                }
                ADCFilter_SetWeights((short) pin, filterWeights);
            }
            swStatus = Switch_Change();




            if (SW4 == 1) { // if in peak to peak mode
                minimum = filterWeights[0];
                maximum = filterWeights[0];
                for (x = 0; x < FILTERLENGTH; x++) {
                    if (filterWeights[x] > maximum) {
                        maximum = filterWeights[x];
                    }
                    if (filterWeights[x] < minimum) {
                        minimum = filterWeights[x];
                    }
                }
                P2PorABS = abs(maximum - minimum); // Peak-to-peak 
            } else {
                P2PorABS = abs(RawReading); // Absolute value
            }

            for (x = 0, y = 0; x < P2PorABS; x++) {
                if ((x % BIT_FRACTION) == 0) {
                    if (y == 0) {
                        y = 1;
                    } else {
                        y |= y << 1;
                    }
                }
            }
            LEDS_SET(y >> 1);
        }
    }

    return 0;
    while (1);
    BOARD_End();
}
// }
#endif

//  Protocol_SendMessage(sizeof(CurrentFrequency) + 1, ID_NVM_READ_BYTE_RESP, &CurrentFrequency);
//                 Protocol_SendMessage(4, ID_NVM_READ_BYTE_RESP, &weightsPL);
// Protocol_SendMessage(sizeof (CurrentFrequency) + 1, ID_NVM_READ_BYTE_RESP, &CurrentFrequency);
























//#include <proc/PIC32MX/p32mx340f512h.h>
//#include <sys/attribs.h>
//#include <stdio.h>
//#include <stdlib.h>
//#include "ADCFilter.h"
//#include "MessageIDs.h"
//#include "Protocol.h"
//#include "BOARD.h"
//#include "FreeRunningTimer.h"
//#include "FrequencyGenerator.h"
//#include "NonVolatileMemory.h"
//
//extern char Lab3PL[65];
//extern short weightsPL[32];
//
//void NOP_Delay(unsigned int delayCount);
//
//void NOP_Delay(unsigned int delayCount) {
//    for (int i = 0; i < delayCount; i++) {
//        asm("nop");
//    }
//}
//
///* Define variables */
//#define NUMOFCHANNELS 4
//#define NUMOFFILTERS 2
//#define MAXPAGELENGTH 64
//#define BIT_MAX 1023
//#define BIT_FRACTION (BIT_MAX >> 3)
//#define PIN0 100
//#define PIN1 300
//#define PIN2 500
//#define PIN3 700
//#define TRUE 0x01
//#define FALSE 0x00
//
///* Set-up the switches */
//#define SW1 PORTDbits.RD8
//#define SW2 PORTDbits.RD9
//#define SW3 PORTDbits.RD10
//#define SW4 PORTDbits.RD11
//
///* Set up change states */
//#define Switch_Change() ((PORTD >> 8) & 0x07)
//#define Switch_Change_Channel() ((PORTD >> 8) & 0x03)
//#define Switch_Change_Filter() ((PORTD >> 8) & 0x04)
//
//
//short LowPass[32] = {-54, -64, -82, -97, -93, -47, 66, 266, 562, 951, 1412, 1909, 2396, 2821, 3136, 3304, 3304, 3136, 2821, 2396, 1909, 1412, 951, 562, 266, 66, -47, -93, -97, -82, -64, -54};
//short HighPass[32] = {0, 0, 39, -91, 139, -129, 0, 271, -609, 832, -696, 0, 1297, -3011, 4755, -6059, 6542, -6059, 4755, -3011, 1297, 0, -696, 832, -609, 271, 0, -129, 139, -91, 39, 0};
//
//static short FilterArray[NUMOFCHANNELS][NUMOFFILTERS][FILTERLENGTH];
////                         channel#       filter#          32 
//
//#ifdef LAB3_APPLICATION
//
//int main(void) {
//    BOARD_Init();
//    Protocol_Init();
//    ADCFilter_Init();
//    FreeRunningTimer_Init();
//    FrequencyGenerator_Init();
//    NonVolatileMemory_Init();
//    LEDS_INIT();
//
//    unsigned char FilterFrequency[FILTERLENGTH];
//    unsigned char pin;
//    unsigned char filter;
//    unsigned char change;
//    unsigned char frequencyState;
//    char filterLength = 32;
//
//    unsigned int pinAddress;
//    unsigned int R_or_W_Check;
//    unsigned int x;
//    unsigned int y;
//    unsigned int P2P_or_Abs;
//    unsigned int freq2set;
//
//    short filterWeights[FILTERLENGTH];
//    short ReadingBuffer[2];
//    short minimum;
//    short maximum;
//    short RawReading;
//    short FilteredReading;
//    short CurrentFrequency;
//
//    /* Initialize switches as input */
//    TRISDbits.TRISD8 = 1;
//    TRISDbits.TRISD9 = 1;
//    TRISDbits.TRISD10 = 1;
//    TRISDbits.TRISD11 = 1;
//
//    uint8_t swStatus = Switch_Change(); // indicates a change in switches
//    uint8_t swChannel = Switch_Change_Channel(); // indicates a change in channel
//    uint8_t swFilter = Switch_Change_Filter(); // indicates a change in filter
//
//    // 1. Send Debug Alive message
//    char alive_message[MAXPAYLOADLENGTH];
//    sprintf(alive_message, "Lab3 Application online at: %s %s", __TIME__, __DATE__);
//    Protocol_SendDebugMessage(alive_message);
//
//    /* Setup switch channel */
//    if (swChannel == 3) {
//        pin = 3;
//        pinAddress = PIN3;
//    } else if (swChannel == 2) {
//        pin = 2;
//        pinAddress = PIN2;
//    } else if (swChannel == 1) {
//        pin = 1;
//        pinAddress = PIN1;
//    } else if (swChannel == 0) {
//        pin = 0;
//        pinAddress = PIN0;
//    }
//
//    filter = SW3;
//    change = (pin << 4) | (filter);
//    Protocol_SendMessage(2, ID_LAB3_CHANNEL_FILTER, &change);
//    NOP_Delay(1600);
//
//    // Load existing filter coefficients from NVM
//    R_or_W_Check = NonVolatileMemory_ReadPage(pinAddress, MAXPAGELENGTH, (unsigned char *) FilterArray[pin][filter]);
//
//    for (x = 0; x < FILTERLENGTH; x++) {
//        filterWeights[x] = FilterArray[pin][filter][x];
//    }
//    ADCFilter_SetWeights((short) pin, filterWeights);
//
//    // use high pass or low pass based on switch 3
//    //    if (SW3 == 0) {
//    //        ADCFilter_SetWeights((short) pin, LowPass);
//    //    } else if (SW3 == 1) {
//    //        ADCFilter_SetWeights((short) pin, HighPass);
//    //    }
//
//    // Display either peak-to-peak or absolute value
//    if (SW4 == 1) {
//        minimum = filterWeights[0];
//        maximum = filterWeights[0];
//        for (x = 0; x < FILTERLENGTH; x++) {
//            if (filterWeights[x] > maximum) {
//                maximum = filterWeights[x];
//            }
//            if (filterWeights[x] < minimum) {
//                minimum = filterWeights[x];
//            }
//        }
//        P2P_or_Abs = abs(maximum - minimum); // Peak-to-peak 
//    } else {
//        P2P_or_Abs = abs(RawReading); // Absolute value
//    }
//
//
//    unsigned int currentMilli;
//    unsigned int previousMilli;
//    currentMilli = FreeRunningTimer_GetMilliSeconds();
//    previousMilli = currentMilli;
//
//    while (1) {
//        if (Protocol_IsMessageAvailable() == TRUE) {
//
//            if (Protocol_ReadNextID() == ID_LAB3_SET_FREQUENCY) {
//                freq2set = Lab3PL[0] | Lab3PL[1] << 8; // this moves Lab3PL[1] to the zero position
//                FrequencyGenerator_SetFrequency(freq2set);
//                Protocol_SendMessage(sizeof (freq2set) + 1, ID_ADC_FILTER_VALUES_RESP, &freq2set);
//
//
//            } else if (Protocol_ReadNextID() == ID_ADC_FILTER_VALUES) {
//                
//            }
//        }
//        
//        
//        
//        
//        
//        // determine if a change to the switches has been made
//        if (Switch_Change() != swStatus) {
//            if (Switch_Change_Channel() != swChannel) { // determine channel based on SW1 and SW2
//                if ((SW2 == 1) && (SW1 == 1)) {
//                    pin = 3;
//                    pinAddress = PIN3;
//                } else if ((SW2 == 1) && (SW1 == 0)) {
//                    pin = 2;
//                    pinAddress = PIN2;
//                } else if ((SW2 == 0) && (SW1 == 1)) {
//                    pin = 1;
//                    pinAddress = PIN1;
//                } else if ((SW2 == 0) && (SW1 == 0)) {
//                    pin = 0;
//                    pinAddress = PIN0;
//                }
//            }
//            swChannel = Switch_Change_Channel(); // update current channel
//
//            if (Switch_Change_Filter() != swFilter) { // make sure the right filter is being used
//                filter = SW3;
//            }
//            swFilter = Switch_Change_Filter(); // update current filter
//
//            change = (pin << 4) | (filter);
//            Protocol_SendMessage(sizeof (change) + 1, ID_LAB3_CHANNEL_FILTER, &change); // send message to notify a change has been made
//
//            R_or_W_Check = NonVolatileMemory_ReadPage(pinAddress, MAXPAGELENGTH, (unsigned char *) FilterArray[pin][filter]);
////            for (x = 0; x < FILTERLENGTH; x++) {
////                filterWeights[x] = FilterArray[pin][filter][x];
////            }
//            if (SW3 == 0) {
//                ADCFilter_SetWeights((short) pin, LowPass);
//            } else if (SW3 == 1) {
//                ADCFilter_SetWeights((short) pin, HighPass);
//            }
//        }
//
//
//    }
//
//    return 0;
//    while (1);
//    BOARD_End();
//}
//// }
//#endif
//
////  Protocol_SendMessage(sizeof(CurrentFrequency) + 1, ID_NVM_READ_BYTE_RESP, &CurrentFrequency);
////                 Protocol_SendMessage(4, ID_NVM_READ_BYTE_RESP, &weightsPL);
//// Protocol_SendMessage(sizeof (CurrentFrequency) + 1, ID_NVM_READ_BYTE_RESP, &CurrentFrequency);
//







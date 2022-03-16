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

extern char Lab3PL[65];
extern short weightsPL[32];

void NOP_Delay(unsigned int delayCount);

void NOP_Delay(unsigned int delayCount) {
    for (int i = 0; i < delayCount; i++) {
        asm("nop");
    }
}

/* Define variables */
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

/* Set-up the switches */
#define SW1 PORTDbits.RD8
#define SW2 PORTDbits.RD9
#define SW3 PORTDbits.RD10
#define SW4 PORTDbits.RD11

/* Set up change states */
#define Switch_Change() ((PORTD >> 8) & 0x07)
#define Switch_Change_Channel() ((PORTD >> 8) & 0x03)
#define Switch_Change_Filter() ((PORTD >> 8) & 0x04)


short LowPass[32] = {-54, -64, -82, -97, -93, -47, 66, 266, 562, 951, 1412, 1909, 2396, 2821, 3136, 3304, 3304, 3136, 2821, 2396, 1909, 1412, 951, 562, 266, 66, -47, -93, -97, -82, -64, -54};
short HighPass[32] = {0, 0, 39, -91, 139, -129, 0, 271, -609, 832, -696, 0, 1297, -3011, 4755, -6059, 6542, -6059, 4755, -3011, 1297, 0, -696, 832, -609, 271, 0, -129, 139, -91, 39, 0};

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

    unsigned char FilterFrequency[FILTERLENGTH];
    unsigned char pin;
    unsigned char filter;
    unsigned char change;
    unsigned char frequencyState;

    unsigned int pinAddress;
    unsigned int R_or_W_Check;
    unsigned int x;
    unsigned int y;
    unsigned int P2P_or_Abs;

    uint8_t swStatus = Switch_Change();
    uint8_t swChannel = Switch_Change_Channel();
    uint8_t swFilter = Switch_Change_Filter();

    // 1. Send Debug Alive message
    char alive_message[MAXPAYLOADLENGTH];
    sprintf(alive_message, "Lab3 Application online at: %s %s", __TIME__, __DATE__);
    Protocol_SendDebugMessage(alive_message);


    short filterWeights[FILTERLENGTH];
    short ReadingBuffer[2];
    short minimum;
    short maximum;
    short RawReading;
    short FilteredReading;
    short CurrentFrequency;
    char filterLength = 32;


    /* Initialize switches as input */
    TRISDbits.TRISD8 = 1;
    TRISDbits.TRISD9 = 1;
    TRISDbits.TRISD10 = 1;
    TRISDbits.TRISD11 = 1;

    /* Setup switch channel */
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
    R_or_W_Check = NonVolatileMemory_ReadPage(pinAddress, MAXPAGELENGTH, (unsigned char *) FilterArray[pin][filter]);

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
        P2P_or_Abs = abs(maximum - minimum); // Peak-to-peak 
    } else {
        P2P_or_Abs = abs(RawReading); // Absolute value
    }


    unsigned int currentMilli;
    unsigned int previousMilli;
    currentMilli = FreeRunningTimer_GetMilliSeconds();
    previousMilli = currentMilli;

    while (1) {
        if (Protocol_IsMessageAvailable() == TRUE) {

            if (Protocol_ReadNextID() == ID_LAB3_SET_FREQUENCY) {
                CurrentFrequency = *weightsPL;
              //  Protocol_SendMessage(3, ID_NVM_READ_BYTE_RESP, &Lab3PL);
              //  Protocol_SendMessage(4, ID_NVM_READ_BYTE_RESP, &weightsPL);

                // CurrentFrequency = Protocol_ShortEndednessConversion(CurrentFrequency);
                //  Protocol_SendMessage(4, ID_NVM_READ_BYTE_RESP, &CurrentFrequency);
                FrequencyGenerator_SetFrequency(CurrentFrequency);

            }
            else if (Protocol_ReadNextID() == ID_LAB3_FREQUENCY_ONOFF) {
                if (Lab3PL[0] == 1) {
                    FrequencyGenerator_On();
                   // LEDS_SET(0b11011);
                } else {
                    FrequencyGenerator_Off();
                   // LEDS_SET(0b01);
                }
            }
            else if (Protocol_ReadNextID() == ID_ADC_SELECT_CHANNEL) {
                pinAddress = Lab3PL[0];
                Protocol_SendMessage(2, ID_ADC_SELECT_CHANNEL_RESP, &pinAddress);
            }
            else if (Protocol_ReadNextID() == ID_ADC_FILTER_VALUES) {
                if ((Lab3PL[0] == -1) && (SW3 == 0)) {
                    ADCFilter_SetWeights((short) pin, LowPass);
                    for (x = 0; x < FILTERLENGTH; x++) {
                        filterWeights[x] = Protocol_ShortEndednessConversion(LowPass[x]);
                    }
                    Protocol_SendMessage(sizeof (filterWeights) + 1, ID_ADC_FILTER_VALUES_RESP, &filterWeights);

                } else if ((Lab3PL[1] == 0) && (SW3 == 1)) {
                    ADCFilter_SetWeights((short) pin, HighPass);
                    for (x = 0; x < FILTERLENGTH; x++) {
                        filterWeights[x] = Protocol_ShortEndednessConversion(HighPass[x]);
                    }
                    Protocol_SendMessage(sizeof (filterWeights) + 1, ID_ADC_FILTER_VALUES_RESP, &filterWeights);

                    for (x = 0; x < FILTERLENGTH; x++) {
                        FilterArray[pin][filter][x] = filterWeights[x];
                    }

                    R_or_W_Check = NonVolatileMemory_WritePage(pin, MAXPAGELENGTH, (unsigned char *) FilterArray[pin][filter]);
                    if (R_or_W_Check == ERROR) {
                        NonVolatileMemory_Init();

                    } else {
                        Protocol_SendMessage(1, ID_NVM_WRITE_BYTE_ACK, 0);
                    }

                }
            }

            }
            currentMilli = FreeRunningTimer_GetMilliSeconds();
            if ((currentMilli - previousMilli) >= 10) {

                //  RawReading = ADCFilter_RawReading((short)pin);
                ReadingBuffer[0] = Protocol_ShortEndednessConversion(ADCFilter_RawReading((short) pin)); //Protocol_ShortEndednessConversion(RawReading);
                // FilteredReading = ADCFilter_FilteredReading((short)pin);
                ReadingBuffer[1] = Protocol_ShortEndednessConversion(ADCFilter_FilteredReading((short) pin));
                Protocol_SendMessage(sizeof (ReadingBuffer) + 1, ID_ADC_READING, &ReadingBuffer);
                previousMilli = currentMilli;

                if (Switch_Change() != swStatus) {
                    if (Switch_Change_Channel() != swChannel) {
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
                    swChannel = Switch_Change_Channel();

                    if (Switch_Change_Filter() != swFilter) {
                        filter = SW3;
                    }
                    swFilter = Switch_Change_Filter();

                    change = (pin << 4) | (filter);
                    Protocol_SendMessage(sizeof(change) + 1, ID_LAB3_CHANNEL_FILTER, &change);

                    R_or_W_Check = NonVolatileMemory_ReadPage(pinAddress, MAXPAGELENGTH, (unsigned char *) FilterArray[pin][filter]);
                    for (x = 0; x < FILTERLENGTH; x++) {
                        filterWeights[x] = FilterArray[pin][filter][x];
                    }
                    if (SW3 == 0) {
                    ADCFilter_SetWeights((short) pin, LowPass);
                    } else if (SW3 == 1) {
                        ADCFilter_SetWeights((short)pin, HighPass);
                    }
                }
                swStatus = Switch_Change();

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
                    P2P_or_Abs = abs(maximum - minimum); // Peak-to-peak 
                } else {
                    P2P_or_Abs = abs(RawReading); // Absolute value
                }

                for (x = 0, y = 0; x < P2P_or_Abs; x++) {
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


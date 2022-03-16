#include <proc/PIC32MX/p32mx340f512h.h>
#include <sys/attribs.h>
#include <stdio.h>
#include "ADCFilter.h"
#include "MessageIDs.h"
#include "Protocol.h"
#include "BOARD.h"
#include "FreeRunningTimer.h"
#include "FrequencyGenerator.h"

#define NUMBEROFCHANNELS 4

extern char Lab3PL[65];
extern short weightsPL[32];

//static signed short FilterBuffer[NUMBEROFCHANNELS][FILTERLENGTH];

static struct {
    signed short tail;
    signed short head;
    signed short data[NUMBEROFCHANNELS][FILTERLENGTH];
} DataBuffer;

static struct {
    signed short tail;
    signed short head;
    signed short data[NUMBEROFCHANNELS][FILTERLENGTH];
} FilteredBuffer;

/**
 * @Function ADCFilter_Init(void)
 * @param None
 * @return SUCCESS or ERROR
 * @brief initializes ADC system along with naive filters */
int ADCFilter_Init(void) {
    AD1CON1 = 0; // clear and reset it all
    AD1CON2 = 0;
    AD1CON3 = 0;

    for (int i = 0; i < NUMBEROFCHANNELS; i++) {
        for (int j = 0; j < FILTERLENGTH; j++) {
            DataBuffer.data[i][j] = 0;
            DataBuffer.head = 0;
            FilteredBuffer.data[i][j] = 0;
            FilteredBuffer.head = 0;
        }
    }

    // STEP 1: connect desired pins to ADC
//    TRISBbits.TRISB2 = 1; // A0, AN2, RB2
//    TRISBbits.TRISB4 = 1; // A1, AN4, RB4
//    TRISBbits.TRISB8 = 1; // A2, AN8, RB8
//    TRISBbits.TRISB10 = 1; // A3, AN10, RB10
    //AD1PCFG = 0xFAEB;
    AD1PCFGbits.PCFG2 = 0;
    AD1PCFGbits.PCFG4 = 0;
    AD1PCFGbits.PCFG8 = 0;
    AD1PCFGbits.PCFG10 = 0;

    // STEP 2: connect same pins to scanner (.CSSL)
   // AD1CSSL = 0x0541; // this selects AN2, AN4, AN8, and AN10 for input scan 
    AD1CSSLbits.CSSL2 = 1;
    AD1CSSLbits.CSSL4 = 1;
    AD1CSSLbits.CSSL8 = 1;
    AD1CSSLbits.CSSL10 = 1;

    // STEP 3:
    AD1CON1bits.ASAM = 1; // AUTO-SAMPLE. Sampling begins immediately after last conversion completes; SAMP bit is automatically set
    AD1CON1bits.SSRC = 0b111; // AUTO-CONVERT
    AD1CON1bits.FORM = 0b000; // Integer 16-bit

    // STEP 4: 
    AD1CON2bits.VCFG = 0b000; // internal voltage as reference (AVdd & AVss)
    AD1CON2bits.CSCNA = 1; // scan inputs. SCAN MODE
    AD1CON2bits.SMPI = 0b0011; // Interrupts at the completion of conversion for each sample/convert sequence

    // STEP 5:
    AD1CON2bits.BUFM = 0; // Buffer configured as one 16-word buffer
    AD1CON3bits.ADRC = 0; // Tad uses Peripheral Bus Clock

    // STEP 6:
    AD1CON3bits.ADCS = 0b10101101; // 173. This sets Tad to 348*Tpb

    // STEP 7: 
    AD1CON3bits.SAMC = 0b10000; // 16. This sets the sample time to 16*Tad

    // STEP 8: Configure Interrupts
    IPC6bits.AD1IP = 7; // interrupt priority
    IPC6bits.AD1IS = 2; // interrupt sub-priority
    IFS1bits.AD1IF = 0; // clear interrupt flag

    // STEP 9: Enable it all
    IEC1bits.AD1IE = 1; // enable interrupts
    AD1CON1bits.ON = 1; // turn on ADC1

    return SUCCESS;
}

/**
 * @Function ADCFilter_RawReading(short pin)
 * @param pin, which channel to return
 * @return un-filtered AD Value
 * @brief returns current reading for desired channel */
short ADCFilter_RawReading(short pin) {
    return DataBuffer.data[pin][DataBuffer.head];
}

/**
 * @Function ADCFilter_FilteredReading(short pin)
 * @param pin, which channel to return
 * @return Filtered AD Value
 * @brief returns filtered signal using weights loaded for that channel */
short ADCFilter_FilteredReading(short pin) {
    // ID_ADC_FILTER_VALUES	0XCC 41 94 0049FFA8FFA60031FFB601260239FCEEFCF30180FDFE076B0E18EB16E57D1DAB1DABE57DEB160E18076BFDFE0180FCF3FCEE02390126FFB60031FFA6FFA80049 B9 4E 0D0A
    //short rawReading = ADCFilter_RawReading(pin);
    return ADCFilter_ApplyFilter(FilteredBuffer.data[pin], DataBuffer.data[pin], DataBuffer.head);
}

/**
 * @Function short ADCFilter_ApplyFilter(short filter[], short values[], short startIndex)
 * @param filter, pointer to filter weights
 * @param values, pointer to circular buffer of values
 * @param startIndex, location of first sample so filter can be applied correctly
 * @return Filtered and Scaled Value
 * @brief returns final signal given the input arguments
 * @warning returns a short but internally calculated value should be an int */
short ADCFilter_ApplyFilter(short filter[], short values[], short startIndex) {
    int i;
    int sum = 0;
    for (i = 0; i < FILTERLENGTH; i++) {
        sum += filter[i] * values[startIndex];
        startIndex = (startIndex - 1); // might need to decrement instead of increment
        if (startIndex < 0) {
            startIndex = 31;
        }
    }
    short result = (sum >> 15) & 0xFFFF;
    return result;
}

/**
 * @Function ADCFilter_SetWeights(short pin, short weights[])
 * @param pin, which channel to return
 * @param pin, array of shorts to load into the filter for the channel
 * @return SUCCESS or ERROR
 * @brief loads new filter weights for selected channel */
int ADCFilter_SetWeights(short pin, short weights[]) {
    for (int i = 0; i < FILTERLENGTH; i++) {
        FilteredBuffer.data[pin][i] = weights[i];
    }
    return SUCCESS;
}

void __ISR(_ADC_VECTOR) ADCIntHandler(void) {
    IFS1bits.AD1IF = 0; // clear interrupt flag

    DataBuffer.data[0][DataBuffer.head] = ADC1BUF0; // store data into data buffer
    DataBuffer.data[1][DataBuffer.head] = ADC1BUF1;
    DataBuffer.data[2][DataBuffer.head] = ADC1BUF2;
    DataBuffer.data[3][DataBuffer.head] = ADC1BUF3;
    DataBuffer.head = (DataBuffer.head + 1) % 32; // increment the head of the data buffer
}

#ifdef ADCFILTER_TEST

int main(void) {
    BOARD_Init();
    Protocol_Init();
    ADCFilter_Init();
    FreeRunningTimer_Init();
    FrequencyGenerator_Init();
    LEDS_INIT();

    char alive_message[MAXPAYLOADLENGTH];
    sprintf(alive_message, "ADC filter message time at %s %s.     BTW, Jake is so cool!", __TIME__, __DATE__);
    Protocol_SendDebugMessage(alive_message);

    struct {
        short rawReading;
        short filterReading;
        //  short ReturnBuffer[4];
    } ADCReading;

    unsigned int currentMilli;
    unsigned int previousMilli;
    currentMilli = FreeRunningTimer_GetMilliSeconds();
    previousMilli = currentMilli;

    short LowPass[32] = {-54, -64, -82, -97, -93, -47, 66, 266, 562, 951, 1412, 1909, 2396, 2821, 3136, 3304, 3304, 3136, 2821, 2396, 1909, 1412, 951, 562, 266, 66, -47, -93, -97, -82, -64, -54};
    short HighPass[32] = {0, 0, 39, -91, 139, -129, 0, 271, -609, 832, -696, 0, 1297, -3011, 4755, -6059, 6542, -6059, 4755, -3011, 1297, 0, -696, 832, -609, 271, 0, -129, 139, -91, 39, 0};
    short BandPass[32] = {73, -88, -90, 49, -74, 294, 569, -786, -781, 384, -514, 1899, 3608, -5354, -6787, 7595, 7595, -6787, -5354, 3608, 1899, -514, 384, -781, -786, 569, 294, -74, 49, -90, -88, 73};

    int j;
    int index;
    int on = 0;
    short currentChannel = 0;
    short filterValues[32];
    short newFilterVal[FILTERLENGTH];
    short ReturnBuffer[2];

    while (1) {
        if (Protocol_IsMessageAvailable() == TRUE) {
            unsigned char thisID = Protocol_ReadNextID();
            if (thisID == ID_ADC_FILTER_VALUES) {
                j = 0;
                //                for (index = 0; index < FILTERLENGTH; index++) {
                //                    newFilterVal[index] = ((Lab3PL[j] << 8) | (Lab3PL[j + 1]));
                //                    j = j + 2;
                //                }
                //                                for (int i = 0; i < 32; i++) {
                //                                    newFilterVal[i] = Protocol_ShortEndednessConversion(newFilterVal[i]);
                //                                }

                // Protocol_SendMessage(sizeof (newFilterVal) + 1, ID_ADC_FILTER_VALUES_RESP, &newFilterVal);
                //  Protocol_SendMessage(sizeof(weightsPL), ID_ADC_FILTER_VALUES_RESP, &weightsPL);
                //                for (j = 0; j < FILTERLENGTH; j++) {
                //                    //  weightsPL[j] = Protocol_ShortEndednessConversion(weightsPL[j]);
                //                    newFilterVal[j] = Protocol_ShortEndednessConversion(newFilterVal[j]);
                //                }
                if (Lab3PL[0] == -1) {
                    ADCFilter_SetWeights(currentChannel, LowPass);
                    for (int i = 0; i < 32; i++) {
                        LowPass[i] = Protocol_ShortEndednessConversion(LowPass[i]);
                    }
                    Protocol_SendMessage(sizeof (LowPass) + 1, ID_ADC_FILTER_VALUES_RESP, &LowPass);

                } else if (Lab3PL[1] == 0) {
                    ADCFilter_SetWeights(currentChannel, HighPass);
                    for (int i = 0; i < 32; i++) {
                        HighPass[i] = Protocol_ShortEndednessConversion(HighPass[i]);
                    }
                    Protocol_SendMessage(sizeof (HighPass) + 1, ID_ADC_FILTER_VALUES_RESP, &HighPass);

                } else if (Lab3PL[1] == 73) {
                    ADCFilter_SetWeights(currentChannel, BandPass);
                    for (int i = 0; i < 32; i++) {
                        BandPass[i] = Protocol_ShortEndednessConversion(BandPass[i]);
                    }
                    Protocol_SendMessage(sizeof (BandPass) + 1, ID_ADC_FILTER_VALUES_RESP, &BandPass);
                }

            } else if (thisID == ID_ADC_SELECT_CHANNEL) {
                currentChannel = Lab3PL[0];
                Protocol_SendMessage(2, ID_ADC_SELECT_CHANNEL_RESP, &Lab3PL);

            } else if (thisID == ID_LAB3_FREQUENCY_ONOFF) {
                if (Lab3PL[0] == 1) {
                    FrequencyGenerator_On();
                    LEDS_SET(0b11011);
                } else {
                    FrequencyGenerator_Off();
                    LEDS_SET(0b01);
                }
                Protocol_SendMessage(2, ID_ADC_FILTER_VALUES_RESP, &Lab3PL);
            }
        }
        currentMilli = FreeRunningTimer_GetMilliSeconds();

        if ((currentMilli - previousMilli) >= 10) {
            //ReturnBuffer[0] = ADCFilter_RawReading(currentChannel);
            ReturnBuffer[0] = Protocol_ShortEndednessConversion(ADCFilter_RawReading(currentChannel));
            //ReturnBuffer[1] = ADCFilter_FilteredReading(currentChannel);
            ReturnBuffer[1] = Protocol_ShortEndednessConversion(ADCFilter_FilteredReading(currentChannel));
            Protocol_SendMessage(5, ID_ADC_READING, &ReturnBuffer);

            previousMilli = currentMilli;
        }
    }
    return 0;
    while (1);
    BOARD_End();
}
#endif

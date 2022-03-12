#include <proc/PIC32MX/p32mx340f512h.h>
#include <sys/attribs.h>
#include <stdio.h>
#include "ADCFilter.h"
#include "MessageIDs.h"
#include "Protocol.h"
#include "BOARD.h"
#include "FreeRunningTimer.h"

#define NUMBEROFCHANNELS 4

extern char Lab3PL[MAXPAYLOADLENGTH];
extern short weightsPL[MAXPAYLOADLENGTH];

static signed short FilterBuffer[NUMBEROFCHANNELS][FILTERLENGTH];

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
    TRISBbits.TRISB2 = 1; // A0, AN2, RB2
    TRISBbits.TRISB4 = 1; // A1, AN4, RB4
    TRISBbits.TRISB8 = 1; // A2, AN8, RB8
    TRISBbits.TRISB10 = 1; // A3, AN10, RB10
    AD1PCFG = 0xFAEB;

    // STEP 2: connect same pins to scanner (.CSSL)
    AD1CSSL = 0x0541; // this selects AN2, AN4, AN8, and AN10 for input scan 

    // STEP 3:
    AD1CON1bits.ASAM = 1; // AUTO-SAMPLE. Sampling begins immediately after last conversion completes; SAMP bit is automatically set
    AD1CON1bits.SSRC = 0b111; // AUTO-CONVERT
    AD1CON1bits.FORM = 0b000; // Integer 16-bit

    // STEP 4: 
    AD1CON2bits.VCFG = 0b000; // internal voltage as reference (AVdd & AVss)
    AD1CON2bits.CSCNA = 1; // scan inputs. SCAN MODE
    AD1CON2bits.SMPI = 0b0000; // Interrupts at the completion of conversion for each sample/convert sequence

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
    short result = sum >> 15;
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
    DataBuffer.head = (DataBuffer.head + 1) % 32; // increment the head of the data buffer
    DataBuffer.data[0][DataBuffer.head] = ADC1BUF0; // store data into data buffer
    DataBuffer.data[1][DataBuffer.head] = ADC1BUF1;
    DataBuffer.data[2][DataBuffer.head] = ADC1BUF2;
    DataBuffer.data[3][DataBuffer.head] = ADC1BUF3;
}

#ifdef ADCFILTER_TEST

int main(void) {
    BOARD_Init();
    Protocol_Init();
    ADCFilter_Init();
    FreeRunningTimer_Init();
    LEDS_INIT();

    char alive_message[MAXPAYLOADLENGTH];
    sprintf(alive_message, "ADC filter message time at %s %s.     BTW, Jake is so cool!", __TIME__, __DATE__);
    Protocol_SendDebugMessage(alive_message);

    struct READING {
        short rawReading;
        short filterReading;
        unsigned char ReturnBuffer[32];
    } ADCReading;

    unsigned int currentMilli;
    unsigned int previousMilli;
    currentMilli = FreeRunningTimer_GetMilliSeconds();
    previousMilli = currentMilli;

    int j;
    short currentChannel = 0;
    signed short filterValues[32];
    short newFilterVal[FILTERLENGTH];

    while (1) {
        if (Protocol_IsMessageAvailable() == TRUE) {
            unsigned char thisID = Protocol_ReadNextID();
            if (thisID == ID_ADC_FILTER_VALUES) {
                Protocol_SendMessage(65, ID_ADC_FILTER_VALUES_RESP, &Lab3PL);
                for (j = 0; j < 32; j++) {
                    //weightsPL[j] = Protocol_ShortEndednessConversion(weightsPL[j]);
                     filterValues[j] = Protocol_ShortEndednessConversion(Lab3PL[j]);
                }
                Protocol_SendMessage(65, ID_ADC_FILTER_VALUES_RESP, &filterValues);
                //  newFilterVal[0] = 
                // Protocol_SendMessage(65, ID_DEBUG, &filterValues);
                //ADCFilter_SetWeights(currentChannel, weightsPL);
              //   ADCFilter_SetWeights(currentChannel, Lab3PL);
                //  Protocol_SendMessage(65, ID_DEBUG, &filterValues);
               // Protocol_SendMessage(32, ID_ADC_FILTER_VALUES_RESP, &weightsPL);
            } else if (thisID == ID_ADC_SELECT_CHANNEL) {
                currentChannel = Lab3PL[0];
                Protocol_SendMessage(2, ID_ADC_SELECT_CHANNEL_RESP, &Lab3PL);
            }
        }
        currentMilli = FreeRunningTimer_GetMilliSeconds();

        if ((currentMilli - previousMilli) >= 10) {
            ADCReading.rawReading = Protocol_ShortEndednessConversion(ADCFilter_RawReading(currentChannel));
            ADCReading.filterReading = Protocol_ShortEndednessConversion(ADCFilter_FilteredReading(currentChannel));
            ADCReading.ReturnBuffer[0] = (ADCReading.rawReading >> 8) & 0xFF;
            ADCReading.ReturnBuffer[1] = (ADCReading.rawReading >> 0) & 0xFF;
            ADCReading.ReturnBuffer[2] = (ADCReading.filterReading >> 8) & 0xFF;
            ADCReading.ReturnBuffer[3] = (ADCReading.filterReading >> 0) & 0xFF;
           // Protocol_SendMessage(5, ID_ADC_READING, &(ADCReading.ReturnBuffer));
            previousMilli = currentMilli;

        }

    }
    // 0XCC 41 94 FFCA FFC0 FFAE FF9F FFA3 FFD1 0042 010A 0232 03B7 0584 0775 095C 0B05 0C40 0CE8 0CE8 0C40 0B05 095C 0775 0584 03B7 0232 010A 0042 FFD1 FFA3 FF9F FFAE FFC0 FFCA B9 90 0D0A
    // 0X         FFCAFFC0FFAEFF9FFFA3FFD10042010A023203B705840775095C0B050C400CE80CE80C400B05095C0775058403B70232010A0042FFD1FFA3FF9FFFAEFFC0FFCA

    return 0;
    while (1);
    BOARD_End();
}
#endif

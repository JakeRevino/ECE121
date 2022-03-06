#include <proc/PIC32MX/p32mx340f512h.h>
#include <sys/attribs.h>
#include <stdio.h>
#include "ADCFilter.h"
#include "MessageIDs.h"
#include "Protocol.h"
#include "BOARD.h"

#define NUMBEROFCHANNELS 4

static signed short FilterBuffer[NUMBEROFCHANNELS][FILTERLENGTH];

static struct {
    signed short tail;
    signed short data[NUMBEROFCHANNELS][FILTERLENGTH];
} DataBuffer;


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
            DataBuffer.tail = 0;
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
    short currentReading;
    if (DataBuffer.tail != 0) {
    currentReading = DataBuffer.data[pin][DataBuffer.tail - 1];
    } else {
        currentReading = DataBuffer.data[pin][FILTERLENGTH - 1];
    }
    return currentReading;
}

/**
 * @Function ADCFilter_FilteredReading(short pin)
 * @param pin, which channel to return
 * @return Filtered AD Value
 * @brief returns filtered signal using weights loaded for that channel */
short ADCFilter_FilteredReading(short pin) {
    
}

/**
 * @Function short ADCFilter_ApplyFilter(short filter[], short values[], short startIndex)
 * @param filter, pointer to filter weights
 * @param values, pointer to circular buffer of values
 * @param startIndex, location of first sample so filter can be applied correctly
 * @return Filtered and Scaled Value
 * @brief returns final signal given the input arguments
 * @warning returns a short but internally calculated value should be an int */
short ADCFilter_ApplyFilter(short filter[], short values[], short startIndex);

/**
 * @Function ADCFilter_SetWeights(short pin, short weights[])
 * @param pin, which channel to return
 * @param pin, array of shorts to load into the filter for the channel
 * @return SUCCESS or ERROR
 * @brief loads new filter weights for selected channel */
int ADCFilter_SetWeights(short pin, short weights[]) {
    for (int i = 0; i < FILTERLENGTH; i++) {
        DataBuffer.data[pin][i] = weights[i];
    }
    return SUCCESS;
}

void __ISR(_ADC_VECTOR) ADCIntHandler(void) {
    IFS1bits.AD1IF = 0; // clear interrupt flag
    // copy data in ADC buffer (for each channel) into 2D array
    //You will need to index into the array such
    //that you keep only the last FILTERLENGTH samples for each channel.
}

#ifdef ADCFILTER_TEST

int main(void) {


    return 0;
}
#endif
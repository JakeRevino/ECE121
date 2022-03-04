

#ifndef ROTARYENCODER_H
#define	ROTARYENCODER_H

/*******************************************************************************
 * PUBLIC #DEFINES                                                            *
 ******************************************************************************/

#define ENCODER_BLOCKING_MODE 0
#define ENCODER_INTERRUPT_MODE 1

#define NOP 0x0000
#define ERRFL 0x0001 // error register
#define PROG 0x0003  // Programming register
#define DIAAGC 0x3FFC // Diagnostic and AGC
#define MAG 0x3FFD // CORDIC magnitude
#define ANGLEUNC 0x3FFE // Measured angle w/o dynamic error compensation
#define ANGLECOM 0x3FFF // Measured angle w/ dynamic error compensation

#define _350ns 14 // 14 ticks = 350ns




/*******************************************************************************
 * PUBLIC FUNCTIONS                                                           *
 ******************************************************************************/

/**
 * @Function RotaryEncoder_Init(char interfaceMode)
 * @param interfaceMode, one of the two #defines determining the interface
 * @return SUCCESS or ERROR
 * @brief initializes hardware in appropriate mode along with the needed interrupts */
int RotaryEncoder_Init(char interfaceMode);

/**
 * @Function int RotaryEncoder_ReadRawAngle(void)
 * @param None
 * @return 14-bit number representing the raw encoder angle (0-16384) */
unsigned short RotaryEncoder_ReadRawAngle(void);


#endif	/* ROTARYENCODER_H */
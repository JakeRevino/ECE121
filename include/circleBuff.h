/* ************************************************************************** */
/** Descriptive File Name

  @Company
    Company Name

  @File Name
    filename.h

  @Summary
    Brief description of the file.

  @Description
    Describe the purpose of this file.
 */
/* ************************************************************************** */
#include <Protocol.h>
#ifndef circleBuff    /* Guard against multiple inclusion */
#define circleBuff


/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Included Files                                                    */
/* ************************************************************************** */
/* ************************************************************************** */



/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Constants                                                         */
/* ************************************************************************** */
/* ************************************************************************** */

/*  A brief description of a section can be given directly below the section
    banner.
 */


/* ************************************************************************** */
/** Descriptive Constant Name

  @Summary
    Brief one-line summary of the constant.
    
  @Description
    Full description, explaining the purpose and usage of the constant.
    <p>
    Additional description in consecutive paragraphs separated by HTML 
    paragraph breaks, as necessary.
    <p>
    Type "JavaDoc" in the "How Do I?" IDE toolbar for more information on tags.
    
  @Remarks
    Any additional remarks
 */


#endif
// *****************************************************************************
// *****************************************************************************
// Section: Data Types
// *****************************************************************************
// *****************************************************************************

/*  A brief description of a section can be given directly below the section
    banner.
 */


// *****************************************************************************

/** Descriptive Data Type Name

  @Summary
    Brief one-line summary of the data type.
    
  @Description
    Full description, explaining the purpose and usage of the data type.
    <p>
    Additional description in consecutive paragraphs separated by HTML 
    paragraph breaks, as necessary.
    <p>
    Type "JavaDoc" in the "How Do I?" IDE toolbar for more information on tags.

  @Remarks
    Any additional remarks
    <p>
    Describe enumeration elements and structure and union members above each 
    element or member.
 */
#define MAX_BUFFER_LENGTH 64 //64

struct CircleBuffer {
    unsigned char data[MAX_BUFFER_LENGTH];
    unsigned char head; // oldest data element
    unsigned char tail; // next available free space
    unsigned char size; // number of elements in buffer
};


// *****************************************************************************
// *****************************************************************************
// Section: Interface Functions
// *****************************************************************************
// *****************************************************************************

void init_buff(struct CircleBuffer *buff);
int check_EmptyBuff(struct CircleBuffer *buff);
int check_FullBuff(struct CircleBuffer *buff);
void enqueue_CB(char input, struct CircleBuffer *buff);
unsigned char dequeue_CB(struct CircleBuffer *buff);



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

#define MAX_BUFFER_LENGTH 64

    struct CircleBuffer {
        unsigned char data[MAX_BUFFER_LENGTH];
        int head; // oldest data element
        int tail; // next available free space
        int size; // number of elements in buffer
        
        unsigned char payloads[MAX_BUFFER_LENGTH][MAXPAYLOADLENGTH]; 
        unsigned char payloadLength[MAX_BUFFER_LENGTH];
        int RX_head; //keeps track of the payload circ bufff
        int RX_tail;
        
        
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
    void enqueue_Payload(unsigned char *input, unsigned char len, struct CircleBuffer *buff);
    unsigned char dequeue_Payload(unsigned char *destination, struct CircleBuffer *buff);
    unsigned char returnID(struct CircleBuffer *buff);
    int check_EmptyRX(struct CircleBuffer *buff);

#endif /* _EXAMPLE_FILE_NAME_H */

/* *****************************************************************************
 End of File
 */

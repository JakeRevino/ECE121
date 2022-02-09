#include "xc.h"
#include "Protocol.h"
#include <circleBuff.h>
#include "BOARD.h"


void init_buff(struct CircleBuffer *buff) { // init the buffer
    buff->head = 0; // set head to 0
    buff->tail = 0; // set tail to 0
    buff->size = 0;

}

int check_EmptyBuff(struct CircleBuffer *buff) {
    if (buff->head == buff->tail) { // if head == tail then its empty
        return 1;
    } else
        return 0;
}

int check_FullBuff(struct CircleBuffer *buff) {
    if (buff->head == ((buff->tail + 1) % MAX_BUFFER_LENGTH)) {
        return 1;
    } else
        return 0;
}

void enqueue_CB(char input, struct CircleBuffer *buff) { // write to CB
    if (!check_FullBuff(buff)) { // check if there is space in the circle buffer
        buff->data[buff->tail] = input; // this is writing the data to the tail
        buff->tail = ((buff->tail + 1) % MAX_BUFFER_LENGTH); // this is incrementing 
        buff->size++;
    }
}

unsigned char dequeue_CB(struct CircleBuffer *buff) { // read from CB
    unsigned char temp = 0;
    if (!check_EmptyBuff(buff)) { // check buffer isn't empty, ie. if there is data to dequeue
        temp = buff->data[buff->head];
        buff->head = ((buff->head + 1) % MAX_BUFFER_LENGTH);
        buff->size--;
        return temp;
    }
}
#include "xc.h"
#include "Protocol.h"
#include "circleBuff.h"
#include "BOARD.h"
#include <stdio.h>
#include <string.h>

void init_buff(struct CircleBuffer *buff) { // init the buffer
    buff->head = 0; // set head to 0
    buff->tail = 0; // set tail to 0
    buff->size = 0;
    buff->RX_head = 0;
    buff->RX_tail = 0;

}

int check_EmptyBuff(struct CircleBuffer *buff) {
    if (buff->size <= 0) {
        return 1;
    } else if (buff->head == buff->tail) { // if head == tail then its empty
        return 1;

    } else
        return 0;
}

int check_EmptyRX(struct CircleBuffer *buff) {
    if (buff->RX_head == buff->RX_tail) { // if head == tail then its empty
        return 1;

    } else
        return 0;
}

int check_FullBuff(struct CircleBuffer *buff) {
    // if (buff->head == ((buff->tail + 1) % MAX_BUFFER_LENGTH)) {
    if (buff->size >= MAX_BUFFER_LENGTH - 1) {
        return 1;
    } else {
        return 0;
    }
}

void enqueue_CB(char input, struct CircleBuffer *buff) { // write to CB
    if (check_FullBuff(buff) == 0) { // check if there is space in the circle buffer
        buff->data[buff->tail] = input; // this is writing the data to the tail
        buff->tail = ((buff->tail + 1) % MAX_BUFFER_LENGTH); // this is incrementing 
        buff->size++;
    }
}

unsigned char dequeue_CB(struct CircleBuffer *buff) { // read from CB
    unsigned char temp = 0;
    if (check_EmptyBuff(buff) == 0) { // check buffer isn't empty, ie. if there is data to dequeue
        temp = buff->data[buff->head];
        buff->head = ((buff->head + 1) % MAX_BUFFER_LENGTH);
        buff->size--;
        return temp;
    }
}

void enqueue_Payload(unsigned char *input, unsigned char len, struct CircleBuffer *buff) {
    // want to first check if its empty
    memcpy(buff->payloads[buff->RX_tail], input, len);
    buff->payloadLength[buff->RX_tail] = len;
    buff->RX_tail = ((buff->RX_tail + 1) % MAX_BUFFER_LENGTH);
}

unsigned char dequeue_Payload(unsigned char *destination, struct CircleBuffer *buff) {
    // also want to make sure its not empty or full
    memcpy(destination, &(buff->payloads[buff->RX_head][1]), buff->payloadLength[buff->RX_head]);
    buff->RX_head = ((buff->RX_head + 1) % MAX_BUFFER_LENGTH);
}

unsigned char returnID(struct CircleBuffer *buff) {
    return buff->payloads[buff->RX_head][0];
}
#include <BOARD.h>
#include <xc.h>
#include <Protocol.h>
#include <Protocol.c>
#include <stdio.h>
#include <stdlib.h>

#define UART_TEST
#define BUFFER_TEST

void main(void) {
    
#ifdef UART_TEST 
    BOARD_Init();
    Protocol_Init();
    while (1) {
        U1TXREG = 'J';
    }
    //while(1);
    //BOARD_End();
#endif
    
#ifdef BUFFER_TEST
    unsigned char test_char[] = {'P', 'E', 'N', 'I', 'S'};
    init_buff();
    //struct CircleBuffer somethingElse;
    enqueue_CB(test_char[0]);
    enqueue_CB(test_char[1]);
    //dequeue_CB();
    enqueue_CB(test_char[2]);
    dequeue_CB();
    dequeue_CB();
    //enqueue_CB(test_char[3]);
   // enqueue_CB(test_char[4]);
    //int full_or_nah = checkBuff_isFull();
    
    
    //TX_Buff.buffer
  
    /*write a test for ur buffer*/
    
    
#endif
    //while(1);
   // BOARD_End();
}

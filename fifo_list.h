#ifndef _LIST_HEAD_H
#define _LIST_HEAD_H


void list_fifo_init(void);
int list_fifo_push(char *p_data,int data_len);
char *list_fifo_pop(int *p_data_len);

#endif

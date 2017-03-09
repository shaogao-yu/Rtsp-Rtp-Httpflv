#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include <semaphore.h>
#include <pthread.h>

#include "list.h"
#include "fifo_list.h"

typedef struct
{
    int buff_size;
    char *p_buff;
    struct list_head entry;
}t_list_unit_def;

t_list_unit_def gt_list_fifo_head;
pthread_mutex_t g_list_fifo_mutex;

void list_fifo_init(void)
{
    INIT_LIST_HEAD(&gt_list_fifo_head.entry);
    pthread_mutex_init(&g_list_fifo_mutex,NULL);
}

int list_fifo_push(char *p_data,int data_len)
{
    t_list_unit_def *pt_list_unit;

    pt_list_unit = (t_list_unit_def *)malloc(sizeof(*pt_list_unit));
    ////printf("malloc unit addr:%08x\r\n",pt_list_uni;
    if(pt_list_unit == NULL)
    {
        //printf("malloc unit error\r\n");
        return -1;
    }
    pt_list_unit->p_buff = (char *)malloc(data_len);
    //printf("malloc data addr:%08x\r\n",pt_list_unit->p_buff);
    if(pt_list_unit->p_buff == NULL)
    {
        //printf("malloc data error\r\n");
        return -2;
    }
    pt_list_unit->buff_size = data_len;

    //printf("malloc cpy:%08x,%08x,%d\r\n",pt_list_unit->p_buff,p_data,data_len);
    memcpy(pt_list_unit->p_buff,p_data,data_len);

    pthread_mutex_lock(&g_list_fifo_mutex);

    list_add_tail(&pt_list_unit->entry,&gt_list_fifo_head.entry);
    pthread_mutex_unlock(&g_list_fifo_mutex);

    return 0;
}

char *list_fifo_pop(int *p_data_len)
{
    t_list_unit_def *pt_list_unit;
    char *p_data;
    int data_len;

    pthread_mutex_lock(&g_list_fifo_mutex);

    if(list_empty(&gt_list_fifo_head.entry))
    {
        data_len = 0;
        p_data = NULL;
        //printf("list_empty\r\n");
    }
    else
    {
        pt_list_unit = list_entry(gt_list_fifo_head.entry.next,t_list_unit_def,entry);

        data_len = pt_list_unit->buff_size;
        p_data = pt_list_unit->p_buff;

        list_del(&pt_list_unit->entry);
        free(pt_list_unit);
        //printf("free unit addr:%08x\r\n",pt_list_unit);
    }

    pthread_mutex_unlock(&g_list_fifo_mutex);

    *p_data_len = data_len;
    return p_data;
}



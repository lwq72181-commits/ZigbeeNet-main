#include <stdio.h>
#include <pthread.h>
#include <sys/time.h>
#include <math.h>
#include "ring_buffer.h"

//data_buf wav_rcv_buffer (700);
//data_buf wav_rcv_buffer1 (700);
//data_buf wav_rcv_buffer2 (700);
//data_buf wav_rcv_buffer3 (700);
//data_buf wav_rcv_buffer4 (700);
//data_buf wav_rcv_buffer5 (700);
//data_buf wav_rcv_buffer6 (700);
//data_buf wav_rcv_buffer7 (700);
//data_buf wav_rcv_buffer8 (700);
//data_buf wav_rcv_buffer9 (700);
//data_buf wav_rcv_buffer10 (700);
//data_buf wav_rcv_buffer11 (700);
//data_buf wav_rcv_buffer12 (700);
//data_buf wav_rcv_buffer13 (700);
//data_buf wav_rcv_buffer14 (700);
//data_buf wav_rcv_buffer15 (700);
//data_buf wav_rcv_buffer16 (700);
//data_buf wav_rcv_buffer17 (700);
//data_buf wav_rcv_buffer18 (700);
//data_buf wav_rcv_buffer19 (700);
//data_buf wav_rcv_buffer20 (700);
//data_buf wav_rcv_buffer21 (700);
//data_buf wav_rcv_buffer22 (700);
//data_buf wav_rcv_buffer23 (700);

void data_buf::print_status()
{
    pthread_mutex_lock (&mutex);
    printf ("buffer cnt = %d %d %d\n", (buf_w_index + BUFFER_POOL_CNT - buf_r_index) % BUFFER_POOL_CNT,
        (write_index + BUFFER_POOL_CNT - read_index) % BUFFER_POOL_CNT,
        (buf_w_index + BUFFER_POOL_CNT - buf_r_index) % BUFFER_POOL_CNT
        + (write_index + BUFFER_POOL_CNT - read_index) % BUFFER_POOL_CNT);
    pthread_mutex_unlock (&mutex);
    return;
}

data_buf::s_data *data_buf::get_buffer (void)
{
    s_data *ret_buff = NULL;
    pthread_mutex_lock (&mutex_buffer);

    if (buf_r_index != buf_w_index) //not empty
    {
        ret_buff = p_buf[buf_r_index++];
        buf_r_index %= BUFFER_POOL_CNT;
    }

    pthread_mutex_unlock (&mutex_buffer);

    return ret_buff;
}


void data_buf::put_buffer (s_data *pbuf)
{
    pthread_mutex_lock (&mutex_buffer);

    if ((buf_w_index + 1) % BUFFER_POOL_CNT == buf_r_index) // full
    {
        printf ("error! buffer empty full\n");
    }
    else
    {
        p_buf[buf_w_index++] = pbuf;
        buf_w_index %= BUFFER_POOL_CNT;
    }

    pthread_mutex_unlock (&mutex_buffer);
    return;
}


data_buf::s_data *data_buf::get_data ()
{
    data_buf::s_data *p_s_data = NULL;

    pthread_mutex_lock (&mutex);
    if (read_index != write_index) //not empty
    {
        p_s_data = data[read_index++];
        read_index %= BUFFER_POOL_CNT;
        date_cnt--;
    }
    else
    {
     //   printf("aduio data buffer empty get_data!!!!\n");
    }
    pthread_mutex_unlock (&mutex);

    return p_s_data;
}

void data_buf::put_data (data_buf::s_data *pdata, struct timeval *stamp)
{
    pthread_mutex_lock (&mutex);

    if ((write_index + 1) % BUFFER_POOL_CNT == read_index) //full
    {
        printf ("error! data buffer full\n");
    }
    else
    {
        data[write_index] = pdata;

        if (NULL == stamp)
        {
            gettimeofday (&data[write_index]->time_stamp, NULL);
        }
        else
        {
            data[write_index]->time_stamp = *stamp;
        }

        write_index++;
        write_index %= BUFFER_POOL_CNT;
        date_cnt++;
    }
    pthread_mutex_unlock (&mutex);
    return;
}

data_buf::s_data * data_buf::get_last_data()
{
    data_buf::s_data *p_s_data = NULL;
    pthread_mutex_lock (&mutex);
    if (read_index != write_index)
    {
        int index = write_index-1;
        if(index<0)
        index = BUFFER_POOL_CNT-1;
        p_s_data =  data[index];
    }
    pthread_mutex_unlock (&mutex);
    return p_s_data;
}

void data_buf::mix_buf_data(s_data *first,s_data *second)
{
//    int i;
//    static double f=1;
//    int mid = 0;
//    int const MAX=32767;
//    int const MIN=-32768;
    pthread_mutex_lock (&mutex);
//    if(first==NULL || second==NULL)
//    {
//        if(first==NULL)
//            puts("1");
//        else
//            puts("2s");
//        printf("mix_buf_data parm is NULL!!!\n");
//        return;
//    }
//    int len = (first->data_len-10)>>1;
//    //printf("len:%d\n",len);
//    short *p1= (short *)(((t_data_pkg *)first->pdata)->data);
//    short *p2= (short *)(((t_data_pkg *)second->pdata)->data);
//    //short *p1= (short *)(((t_data_pkg *)first)->data);
//    //short *p2= (short *)(((t_data_pkg *)second)->data);
//    for(i=0;i<len;i++)
//    {
//        #if 0
//        ////归一化混音
//        mid = (p1[i]+p2[i])*f;
//        if (mid>MAX)
//        {
//            f=(double)MAX/(double)(mid);
//            mid=MAX;
//        }
//        if (mid<MIN)
//        {
//            f=(double)MIN/(double)(mid);
//            mid=MIN;
//        }
//        if (f<1)
//        {
//            f+=((double)1-f)/(double)32;
//        }
//        p1[i]=(short)mid;
//        #endif
        
//        #if 1
//        //newlc
//        if( p1[i] < 0 && p2[i] < 0)
//        mid = p1[i] +p2[i] - (p1[i]  * p2[i] / -(pow(2,16-1)-1));
//        else
//        mid = p1[i] +p2[i] - (p1[i]  * p2[i] / (pow(2,16-1)-1));
//        p1[i]=(short)mid;
//        #endif
//    }
    pthread_mutex_unlock (&mutex);
}




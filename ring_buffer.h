#ifndef _RING_BUFFER_H
#define _RING_BUFFER_H
#include<stdlib.h>
#include<QDebug>
//#include "typedef.h"
#define BUFFER_POOL_CNT    512//16//32//64//512//64
class data_buf {
public:
    typedef struct {
        int data_len;
        int cnt;
        struct timeval time_stamp; // 数据创建时间
        int *pdata;
    } s_data;
    int read_index;
    int write_index;
    int buf_r_index;
    int buf_w_index;
    unsigned int date_cnt;
private:
    pthread_mutex_t mutex;
    pthread_mutex_t mutex_buffer;

	float rat;
	
    s_data *data[BUFFER_POOL_CNT];
    s_data *p_buf[BUFFER_POOL_CNT];

public:
    data_buf (int buf_size):read_index(0), write_index(0), buf_r_index(0), buf_w_index(BUFFER_POOL_CNT-1),date_cnt(0),rat(0.5) {//index 相等表示空
        int i;
        pthread_mutex_init (&mutex, NULL);
        pthread_mutex_init (&mutex_buffer, NULL);

        for (i = 0; i < BUFFER_POOL_CNT -1; i++)
        {
            //qDebug("init buff size :%d\n",buf_size);
            p_buf[i] = new s_data;
            if (NULL == p_buf[i])
            {
                qDebug ("new s_data error\n");
                exit (0);
            }
            else
            {
                p_buf[i]->pdata = new int[buf_size];
                if (NULL == p_buf[i]->pdata)
                {
                    qDebug ("new buffer error\n");
                    exit (0);
                }
            }
        }
    }

    ~data_buf() {
        int i = 0;
        qDebug ("need to delete buffer\n");
        for (i = 0; i < BUFFER_POOL_CNT-1; i++)
        {
            delete [] p_buf[i]->pdata;
            delete p_buf[i];
        }
    }

    s_data *get_buffer (void);
    void put_buffer (data_buf::s_data *pbuf);

    s_data *get_data (void);
    void put_data (data_buf::s_data *pdata, struct timeval *stamp = NULL);
    s_data * get_last_data();
    void mix_buf_data(s_data *first,s_data *second);
    void print_status();
	int set_fifo_level(float rate){
		rat = rate;
		return 0;
	};
	
	bool data_is_ready(){
		bool ret =false;
		int cnt;
		pthread_mutex_lock (&mutex);
		cnt = (write_index + BUFFER_POOL_CNT - read_index) % BUFFER_POOL_CNT;
		if (cnt > rat * BUFFER_POOL_CNT) //not empty
		{
			ret = true;
		}
		pthread_mutex_unlock (&mutex);
		return ret;
	};
};



#endif

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <fcntl.h>
#include <time.h>
#include <sys/ioctl.h>
#include <linux/ioctl.h>
#include <sys/stat.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/input.h>
#include <errno.h>
#include <string.h>
#include <getopt.h>
#include <stdbool.h>
#include <libgen.h>
#include <sys/select.h>
#include <signal.h>
#include <sched.h>

#include "zigbeedatahandle.h"


ZigBeeDataHandle::ZigBeeDataHandle()
{
    m_rbuffer = NULL;
    initBuffer();
    initParam();
}


int ZigBeeDataHandle::initBuffer()
{
    int i;
    qDebug("start init ZigBee buffer!!!\n");
    for(i=0;i<24;i++)
    {
        m_bufVector[i] = new data_buf(512);
        //m_bufVector.append(*m_data[i]);
    }
//    m_bufVector.append(data_buf(700));
    qDebug("init ZigBee buffer over!!!\n");
    return 0;
}

int ZigBeeDataHandle::initParam()
{
    return 0;
}

int ZigBeeDataHandle::unpackADCvalue()
{
    qDebug("unpackADCvalue\n");
    for(int i=0;i<24;i++)
    {
        data_buf::s_data *p_buf = m_bufVector[i]->get_buffer();
        if(p_buf==NULL)
        {
            qDebug("buff is full:%d\n",i);
            continue;
        }
        memcpy(p_buf->pdata, &m_rbuffer[i*512], 2048);
        p_buf->data_len = 2048;
        m_bufVector[i]->put_data(p_buf);
    }
    qDebug("unpackADCvalue over\n");
    return 0;
}

int ZigBeeDataHandle::setSche()
{
//    pid_t pid = getpid();
//    int policy;
//    struct sched_param param;

//    // 获取当前的调度策略和参数
//    if (sched_getparam(pid, &param) == -1) {
//        perror("sched_getparam");
//        return -1;
//    }
//    if (sched_getscheduler(pid) == -1) {
//        perror("sched_getscheduler");
//        return -1;
//    }

//    // 设置新的调度策略和优先级
//    policy = SCHED_RR; // 可选的策略有：SCHED_OTHER, SCHED_RR, SCHED_FIFO
//    param.sched_priority = 99; // 范围通常是1到99，数值越高，优先级越高

//    // 应用新的调度策略和优先级
//    if (sched_setscheduler(pid, policy, &param) == -1) {
//        perror("sched_setscheduler");
//        return -1;
//    }

//    // 再次获取并打印调度信息以确认设置
//    if (sched_getparam(pid, &param) == -1) {
//        perror("sched_getparam");
//        return -1;
//    }
//    printf("Scheduling policy: %s\n", (policy == SCHED_RR) ? "SCHED_RR" :
//                                     (policy == SCHED_FIFO) ? "SCHED_FIFO" :
//                                     "SCHED_OTHER");
//    printf("Scheduling priority: %d\n", param.sched_priority);
    return 0;
}


void ZigBeeDataHandle::dowork()
{
//    unsigned int dev_addr = 0;
//    int status = 0;
//    int ret = -1, i = 0;
//    struct input_event input_buf;
//    struct timeval start, end, time_diff;
//    fd_set input;
//    struct timeval timeout;
    int buffer[16*1024] ={0};
//    while(1)
//    {
//        timeout.tv_sec = 1;
//        timeout.tv_usec = 0;
//        FD_ZERO(&input);
//        FD_SET(m_DMAInfo->input_fd, &input);
//        ret = select(m_DMAInfo->input_fd + 1, &input, NULL, NULL, &timeout);
//        if (ret < 0) {
//            status = -1;
//            qDebug("select failed\n");
//            //goto error;
//        } else if (ret == 0) {
//            status = -1;
//            qDebug("select timeout\n");
//            continue;;
//        }
//        ret = read(m_DMAInfo->input_fd, &input_buf, sizeof(struct input_event));
//        if (ret < 0) {
//            status = -1;
//            qDebug("read input_fd error\n");
////            goto error;
//        } else if (input_buf.type != 0) {
//            data_buf::s_data *p_buf = m_rbuffer->get_buffer();
//            if(p_buf==NULL)
//            {
//                neon_copy(buffer, (int *)m_DMAInfo->vir_addr, m_DMAInfo->params.size);
//                continue;
//            }
//            neon_copy((int *)p_buf->pdata, (int *)m_DMAInfo->vir_addr, m_DMAInfo->params.size);
//            p_buf->data_len = m_DMAInfo->params.size;
//            m_rbuffer->put_data(p_buf);
//            //unpackADCvalue();

//        }
//    }
    return;
}

#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <linux/input.h>
#include <errno.h>
#include <string.h>
#include <getopt.h>
#include <stdbool.h>
#include <libgen.h>
#include <sys/select.h>
#include <signal.h>
#include <sched.h>
#include "Dmamemcpy.h"
#include "Acquisitiontype.h"
DmaMemcpy::DmaMemcpy()
{
    m_rbuffer = NULL;
    init_parameter();
    initBuffer();
}

int DmaMemcpy::initBuffer()
{
    int i;
    qDebug("start init buffer!!!\n");
    for(i=0;i<24;i++)
    {
       // qDebug("1!!!\n");
        m_bufVector[i] = new data_buf(512);
        //qDebug("2!!!\n");
        //m_bufVector.append(*m_data[i]);
        //qDebug("3!!!\n");
    }
//    m_bufVector.append(data_buf(700));
    qDebug("init buffer over!!!\n");
    return 0;
}

bool DmaMemcpy::init_parameter()
{
    int ret = 0;
    m_DMAInfo = (DMAInfo*)malloc(sizeof(DMAInfo));
    m_DMAInfo->params.address = 0xf0200000;
    m_DMAInfo->params.size = DMA_BUFFER_SIZE;//65536;//TODO: change accurate size
    m_DMAInfo->dma_fd = -1;
    m_DMAInfo->input_fd = -1;
    strcpy(m_DMAInfo->params.dev, "/dev/input/event6");
    if(m_rbuffer != NULL)
    {
        qDebug("m_rbuffer is not null\n");
        free(m_rbuffer);
    }
    m_rbuffer = new data_buf(m_DMAInfo->params.size);
    //m_rbuffer = (int *)malloc(m_DMAInfo->params.size);
    if (m_rbuffer == NULL) {
        qDebug("malloc w_buffer failed!\n");
        return false;
    }

    ret = dma_init(m_DMAInfo);
    if (ret < 0) {
        qDebug("dma_init failed!!!\n");
        dma_destroy(m_DMAInfo);
        return false;
    }
    else
        return true;
}

int DmaMemcpy::convert24To32Signed(int input) {
    // 首先，判断原始数值的正负
    if (input & 0x800000) { // 如果是负数（24位中的最高位为1）
        // 对于负数，扩展符号位，即在高位补1
        return input | 0xFF000000; // 在input的高8位补上1，其余位补0
    } else {
        // 对于正数或零，直接在高位补0
        return input & 0xFFFFFF; // 保持低24位不变，高8位自然为0
    }
}

void DmaMemcpy::neon_copy(volatile void *dst, volatile void *src, int sz)
{
    /* align for 64 bytes */
    if (sz & 63) {
        sz = (sz & -64) + 64;
    }

    asm volatile (
        "sub %[dst], %[dst], #64 \n"
        "1: \n"
        "ldnp q0, q1, [%[src]] \n"
        "ldnp q2, q3, [%[src], #32] \n"
        "add %[dst], %[dst], #64 \n"
        "subs %[sz], %[sz], #64 \n"
        "add %[src], %[src], #64 \n"
        "stnp q0, q1, [%[dst]] \n"
        "stnp q2, q3, [%[dst], #32] \n"
        "b.gt 1b \n"
        : [dst]"+r"(dst), [src]"+r"(src), [sz]"+r"(sz) : : "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7", "cc", "memory");
}

int DmaMemcpy::dma_init(struct DMAInfo *dma_info)
{
    if (dma_info == NULL) {
        return -1;
    }

    qDebug("Open dma & input device node...\n");
    dma_info->dma_fd = open(DMA_MEMCPY_PATH, O_RDWR);
    if(dma_info->dma_fd < 0) {
        qDebug("can't open %s ret:%d\n", DMA_MEMCPY_PATH,dma_info->dma_fd);
        return -1;
    }

    dma_info->input_fd = open(dma_info->params.dev, O_RDONLY | O_NONBLOCK);
    if(dma_info->input_fd < 0) {
        qDebug("can't open %s ret:%d\n", dma_info->params.dev,dma_info->input_fd);
        return -1;
    }
    qDebug("Success open dma & input device node.\n\n");

    return 0;
}

void DmaMemcpy::dma_destroy(struct DMAInfo *dma_info)
{
//    if (dma_info->map_base_addr != NULL) {
//        munmap(dma_info->map_base_addr, dma_info->params.size);
//        dma_info->map_base_addr = NULL;
//    }

    if (dma_info->dma_fd != -1)
    {
        qDebug("need to close dmafd:%d\n",dma_info->dma_fd);
        close(dma_info->dma_fd);
    }
    if (dma_info->input_fd != -1)
    {
        qDebug("need to close input_fd:%d\n",dma_info->input_fd);
        close(dma_info->input_fd);
    }
}

int DmaMemcpy::unpackADCvalue()
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
}

int DmaMemcpy::setSche()
{
    pid_t pid = getpid();
    int policy;
    struct sched_param param;

    // 获取当前的调度策略和参数
    if (sched_getparam(pid, &param) == -1) {
        perror("sched_getparam");
        return -1;
    }
    if (sched_getscheduler(pid) == -1) {
        perror("sched_getscheduler");
        return -1;
    }

    // 设置新的调度策略和优先级
    policy = SCHED_RR; // 可选的策略有：SCHED_OTHER, SCHED_RR, SCHED_FIFO
    param.sched_priority = 99; // 范围通常是1到99，数值越高，优先级越高

    // 应用新的调度策略和优先级
    if (sched_setscheduler(pid, policy, &param) == -1) {
        perror("sched_setscheduler");
        return -1;
    }

    // 再次获取并打印调度信息以确认设置
    if (sched_getparam(pid, &param) == -1) {
        perror("sched_getparam");
        return -1;
    }
    qDebug("Scheduling policy: %s\n", (policy == SCHED_RR) ? "SCHED_RR" :
                                     (policy == SCHED_FIFO) ? "SCHED_FIFO" :
                                     "SCHED_OTHER");
    qDebug("Scheduling priority: %d\n", param.sched_priority);
    return 0;
}


void DmaMemcpy::dowork()
{
    struct DMASetupParams dma_setup_params;
    unsigned int dev_addr = 0;
    int status = 0;
    int ret = -1, i = 0;
    struct input_event input_buf;
    struct timeval start, end, time_diff;
    fd_set input;
    struct timeval timeout;
    int buffer[16*1024] ={0};
    qDebug("===init dma\n");
    /* Setup DMA */
    dma_setup_params.dev_phy_addr           = m_DMAInfo->params.address;
    dma_setup_params.transfer_total_size    = m_DMAInfo->params.size;
    dma_setup_params.transfer_single_size   = m_DMAInfo->params.size;
    /* 0: write, 1: read */
    dma_setup_params.transfer_direction     = 1;
    setSche();
    dev_addr = ioctl(m_DMAInfo->dma_fd, DMA_MEMCPY_SETUP, &dma_setup_params);
    if (dev_addr == 0) {
        qDebug("can't get device addr: 0x%x\n", dev_addr);
        return;
    }

    /* Start DMA */
    ret = ioctl(m_DMAInfo->dma_fd, DMA_MEMCPY_START, 0);
    if (ret != 0) {
        return;
    }

    m_DMAInfo->vir_addr = mmap(NULL, m_DMAInfo->params.size, PROT_READ | PROT_WRITE, MAP_SHARED, m_DMAInfo->dma_fd, 0);
    if (m_DMAInfo->vir_addr == MAP_FAILED) {
        qDebug("mmap failed\n");
        return;
    }

    while(1)
    {
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        FD_ZERO(&input);
        FD_SET(m_DMAInfo->input_fd, &input);
        ret = select(m_DMAInfo->input_fd + 1, &input, NULL, NULL, &timeout);
        if (ret < 0) {
            status = -1;
            qDebug("select failed\n");
            //goto error;
        } else if (ret == 0) {
            status = -1;
            qDebug("select timeout\n");
            continue;;
        }
        ret = read(m_DMAInfo->input_fd, &input_buf, sizeof(struct input_event));
        if (ret < 0) {
            status = -1;
            qDebug("read input_fd error\n");
//            goto error;
        } else if (input_buf.type != 0) {
            data_buf::s_data *p_buf = m_rbuffer->get_buffer();
            if(p_buf==NULL)
            {
                //qDebug("m_rbuffer is full:%d\n",i);
                neon_copy(buffer, (int *)m_DMAInfo->vir_addr, m_DMAInfo->params.size);
                continue;
            }
            neon_copy((int *)p_buf->pdata, (int *)m_DMAInfo->vir_addr, m_DMAInfo->params.size);
            p_buf->data_len = m_DMAInfo->params.size;
            m_rbuffer->put_data(p_buf);
            //qDebug("unpackADCvalue\n");
            //unpackADCvalue();

        }
    }
error:
    /* Shutdown DMA transfer */
    ret = ioctl(m_DMAInfo->dma_fd, DMA_MEMCPY_SHUTDOWN, 0);
    if (ret < 0) {
        status = -1;
    }
    munmap(m_DMAInfo->vir_addr, m_DMAInfo->params.size);
    qDebug(" DMA thread over!!\n");
    return;
}

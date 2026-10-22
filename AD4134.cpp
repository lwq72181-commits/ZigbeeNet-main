#include "AD4134.h"
#include <linux/spi/spidev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdint.h>
#include <unistd.h>
#include <QThread>
#include "TStringUtil.h"
#include "ad713x.h"
AD4134::AD4134()
{

}

void AD4134::init()
{
    memset(&m_params, 0, sizeof(m_params));
    strcpy(m_params.dev, "/dev/spidev4.0");
    m_params.speed = 150000000;
    m_params.bits = 8;
    //m_params.mode |= SPI_CPHA; //SPI_CPHA = SPI_CPOL = 0
    //m_params.mode |= SPI_CPOL;
    openAD();
    ad713x_init(&m_params);
}

uint8_t AD4134::crc8(const void *data, int len) {
    uint8_t crc = 0xA5; // Initialize the CRC
    const unsigned char *ptr = (const unsigned char *)data;
    while (len--) {
        crc ^= *ptr++; // XOR the current byte with the CRC
        for (int i = 8; i > 0; i--) {
            if (crc & 0x80) { // If the most significant bit is set...
                crc = (crc << 1) ^ POLYNOMIAL_CRC8; // Shift left and XOR with the polynomial
            } else {
                crc <<= 1; // Otherwise, just shift left
            }
            crc &= 0xFF; // Mask to keep only the least significant 8 bits
        }
    }
    return crc; // Return the final CRC value
}

int32_t AD4134::ad713x_spi_reg_read(uint8_t reg_addr,uint8_t *reg_data, struct _Params *params)
{
    int32_t ret = 0;
    uint8_t buf[2];
    uint8_t bufR[2];

    buf[0] = reg_addr | 0x80;
    buf[1] = 0x00;
    params->size = 2;
    ret = transfer(m_fd, buf, bufR, params);
    if(ret < 1)
    {
       qDebug("fspi transfer error");
    }
    else
       reg_data[0] = bufR[1];
    return ret;
}

uint8_t AD4134::ad713x_spi_reg_write(uint8_t reg_addr,uint8_t reg_data, struct _Params *params)
{
    uint8_t buf[2];
    uint8_t bufR[2];
    buf[0] = reg_addr;
    buf[1] = reg_data;
    params->size = 2;
    transfer(m_fd, buf, bufR, params);
    return bufR[0];
}

//int32_t AD4134::ad713x_spi_write_mask(
//                  uint8_t reg_addr,
//                  uint32_t mask,
//                  uint8_t data)
//{
//    uint8_t reg_data;
//    int32_t ret = 0;

//    ret = ad713x_spi_reg_read(reg_addr, &reg_data);
//    if(ret<0)
//        return -1;
//    reg_data &= ~mask;
//    reg_data |= data;

//    return ad713x_spi_reg_write(reg_addr, reg_data);
//}

int32_t AD4134::ad713x_dout_format_config(struct _Params *params,uint8_t count)
{
    int32_t ret = 0;
    uint8_t reg_data;
    ret = ad713x_spi_reg_read(AD713X_REG_DIGITAL_INTERFACE_CONFIG, &reg_data, params);
    if(ret<0)
        return -1;
    reg_data &= 0xf0;
    reg_data |= 0x0d;//daisy-chain TODO:change nu of ad may effect config

    reg_data &= 0x0f;
    reg_data |= (count-1)<<4;//daisy-chain channels count value: count-1   1-16
    ret = ad713x_spi_reg_write(AD713X_REG_DIGITAL_INTERFACE_CONFIG, reg_data, params);
    return ret;
}

int32_t AD4134::ad713x_mag_phase_clk_delay(struct _Params *params,
                   bool clk_delay_en)
{
    int32_t ret = 0;
    uint8_t reg_data;
    if (clk_delay_en)
        reg_data = 0x00;
        //temp_clk_delay = DELAY_2_CLOCKS;
    else
        reg_data = 0xaa;
        //temp_clk_delay = DELAY_NONE;
//    ret = ad713x_spi_reg_read(AD713X_REG_MPC_CONFIG, &reg_data, params);
//    if(ret<0)
//        return -1;
    ret = ad713x_spi_reg_write(AD713X_REG_DIGITAL_INTERFACE_CONFIG, reg_data, params);
//    for (i = CH3; i >= 0; i--) {
//        ret = ad713x_spi_write_mask(dev, AD713X_REG_MPC_CONFIG,
//                        AD713X_MPC_CLKDEL_EN_CH_MSK(i),
//                        AD713X_MPC_CLKDEL_EN_CH_MODE(temp_clk_delay, i));
//        if (NO_OS_IS_ERR_VALUE(ret))
//            return -1;
//    }

    return ret;
}

int32_t AD4134::ad713x_init_chan_bw(struct _Params *params, bool bw_en)
{
    int32_t ret;
    uint8_t reg_data;
    if (bw_en)
        reg_data = 0x00;//0.433 × ODR
        //temp_clk_delay = DELAY_2_CLOCKS;
    else
        reg_data = 0x0f;//0.10825 × ODR

    ret = ad713x_spi_reg_write(AD713X_REG_DIGITAL_INTERFACE_CONFIG, reg_data, params);
    return ret;
}

int32_t AD4134::ad713x_set_out_data_frame(struct _Params *params)
{
    int32_t ret = 0;
    uint8_t reg_data;
    ret = ad713x_spi_reg_read(AD713X_REG_DATA_PACKET_CONFIG, &reg_data, params);
    if(ret<0)
        return -1;
    reg_data &= 0xcf;
    reg_data |= 0x20;
    ret = ad713x_spi_reg_write(AD713X_REG_DATA_PACKET_CONFIG, reg_data, params);
    return ret;
}

int32_t AD4134::ad713x_init(struct _Params *params)
{
    int32_t ret = 0;
    uint8_t data;
    int i = 1;
    for(;i<7;i++)
    {
        //i2cset -f -y -a 0 0x78 0x01 0x00
        std::string cmd = comn::StringUtil::format("i2cset -f -y -a 0 0x78 0x04 %d", i);
        qDebug("%s",cmd.c_str());
        system(cmd.c_str());
        QThread::usleep(500);
        ret = ad713x_spi_reg_read(AD713X_REG_CHIP_TYPE, &data, params);
        if (data != 0x07)
        {
            qDebug("adc chip info error!!!");
        }
        ret = ad713x_spi_reg_read(AD713X_REG_DEVICE_CONFIG, &data, params);
        data |= 0x01; //high performance

        ret = ad713x_spi_reg_write(AD713X_REG_DEVICE_CONFIG, data, params);

        ret = ad713x_set_out_data_frame(params);

        ret = ad713x_dout_format_config(params, 1);

        ret = ad713x_mag_phase_clk_delay(params, true);

        ret = ad713x_init_chan_bw(params, true);
    }
    return ret;
}

int AD4134::transfer(int fd, uint8_t const *tx, uint8_t const *rx, struct _Params *params)
{
    int ret;
    struct spi_ioc_transfer tr = {
        .tx_buf         = (unsigned long)tx,
        .rx_buf         = (unsigned long)rx,
        .len            = params->size,
        .speed_hz       = params->speed,
        .delay_usecs    = params->delay,
        .bits_per_word  = params->bits,
        .cs_change      = 0,               // 显式初始化未使用的字段
        .tx_nbits       = 0,
        .rx_nbits       = 0,
        .pad            = 0,
    };

    if (params->mode & SPI_TX_QUAD)
        tr.tx_nbits = 4;
    else if (params->mode & SPI_TX_DUAL)
        tr.tx_nbits = 2;
    if (params->mode & SPI_RX_QUAD)
        tr.rx_nbits = 4;
    else if (params->mode & SPI_RX_DUAL)
        tr.rx_nbits = 2;

    if (!(params->mode & SPI_LOOP)) {
        if (params->mode & (SPI_TX_QUAD | SPI_TX_DUAL))
            tr.rx_buf = 0;
        else if (params->mode & (SPI_RX_QUAD | SPI_RX_DUAL))
            tr.tx_buf = 0;
    }

    ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
    if (ret < 1) {
        perror("can't send spi message");
    }
    return ret;
}

int AD4134::openAD()
{
    int ret = 0;
    /* Open spi device */
    m_fd = open(m_params.dev, O_RDWR);
    if (m_fd < 0) {
        qDebug("open spi error!!!\n");
        ret = m_fd;
    }

    /* Set spi mode */
    ret = ioctl(m_fd, SPI_IOC_WR_MODE32, &m_params.mode);
    if (ret == -1) {
        qDebug("SPI_IOC_WR_MODE32");
    }

    /* Get spi mode */
    ret = ioctl(m_fd, SPI_IOC_RD_MODE, &m_params.mode);
    if (ret == -1) {
        qDebug("SPI_IOC_RD_MODE");
    }

    /* Set bits per word */
    ret = ioctl(m_fd, SPI_IOC_WR_BITS_PER_WORD, &m_params.bits);
    if (ret == -1) {
        qDebug("SPI_IOC_WR_BITS_PER_WORD");
    }

    /* Get bits per word */
    ret = ioctl(m_fd, SPI_IOC_RD_BITS_PER_WORD, &m_params.bits);
    if (ret == -1) {
        qDebug("SPI_IOC_RD_BITS_PER_WORD");
    }

    /* Set max speed (hz) */
    ret = ioctl(m_fd, SPI_IOC_WR_MAX_SPEED_HZ, &m_params.speed);
    if (ret == -1) {
        qDebug("SPI_IOC_WR_MAX_SPEED_HZ");
    }

    /* Get max speed (hz) */
    ret = ioctl(m_fd, SPI_IOC_RD_MAX_SPEED_HZ, &m_params.speed);
    if (ret == -1) {
        qDebug("SPI_IOC_RD_MAX_SPEED_HZ");
    }
    return ret;
}

void AD4134::closeAD()
{
    if (m_fd != -1)
        close(m_fd);
}

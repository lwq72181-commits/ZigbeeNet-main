#ifndef AD4134_H
#define AD4134_H
#include "Acquisitiontype.h"

class AD4134
{
public:
    AD4134();
    void init();
    int openAD();
    void closeAD();
    int transfer(int fd, uint8_t const *tx, uint8_t const *rx, struct _Params *params);
protected:
    uint8_t ad713x_spi_reg_write(uint8_t reg_addr,uint8_t reg_data, struct _Params *params);
    int32_t ad713x_spi_reg_read(uint8_t reg_addr,uint8_t *reg_data, struct _Params *params);
    int32_t ad713x_set_out_data_frame(struct _Params *params);
    int32_t ad713x_dout_format_config(struct _Params *params,uint8_t count);
    int32_t ad713x_init_chan_bw(struct _Params *params, bool bw_en);
    int32_t ad713x_mag_phase_clk_delay(struct _Params *params,bool clk_delay_en);
    int32_t ad713x_init(struct _Params *params);
    uint8_t crc8(const void *data, int len);
protected:
    struct _Params m_params;
    int m_fd;
};

#endif // AD4134_H

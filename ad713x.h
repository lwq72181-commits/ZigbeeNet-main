#ifndef SRC_AD713X_H_
#define SRC_AD713X_H_


#include <stdbool.h>
#include <stdio.h>

#define AD713X_REG_INTERFACE_CONFIG_A		0x00
#define AD713X_REG_INTERFACE_CONFIG_B		0x01
#define AD713X_REG_DEVICE_CONFIG		0x02
#define AD713X_REG_CHIP_TYPE			0x03
#define AD713X_REG_PRODUCT_ID_LSB		0x04
#define AD713X_REG_PRODUCT_ID_MSB		0x05
#define AD713X_REG_CHIP_GRADE			0x06
#define AD713X_REG_CHIP_INDEX			0x07
#define AD713X_REG_SCTATCH_PAD			0x0A
#define AD713X_REG_SPI_REVISION			0x0B
#define AD713X_REG_VENDOR_ID_LSB		0x0C
#define AD713X_REG_VENDOR_ID_MSB		0x0D
#define AD713X_REG_STREAM_MODE			0x0E
#define AD713X_REG_TRANSFER_REGISTER		0x0F
#define AD713X_REG_DEVICE_CONFIG1		0x10
#define AD713X_REG_DATA_PACKET_CONFIG		0x11
#define AD713X_REG_DIGITAL_INTERFACE_CONFIG	0x12
#define AD713X_REG_POWER_DOWN_CONTROL		0x13
#define AD713X_REG_AIN_RANGE_SELECT		0x14
#define AD713X_REG_DEVICE_STATUS		0x15
#define AD713X_REG_ODR_VAL_INT_LSB		0x16
#define AD713X_REG_ODR_VAL_INT_MID		0x17
#define AD713X_REG_ODR_VAL_INT_MSB		0x18
#define AD713X_REG_ODR_VAL_FLT_LSB		0x19
#define AD713X_REG_ODR_VAL_FLT_MID0		0x1A
#define AD713X_REG_ODR_VAL_FLT_MID1		0x1B
#define AD713X_REG_ODR_VAL_FLT_MSB		0x1C
#define AD713X_REG_CHANNEL_ODR_SELECT		0x1D
#define AD713X_REG_CHAN_DIG_FILTER_SEL		0x1E
#define AD713X_REG_FIR_BW_SEL			0x1F
#define AD713X_REG_GPIO_DIR_CTRL		0x20
#define AD713X_REG_GPIO_DATA			0x21
#define AD713X_REG_ERROR_PIN_SRC_CONTROL	0x22
#define AD713X_REG_ERROR_PIN_CONTROL		0x23
#define AD713X_REG_VCMBUF_CTRL			0x24
#define AD713X_REG_DIAGNOSTIC_CONTROL		0x25
#define AD713X_REG_MPC_CONFIG			0x26
#define AD713X_REG_CH0_GAIN_LSB			0x27
#define AD713X_REG_CH0_GAIN_MID			0x28
#define AD713X_REG_CH0_GAIN_MSB			0x29
#define AD713X_REG_CH0_OFFSET_LSB		0x2A
#define AD713X_REG_CH0_OFFSET_MID		0x2B
#define AD713X_REG_CH0_OFFSET_MSB		0x2C
#define AD713X_REG_CH1_GAIN_LSB			0x2D
#define AD713X_REG_CH1_GAIN_MID			0x2E
#define AD713X_REG_CH1_GAIN_MSB			0x2F
#define AD713X_REG_CH1_OFFSET_LSB		0x30
#define AD713X_REG_CH1_OFFSET_MID		0x31
#define AD713X_REG_CH1_OFFSET_MSB		0x32
#define AD713X_REG_CH2_GAIN_LSB			0x33
#define AD713X_REG_CH2_GAIN_MID			0x34
#define AD713X_REG_CH2_GAIN_MSB			0x35
#define AD713X_REG_CH2_OFFSET_LSB		0x36
#define AD713X_REG_CH2_OFFSET_MID		0x37
#define AD713X_REG_CH2_OFFSET_MSB		0x38
#define AD713X_REG_CH3_GAIN_LSB			0x39
#define AD713X_REG_CH3_GAIN_MID			0x3A
#define AD713X_REG_CH3_GAIN_MSB			0x3B
#define AD713X_REG_CH3_OFFSET_LSB		0x3C
#define AD713X_REG_CH3_OFFSET_MID		0x3D
#define AD713X_REG_CH3_OFFSET_MSB		0x3E
#define AD713X_REG_MCLK_COUNTER			0x3F
#define AD713X_REG_DIG_FILTER_OFUF		0x40
#define AD713X_REG_DIG_FILTER_SETTLED		0x41
#define AD713X_REG_INTERNAL_ERROR		0x42
#define AD713X_REG_POWER_OV_ERROR_1		0x43
#define AD713X_REG_POWER_UV_ERROR_1		0x44
#define AD713X_REG_POWER_OV_ERROR_2		0x45
#define AD713X_REG_POWER_UV_ERROR_2		0x46
#define AD713X_REG_SPI_ERROR			0x47
#define AD713X_REG_AIN_OR_ERROR			0x48
#define AD713X_REG_AVDD5_VALUE			0x49
#define AD713X_REG_DVDD5_VALUE			0x4A
#define AD713X_REG_VREF_VALUE			0x4B
#define AD713X_REG_LDOIN_VALUE			0x4C
#define AD713X_REG_AVDD1V8_VALUE		0x4D
#define AD713X_REG_DVDD1V8_VALUE		0x4E
#define AD713X_REG_CLKVDD_VALUE			0x4F
#define AD713X_REG_IOVDD_VALUE			0x50
#define AD713X_REG_TEMPERATURE_DATA		0x51

/**
 * @enum ad713x_supported_dev_ids
 * @brief ID of devices supported by the driver
 */
enum ad713x_supported_dev_ids {
    ID_AD7132,
    ID_AD7134,
    ID_AD7136,
    ID_AD4134
};

/**
 * @enum ad713x_power_mode
 * @brief AD713x power modes
 */
enum ad713x_power_mode {
    /** Low power mode option */
    LOW_POWER,
    /** Full power mode option */
    HIGH_POWER
};

/**
 * @enum ad713x_adc_data_len
 * @brief AD713x possible number of bits per data sample
 */
enum ad713x_adc_data_len {
    /** 16 bit data sample */
    ADC_16_BIT_DATA,
    /** 24 bit data sample */
    ADC_24_BIT_DATA,
    /** 32 bit data sample */
    ADC_32_BIT_DATA,
    /** To know when to stop when cycling between them */
    INVALID
};

/**
 * @enum ad713x_crc_header
 * @brief AD713x possible data CRC header choices
 */
enum ad713x_crc_header {
    /** Data sample comes with no CRC attached */
    NO_CRC,
    /** Data sample comes with 6-bit CRC attached */
    CRC_6,
    /** Data sample comes with 8-bit CRC attached */
    CRC_8
};

/**
 * @enum ad713x_doutx_format
 * @brief AD713x list for possible output modes
 */
enum ad713x_doutx_format {
    /** Single channel Daisy-chain mode */
    SINGLE_CH_DC,
    /** Dual channel Daisy-chain mode */
    DUAL_CH_DC,
    /** Quad-channel parallel output mode */
    QUAD_CH_PO,
    /** Channel average mode */
    CH_AVG_MODE
};

/**
 * @enum ad713x_dig_filter_sel
 * @brief AD713x list of input filters.
 */
enum ad713x_dig_filter_sel {
    /** Wideband filter (Finite impulse response) */
    FIR,
    /** Sinc6 filter */
    SINC6,
    /** Sinc3 filter */
    SINC3,
    /** Sinc3 filter with 50Hz and 60Hz rejection */
    SINC3_50_60_REJ
};

/**
 * @enum ad713x_channels
 * @brief AD713x list of channels
 */
enum ad713x_channels {
    /** Channel 0 */
    CH0,
    /** Channel 1 */
    CH1,
    /** Channel 2 */
    CH2,
    /** Channel 3 */
    CH3,
    /** Max number of channels */
    AD713X_CH_MAX
};

/**
 * @enum ad717x_mpc_clkdel
 * @brief AD713x list of clock delays
 */
enum ad717x_mpc_clkdel {
    /** No delay */
    DELAY_NONE,
    /** One clock cycle delay */
    DELAY_1_CLOCKS,
    /** Two clock cycles delay */
    DELAY_2_CLOCKS
};



#endif /* SRC_AD713X_H_ */

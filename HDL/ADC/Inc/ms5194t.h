#ifndef MS5194T_H_
#define MS5194T_H_

#define HDL_ADC_MS5194T_PRV_SIZE        64

#define HDL_ADC_MS5194T_INVALID_VALUE   0xFFFFFFFF

/* Write Enable Bit. A 0 must be written to this bit so that the write to the communications register actually occurs. If
   a 1 is the first bit written, the part does not clock on to subsequent bits in the register. It stays at this bit location
   until a 0 is written to this bit. Once a 0 is written to the WEN bit, the next seven bits are loaded to the
   communications register */
#define MS5194T_COMM_REG_WEN                 ((uint8_t)(1 << 7))
/* A 0 in this bit location indicates that the next operation is a write to a specified register. A 1 in this position
  indicates that the next operation is a read from the designated register. */
#define MS5194T_COMM_REG_READ                ((uint8_t)(1 << 6))

/* Register Address Bits. These address bits are used to select which registers of the ADC are being selected during
   this serial interface communication*/
#define MS5194T_COMM_REG_RS                  ((uint8_t)(7 << 3))
/* Communications Register During a Write Operation 8-bit */
#define MS5194T_COMM_REG_RS_COMM_REG         ((uint8_t)(0 << 3))
/* Status Register During a Read Operation 8-bit */
#define MS5194T_COMM_REG_RS_STATUS_REG       ((uint8_t)(0 << 3))
/* Mode Register 16-bit */
#define MS5194T_COMM_REG_RS_MODE_REG         ((uint8_t)(1 << 3))
/* Configuration Register 16-bit */
#define MS5194T_COMM_REG_RS_CONFIG_REG       ((uint8_t)(2 << 3))
/* Data Register 24-bit (AD7794)/16-Bit (AD7795) */
#define MS5194T_COMM_REG_RS_DATA_REG         ((uint8_t)(3 << 3))
/* ID Register 8-bit */
#define MS5194T_COMM_REG_RS_ID_REG           ((uint8_t)(4 << 3))
/* IO Register 8-bit */
#define MS5194T_COMM_REG_RS_IO_REG           ((uint8_t)(5 << 3))
/* Offset Register 24-bit (AD7794)/16-Bit (AD7795) */
#define MS5194T_COMM_REG_RS_OFFSET_REG       ((uint8_t)(6 << 3))
/* Full-Scale Register */
#define MS5194T_COMM_REG_RS_FULL_SCALE_REG   ((uint8_t)(7 << 3))

/* Continuous Read of the Data Register. When this bit is set to 1 (and the data register is selected), the serial
   interface is configured so that the data register can be read continuously, that is, the contents of the data register
   are automatically placed on the DOUT pin when the SCLK pulses are applied after the RDY pin goes low to
   indicate that a conversion is complete. The communications register does not have to be written to for data reads.
   To enable continuous read mode, the instruction 01011100 must be written to the communications register. To
   exit the continuous read mode, the instruction 01011000 must be written to the communications register while
   the RDY pin is low. While in continuous read mode, the ADC monitors activity on the DIN line so it can receive the
   instruction to exit continuous read mode. Additionally, a reset occurs if 32 consecutive 1s are seen on DIN.
   Therefore, DIN should be held low in continuous read mode until an instruction is written to the device. */
#define MS5194T_COMM_REG_CREAD               ((uint8_t)(1 << 2))

/* These bits must be programmed to Logic 0 for correct operation. */
#define MS5194T_COMM_REG_RESERVED            ((uint8_t)(3 << 0))



/* Ready Bit for ADC. Cleared when data is written to the ADC data register. The RDY bit is set automatically after the
   ADC data register has been read or a period of time before the data register is updated with a new conversion
   result to indicate to the user not to read the conversion data. It is also set when the part is placed in power-down
   mode. The end of a conversion is also indicated by the DOUT/RDY pin. This pin can be used as an alternative to the
   status register for monitoring the ADC for conversion data */
#define MS5194T_STATUS_REG_NOT_RDY           ((uint8_t)(1 << 7))
/* ADC Error Bit. This bit is written to at the same time as the RDY bit. Set to indicate that the result written to the
   ADC data register has been clamped to all 0s or all 1s. Error sources include overrange, underrange, or the absence
   of a reference voltage. Cleared by a write operation to start a conversion. */
#define MS5194T_STATUS_REG_ERR               ((uint8_t)(1 << 6))

/* No External Reference Bit. Set to indicate that the selected reference (REFIN1 or REFIN2) is at a voltage that is
   below a specified threshold. When set, conversion results are clamped to all 1s. Cleared to indicate that a valid
   reference is applied to the selected reference pins. The NOXREF bit is enabled by setting the REF_DET bit in the
   configuration register to 1. The ERR bit is also set if the voltage applied to the selected reference input is invalid. */
#define MS5194T_STATUS_REG_NOXREF            ((uint8_t)(1 << 5))

/* This bit is automatically cleared. */
#define MS5194T_STATUS_REG_4                 ((uint8_t)(1 << 4))

/* SR3 0/1 This bit is automatically cleared on the AD7795 and is automatically set on the AD7794. */
#define MS5194T_STATUS_REG_SR3               ((uint8_t)(1 << 3))

/* CH2 to CH0 These bits indicate which channel is being converted by the ADC. */
#define MS5194T_STATUS_REG_CH                ((uint8_t)(7 << 0))
#define MS5194T_STATUS_REG_CH0               ((uint8_t)(0 << 0))
#define MS5194T_STATUS_REG_CH1               ((uint8_t)(1 << 0))
#define MS5194T_STATUS_REG_CH2               ((uint8_t)(2 << 0))
#define MS5194T_STATUS_REG_CH3               ((uint8_t)(3 << 0))
#define MS5194T_STATUS_REG_CH4               ((uint8_t)(4 << 0))
#define MS5194T_STATUS_REG_CH5               ((uint8_t)(5 << 0))


/* Mode Select Bits. These bits select the operating mode of the AD7794/AD7795 */
#define MS5194T_MODE_REG_MS                  ((uint16_t)(7 << 13))
/* Continuous Conversion Mode (Default).
   In continuous conversion mode, the ADC continuously performs conversions and places the result in the data
   register. RDY goes low when a conversion is complete. The user can read these conversions by placing the device
   in continuous read mode whereby the conversions are automatically placed on the DOUT line when SCLK pulses
   are applied. Alternatively, the user can instruct the ADC to output the conversion by writing to the communications register. After power-on, the first conversion is available after a period of 2/fADC when chop is enabled or
   1/fADC when chop is di */
#define MS5194T_MODE_REG_MS_CONT             ((uint16_t)(0 << 13))
/* Single Conversion Mode.
   When single conversion mode is selected, the ADC powers up and performs a single conversion. The oscillator
   requires 1 ms to power up and settle. The ADC then performs the conversion, which takes a time of 2/fADC when
   chop is enabled, or 1/fADC when chop is disabled. The conversion result is placed in the data register, RDY goes
   low, and the ADC returns to power-down mode. The conversion remains in the data register and RDY remains
   active (low) until the data is read or another conversion is performed. */
#define MS5194T_MODE_REG_MS_SINGLE           ((uint16_t)(1 << 13))
/* Idle Mode.
   In idle mode, the ADC filter and modulator are held in a reset state although the modulator clocks are still
   provided. */
#define MS5194T_MODE_REG_MS_IDLE             ((uint16_t)(2 << 13))
/* Power-Down Mode.
   In power-down mode, all the AD7794/AD7795 circuitry is powered down including the current sources, power
   switch, burnout currents, bias voltage generator, and clock circuitry. */
#define MS5194T_MODE_REG_MS_PWR_DWN          ((uint16_t)(3 << 13))
/* Internal Zero-Scale Calibration.
   An internal short is automatically connected to the enabled channel. A calibration takes two conversion cycles to
   complete when chop is enabled and one conversion cycle when chop is disabled. RDY goes high when the
   calibration is initiated and returns low when the calibration is complete. The ADC is placed in idle mode following
   a calibration. The measured offset coefficient is placed in the offset register of the selected channel. */
#define MS5194T_MODE_REG_MS_CAL_I_ZERO_SCALE ((uint16_t)(4 << 13))
/* Internal Full-Scale Calibration.
   A full-scale input voltage is automatically connected to the selected analog input for this calibration.
   When the gain equals 1, a calibration takes two conversion cycles to complete when chop is enabled and one
   conversion cycle when chop is disabled.
   For higher gains, four conversion cycles are required to perform the full-scale calibration when chop is enabled
   and 2 conversion cycles when chop is disabled.
   RDY goes high when the calibration is initiated and returns low when the calibration is complete. The ADC is
   placed in idle mode following a calibration. The measured full-scale coefficient is placed in the full-scale register
   of the selected channel.
   Internal full-scale calibrations cannot be performed when the gain equals 128. With this gain setting, a system
   full-scale calibration can be performed. A full-scale calibration is required each time the gain of a channel is
   changed to minimize the full-scale error. */
#define MS5194T_MODE_REG_MS_CAL_I_FULL_SCALE ((uint16_t)(5 << 13))
/* System Zero-Scale Calibration.
   User should connect the system zero-scale input to the channel input pins as selected by the CH2 bit, CH1 bit,
   and CH0 bit. A system offset calibration takes two conversion cycles to complete when chop is enabled and one
   conversion cycle when chop is disabled. RDY goes high when the calibration is initiated and returns low when
   the calibration is complete. The ADC is placed in idle mode following a calibration. The measured offset coefficient 
   is placed in the offset register of the selected channel.  */
#define MS5194T_MODE_REG_MS_CAL_S_ZERO_SCALE ((uint16_t)(6 << 13))
/* System Full-Scale Calibration.
   User should connect the system full-scale input to the channel input pins as selected by the CH2 bit, CH1 bit, and
   CH0 bit.
   A calibration takes two conversion cycles to complete when chop is enabled and one conversion cycle when
   chop is disabled. RDY goes high when the calibration is initiated and returns low when the calibration is
   complete. The ADC is placed in idle mode following a calibration. The measured full-scale coefficient is placed in
   the full-scale register of the selected channel.
   A full-scale calibration is required each time the gain of a channel is changed. */
#define MS5194T_MODE_REG_MS_CAL_S_FULL_SCALE ((uint16_t)(7 << 13))

/* Power Switch Control Bit. Set by user to close the power switch PSW to GND. The power switch can sink
   up to 30 mA. Cleared by user to open the power switch. When the ADC is placed in power-down mode,
   the power switch is opened. */
#define MS5194T_MODE_REG_PSW                 ((uint16_t)(1 << 12))

/* These bits must be programmed with a Logic 0 for correct operation */
#define MS5194T_MODE_REG_10_11               ((uint16_t)(3 << 10))

/* Instrumentation Amplifier Common-Mode Bit. This bit is used in conjunction with the CHOP-DIS bit. With
   chop disabled, the user can operate with a wider range of common-mode voltages when AMP-CM is
   cleared. However, the dc common-mode rejection degrades. With AMP-CM set, the span for the commonmode voltage is reduced (see the Specifications section). However, the dc common-mode rejection is
   significantly better */
#define MS5194T_MODE_REG_AMP_CM              ((uint16_t)(1 << 9))

/* This bit must be programmed with a Logic 0 for correct operation. */
#define MS5194T_MODE_REG_8                   ((uint16_t)(1 << 8))

/* These bits are used to select the clock source for the AD7794/AD7795. Either the on-chip 64 kHz clock can
   be used or an external clock can be used. The ability to use an external clock allows several AD7794/AD7795
   devices to be synchronized. Also, 50 Hz/60 Hz rejection is improved when an accurate external clock
   drives the AD7794/AD7795. */
#define MS5194T_MODE_REG_CLK                 ((uint16_t)(3 << 6))
/* Internal 64 kHz clock. Internal clock is not available at the CLK pin. */
#define MS5194T_MODE_REG_CLK_INT64K          ((uint16_t)(0 << 6))
/* Internal 64 kHz clock. This clock is made available at the CLK pin.  */
#define MS5194T_MODE_REG_CLK_INT64K_OUT      ((uint16_t)(1 << 6))
/* External 64 kHz. The external clock can have a 45:55 duty cycle */
#define MS5194T_MODE_REG_CLK_EXT64K          ((uint16_t)(2 << 6))
/* External clock. The external clock is divided by 2 within the AD7794/AD7795 */
#define MS5194T_MODE_REG_CLK_EXT64K_DIV2     ((uint16_t)(3 << 6))

/* This bit must be programmed with a Logic 0 for correct operation. */
#define MS5194T_MODE_REG_5                   ((uint16_t)(1 << 5))

/* This bit is used to enable or disable chop. On power-up or following a reset, CHOP-DIS is cleared so chop is
   enabled. When CHOP-DIS is set, chop is disabled. This bit is used in conjunction with the AMP-CM bit.
   When chop is disabled, the AMP-CM bit should be set. This limits the common-mode voltage that can be
   used by the ADC, but the dc common-mode rejection does not degrade. */
#define MS5194T_MODE_REG_CHOP_DIS            ((uint16_t)(1 << 4))

/* Filter Update Rate Select Bits */
/* 0: F_ADC _ Hz T_SETTLE _ ms */
/* 1: F_ADC 470Hz T_SETTLE 4 ms */
/* 2: F_ADC 242Hz T_SETTLE 8 ms */
/* 3: F_ADC 123Hz T_SETTLE 16 ms */
/* 4: F_ADC 62Hz T_SETTLE 32 ms */
/* 5: F_ADC 50Hz T_SETTLE 40 ms */
/* 6: F_ADC 39Hz T_SETTLE 48 ms */
/* 7: F_ADC 33.2Hz T_SETTLE 60 ms */
/* 8: F_ADC 19.6Hz T_SETTLE 101 ms Rejection 90 dB (60 Hz only) */
/* 9: F_ADC 16.7Hz T_SETTLE 120 ms Rejection 80 dB (50 Hz only) */
/* 10:F_ADC 16.7Hz T_SETTLE 120 ms Rejection 65 dB (50 Hz and 60 Hz) */
/* 11:F_ADC 12.5Hz T_SETTLE 160 ms Rejection 66 dB (50 Hz and 60 Hz) */
/* 12:F_ADC 10Hz T_SETTLE 200 ms Rejection 69 dB (50 Hz and 60 Hz) */
/* 13:F_ADC 8.33Hz T_SETTLE 240 ms Rejection 70 dB (50 Hz and 60 Hz) */
/* 14:F_ADC 6.25Hz T_SETTLE 320 ms Rejection 72 dB (50 Hz and 60 Hz) */
/* 15:F_ADC 4.17Hz T_SETTLE 480 ms Rejection 74 dB (50 Hz and 60 Hz) */
#define MS5194T_MODE_REG_FILTER_RATE(x)      ((uint16_t)((x & 15) << 0))


/* Bias Voltage Generator Enable. The negative terminal of the analog inputs can be biased up to AVDD/2.
   These bits are used in conjunction with the BOOST bit*/
#define MS5194T_CONFIG_REG_VBIAS             ((uint16_t)(3 << 14))
/* Bias voltage generator disabled  */
#define MS5194T_CONFIG_REG_VBIAS_DISBL       ((uint16_t)(0 << 14))
/* Bias voltage generator connected to AIN1(−) */
#define MS5194T_CONFIG_REG_VBIAS_AIN1N       ((uint16_t)(1 << 14))
/* Bias voltage generator connected to AIN2(−) */
#define MS5194T_CONFIG_REG_VBIAS_AIN2N       ((uint16_t)(2 << 14))
/* Bias voltage generator connected to AIN3(−) */
#define MS5194T_CONFIG_REG_VBIAS_AIN3N       ((uint16_t)(3 << 14))

/* Burnout Current Enable Bit. This bit must be programmed with a Logic 0 for correct operation. When this
   bit is set to 1 by the user, the 100 nA current sources in the signal path are enabled. When BO = 0, the
   burnout currents are disabled. The burnout currents can be enabled only when the buffer or in-amp is active */
#define MS5194T_CONFIG_REG_BO                ((uint16_t)(1 << 13))

/* Unipolar/Bipolar Bit. Set by user to enable unipolar coding, that is, zero differential input results in
   0x000000 output and a full-scale differential input results in 0xFFFFFF output. Cleared by the user to
   enable bipolar coding. Negative full-scale differential input results in an output code of 0x000000, zero
   differential input results in an output code of 0x800000, and positive full-scale differential input results in
   an output code of 0xFFFFFF. */
#define MS5194T_CONFIG_REG_UB                ((uint16_t)(1 << 12))

/* This bit is used in conjunction with the VBIAS1 and VBIAS0 bits. When set, the current consumed by the
   bias voltage generator is increased, which reduces its power-up time. */
#define MS5194T_CONFIG_REG_BOOST             ((uint16_t)(1 << 11))

/* Gain Select Bits.
   Written by the user to select the ADC input range as follows:
   0: Gain 1 (in-amp not used) ADC Input Range 2.5 V
   1: Gain 2 (in-amp not used) ADC Input Range 1.25 V
   2: Gain 4 ADC Input Range 625 mV
   3: Gain 8 ADC Input Range 312.5 mV
   4: Gain 16 ADC Input Range 156.2 mV
   5: Gain 32 ADC Input Range 78.125 mV
   6: Gain 64 ADC Input Range 39.06 mV
   7: Gain 128 ADC Input Range 19.53 mV */
#define MS5194T_CONFIG_REG_IN_AMP(x)         ((uint16_t)((x & 7) << 8))

/* Reference Select Bits.
   The reference source for the ADC is selected using these bits. */
#define MS5194T_CONFIG_REG_REFSEL            ((uint16_t)(3 << 6))
/* External reference applied between REFIN1(+) and REFIN1(−) */
#define MS5194T_CONFIG_REG_REFSEL_EXTIN1     ((uint16_t)(0 << 6))
/* External reference applied between REFIN2(+) and REFIN2(−) */
#define MS5194T_CONFIG_REG_REFSEL_EXTIN2     ((uint16_t)(1 << 6))
/* Internal 1.17 V reference */
#define MS5194T_CONFIG_REG_REFSEL_INT_1V17   ((uint16_t)(2 << 6))

/* Enables the reference detect function. When set, the NOXREF bit in the status register indicates when the
   external reference being used by the ADC is open circuit or less than 0.5 V. When cleared, the reference
   detect function is disabled. */
#define MS5194T_CONFIG_REG_REF_DET           ((uint16_t)(1 << 5))

/* Configures the ADC for buffered or unbuffered mode of operation. If cleared, the ADC operates in
   unbuffered mode, lowering the power consumption of the device. If set, the ADC operates in buffered
   mode, allowing the user to place source impedances on the front end without contributing gain errors to
   the system. For gains of 1 and 2, the buffer can be enabled or disabled. For higher gains, the buffer is
   automatically enabled. With the buffer disabled, the voltage on the analog input pins can be from 30 mV
   below GND to 30 mV above AVDD. When the buffer is enabled, it requires some headroom so the voltage
   on any input pin must be limited to 100 mV within the power supply rails. */
#define MS5194T_CONFIG_REG_BUF               ((uint16_t)(1 << 4))

/* Channel Select Bits.
   Written by the user to select the active analog input channel to the ADC.
   0: Active channel AIN1(+)/AIN1(−) Calibration Pair 0
   1: Active channel AIN2(+)/AIN2(−) Calibration Pair 1
   2: Active channel AIN3(+)/AIN3(−) Calibration Pair 2
   3: Active channel AIN4(+)/AIN4(−) Calibration Pair 3
   4: Active channel AIN5(+)/AIN5(−) Calibration Pair 3
   5: Active channel AIN6(+)/AIN6(−) Calibration Pair 3
   6: Active channel Temp Sensor Calibration Pair Automatically selects the internal 1.17 V reference and sets the gain to 1
   7: Active channel AVDD Monitor Calibration Pair Automatically selects the internal 1.17 V reference and sets the gain to 1/6
   8: Active channel AIN1(−)/AIN1(−) Calibration Pair 0 */
#define MS5194T_CONFIG_REG_CH_SEL(x)         ((uint16_t)((x & 15) << 0))



/* This bit must be programmed with a Logic 0 for correct operation. */
#define MS5194T_IO_REG_7                     ((uint8_t)(1 << 7))
/* Configures Pin AIN6(+)/P1 and Pin AIN6(−)/P2 as analog input pins or digital output pins. When this
   bit is set, the pins are configured as Digital Output Pin P1 and Digital Output Pin P2. When this bit is
   cleared, these pins are configured as Analog Input Pin AIN6(+) and Analog Input Pin AIN6(−) */
#define MS5194T_IO_REG_IOEN                  ((uint8_t)(1 << 6))
/* P2 Data. When IOEN is set, the data for Digital Output Pin P2 is written to Bit IO2DAT. */
#define MS5194T_IO_REG_IO2DAT                ((uint8_t)(1 << 5))
/* P1 Data. When IOEN is set, the data for Digital Output Pin P1 is written to Bit IO1DAT. */
#define MS5194T_IO_REG_IO1DAT                ((uint8_t)(1 << 4))

/* Direction of Current Sources Select Bits. */
#define MS5194T_IO_REG_IEXCDIR               ((uint8_t)(3 << 2))
/* Current Source IEXC1 connected to Pin IOUT1. Current Source IEXC2 connected to Pin IOUT2. */
#define MS5194T_IO_REG_IEXCDIR_11_22         ((uint8_t)(0 << 2))
/* Current Source IEXC1 connected to Pin IOUT2. Current Source IEXC2 connected to Pin IOUT1. */
#define MS5194T_IO_REG_IEXCDIR_12_21         ((uint8_t)(1 << 2))
/* Both current sources connected to Pin IOUT1. Permitted only when the current sources are set to 10 μA or 210 μA.  */
#define MS5194T_IO_REG_IEXCDIR_11_21         ((uint8_t)(2 << 2))
/* Both current sources connected to Pin IOUT2. Permitted only when the current sources are set to 10 μA or 210 μA. */
#define MS5194T_IO_REG_IEXCDIR_12_22         ((uint8_t)(3 << 2))

/* These bits are used to enable and disable the current sources. They also select the value of the
   excitation currents.  */
#define MS5194T_IO_REG_IEXCEN                ((uint8_t)(3 << 0))
/* Excitation currents disabled */
#define MS5194T_IO_REG_IEXCEN_DISBL          ((uint8_t)(0 << 0))
/* Excitation current 10 μA */
#define MS5194T_IO_REG_IEXCEN_10UA           ((uint8_t)(1 << 0))
/* Excitation current 210 μA */
#define MS5194T_IO_REG_IEXCEN_210UA          ((uint8_t)(2 << 0))
/* Excitation current 1 mA */
#define MS5194T_IO_REG_IEXCEN_1MA            ((uint8_t)(3 << 0))


#define MS5194T_MODE_REG_DEFAULT             MS5194T_MODE_REG_FILTER_RATE(10)

#define MS5194T_CONFIG_REG_DEFAULT           MS5194T_CONFIG_REG_BUF | MS5194T_CONFIG_REG_IN_AMP(7)

#define MS5194T_IO_REG_DEFAULT               !MS5194T_IO_REG_IOEN

typedef enum {
  HDL_ADC_MS5194T_CHANNEL_ENABLE               = 0x01,
  HDL_ADC_MS5194T_CHANNEL_ZERO_SCALE_CALIBRATE = 0x02,
  HDL_ADC_MS5194T_CHANNEL_FULL_SCALE_CALIBRATE = 0x04,
} hdl_adc_ms5194t_channel_option_t;

typedef struct {
  uint16_t config_reg;
  hdl_adc_ms5194t_channel_option_t options;
} hdl_adc_ms5194t_source_t;

#define _hdl_adc_ms5194t_src(...)        ((hdl_adc_ms5194t_source_t *[]){__VA_ARGS__, NULL})
#define hdl_adc_ms5194t_sources(...)     _hdl_adc_ms5194t_src(__VA_ARGS__),\
                                         .values = (uint32_t [sizeof(_hdl_adc_ms5194t_src(__VA_ARGS__))/sizeof(hdl_adc_ms5194t_source_t *)]){}

typedef struct {
  uint8_t io_reg;
  uint16_t mode_reg;
  hdl_adc_ms5194t_source_t **sources;
  uint32_t *values;
} hdl_adc_ms5194t_config_t;

/*
    depends on 
    spi_client
    rdy_pin
    time_counter
 */
typedef struct {
  hdl_module_t module;
  const hdl_adc_ms5194t_config_t *config;
  PRIVATE(hdl, HDL_ADC_MS5194T_PRV_SIZE);
} hdl_adc_ms5194t_t;

hdl_module_state_t hdl_adc_ms5194t(void *desc, uint8_t enable);

uint32_t hdl_adc_ms5194t_get(hdl_adc_ms5194t_t *hdl_adc, uint32_t src);
uint32_t hdl_adc_ms5194t_age(hdl_adc_ms5194t_t *hdl_adc);

#endif /* MS5194T_H_ */

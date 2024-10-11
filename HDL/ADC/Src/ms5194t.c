#include "app.h"
#include "CodeLib.h"

#define MS5194T_RX_MESSAGE_SIZE    3
#define MS5194T_TX_MESSAGE_SIZE    4

typedef struct {
  hdl_module_t module;
  const hdl_adc_ms5194t_config_t *config;
  struct {
    coroutine_t worker;
    uint32_t timestamp;
    uint8_t state;
    uint8_t command_state;
    uint8_t src_count;
    uint8_t src_current;
    uint32_t rx_data;
    uint32_t tx_data;
    uint8_t comm_reg;
    uint8_t src_calibrated;
    hdl_spi_message_t message;
  } private;  
} hdl_adc_ms5194t_private_t;

HDL_ASSERRT_STRUCTURE_CAST(hdl_adc_ms5194t_private_t, hdl_adc_ms5194t_t, HDL_ADC_MS5194T_PRV_SIZE, ms5194t.h);

#define MS5194T_ADC_COMMAND_STATE_IDLE                  0
#define MS5194T_ADC_COMMAND_STATE_CS                    1
#define MS5194T_ADC_COMMAND_STATE_CS_RELEASE            2
#define MS5194T_ADC_COMMAND_STATE_TX_COMM_REG           4
#define MS5194T_ADC_COMMAND_STATE_XFER_DATA_8           8
#define MS5194T_ADC_COMMAND_STATE_XFER_DATA_16         16
#define MS5194T_ADC_COMMAND_STATE_XFER_DATA_32         32
#define MS5194T_ADC_COMMAND_STATE_XFER_DATA_MSK        56
#define MS5194T_ADC_COMMAND_STATE_PUSH_MESSAGE         64
#define MS5194T_ADC_COMMAND_STATE_MESSAGE_AWAITING    128

static void _adc_xfer(hdl_adc_ms5194t_private_t *adc) {
  while(adc->private.command_state != MS5194T_ADC_COMMAND_STATE_IDLE) {
    if(adc->private.command_state & MS5194T_ADC_COMMAND_STATE_PUSH_MESSAGE) {
      hdl_spi_client_ch_t *spi = (hdl_spi_client_ch_t *)adc->module.dependencies[0];
      if(hdl_spi_transfer_message(spi, &adc->private.message)) {
        adc->private.command_state &= ~MS5194T_ADC_COMMAND_STATE_PUSH_MESSAGE;
        adc->private.command_state |= MS5194T_ADC_COMMAND_STATE_MESSAGE_AWAITING;
      }
      break;
    }
    else {
      if(adc->private.command_state & MS5194T_ADC_COMMAND_STATE_MESSAGE_AWAITING) {
        if(!(adc->private.message.state & HDL_SPI_MESSAGE_STATUS_COMPLETE)) break;
        adc->private.command_state &= ~MS5194T_ADC_COMMAND_STATE_MESSAGE_AWAITING;
      }
      else {
        adc->private.message.rx_skip = 0;
        adc->private.message.rx_take = 0;
        adc->private.message.tx_len = 0;
        adc->private.message.options = 0;
        adc->private.command_state |= MS5194T_ADC_COMMAND_STATE_PUSH_MESSAGE;
        if(adc->private.command_state & MS5194T_ADC_COMMAND_STATE_CS) {
          adc->private.message.options = HDL_SPI_MESSAGE_CH_SELECT;
          adc->private.command_state &= ~MS5194T_ADC_COMMAND_STATE_CS;
        }
        else if(adc->private.command_state & MS5194T_ADC_COMMAND_STATE_TX_COMM_REG) {
          adc->private.message.tx_len = 1;
          adc->private.message.tx_buffer = (uint8_t *)&adc->private.comm_reg;
          adc->private.command_state &= ~MS5194T_ADC_COMMAND_STATE_TX_COMM_REG;
        }
        else if(adc->private.command_state & MS5194T_ADC_COMMAND_STATE_XFER_DATA_MSK) {
          adc->private.message.tx_len = (adc->private.command_state & MS5194T_ADC_COMMAND_STATE_XFER_DATA_8)? 1: 0;
          adc->private.message.tx_len |= (adc->private.command_state & MS5194T_ADC_COMMAND_STATE_XFER_DATA_16)? 2: 0;
          adc->private.message.tx_len |= (adc->private.command_state & MS5194T_ADC_COMMAND_STATE_XFER_DATA_32)? 4: 0;
          adc->private.message.rx_take = adc->private.message.tx_len;
          adc->private.message.tx_buffer = (uint8_t *)&adc->private.tx_data;
          adc->private.message.rx_buffer = (uint8_t *)&adc->private.rx_data;
          adc->private.command_state &= ~MS5194T_ADC_COMMAND_STATE_XFER_DATA_MSK;
        }
        else if(adc->private.command_state & MS5194T_ADC_COMMAND_STATE_CS_RELEASE) {
          adc->private.command_state &= ~MS5194T_ADC_COMMAND_STATE_CS_RELEASE;
          adc->private.message.options |= HDL_SPI_MESSAGE_CH_RELEASE;
        }

      }
    }
  }
}

#define MS5194T_ADC_TASK_INIT                     0x00
#define MS5194T_ADC_TASK_ZS_CALIBR                0x10
#define MS5194T_ADC_TASK_FS_CALIBR                0x20
#define MS5194T_ADC_TASK_CONVERT                  0x30
#define MS5194T_ADC_TASK_GET_VALUE                0x40
#define MS5194T_ADC_TASK_UPDATE_VALUE16           0x50
#define MS5194T_ADC_TASK_UPDATE_VALUE24           0x60
#define MS5194T_ADC_TASK_CALIB_COMPLETE           0x70

#define MS5194T_ADC_TASK_MASK                     0xF0

#define MS5194T_ADC_STATE_RESET                   0x00
#define MS5194T_ADC_STATE_SET_MODE                0x01
#define MS5194T_ADC_STATE_SET_IO                  0x02
#define MS5194T_ADC_STATE_SET_CH_CONFIG           0x03
#define MS5194T_ADC_STATE_GET_CH_CONFIG           0x04
#define MS5194T_ADC_STATE_VALIDATE_CONFIG         0x05
#define MS5194T_ADC_STATE_DO_WORK                 0x06
#define MS5194T_ADC_STATE_CS                      0x07
#define MS5194T_ADC_STATE_CHECK_READY             0x08
#define MS5194T_ADC_STATE_DEINIT                  0x09
#define MS5194T_ADC_STATE_UNLOAD                  0x0A

#define MS5194T_ADC_STATE_MASK                    0x0F

#define MS5194T_ADC_STATE_INIT                    MS5194T_ADC_TASK_INIT | MS5194T_ADC_STATE_RESET


#define _adc_ms5194t_msb_u8_data(x)              (x & 0xff)
#define _adc_ms5194t_msb_u16_data(x)             (((x << 8) & 0xff00) | ((x >> 8) & 0x00ff))
#define _adc_ms5194t_msb_u24_data(x)             (((x >> 16) & 0x0000ff) | (x & 0x00ff00) | ((x << 16) & (0xff0000)))

static uint8_t _adc_ms5194t_worker(coroutine_t *this, uint8_t cancel, void *arg) {
  hdl_adc_ms5194t_private_t *adc = (hdl_adc_ms5194t_private_t *)arg;
  if(adc->private.command_state != MS5194T_ADC_COMMAND_STATE_IDLE) {
    _adc_xfer(adc);
  }
  else {
    switch (adc->private.state & MS5194T_ADC_STATE_MASK) {
      case MS5194T_ADC_STATE_RESET: {
        adc->private.src_current = adc->private.src_count;
        adc->private.src_calibrated = 0;
        adc->private.tx_data = 0xffffffff;
        adc->private.command_state = MS5194T_ADC_COMMAND_STATE_CS | MS5194T_ADC_COMMAND_STATE_XFER_DATA_32 | MS5194T_ADC_COMMAND_STATE_CS_RELEASE;
        adc->private.state = MS5194T_ADC_STATE_SET_MODE;
        break;
      }

      case MS5194T_ADC_STATE_SET_MODE: {
        adc->private.comm_reg = !MS5194T_COMM_REG_WEN | !MS5194T_COMM_REG_READ | MS5194T_COMM_REG_RS_MODE_REG | !MS5194T_COMM_REG_CREAD;
        uint16_t mode = adc->config->mode_reg & ~MS5194T_MODE_REG_MS | MS5194T_MODE_REG_MS_PWR_DWN;
        adc->private.tx_data = _adc_ms5194t_msb_u16_data(mode);
        adc->private.command_state = MS5194T_ADC_COMMAND_STATE_CS | MS5194T_ADC_COMMAND_STATE_TX_COMM_REG | 
          MS5194T_ADC_COMMAND_STATE_XFER_DATA_16 | MS5194T_ADC_COMMAND_STATE_CS_RELEASE;
        adc->private.state = MS5194T_ADC_STATE_SET_IO;
        break;
      }

      case MS5194T_ADC_STATE_SET_IO: {
        adc->private.comm_reg = !MS5194T_COMM_REG_WEN | !MS5194T_COMM_REG_READ | MS5194T_COMM_REG_RS_IO_REG | !MS5194T_COMM_REG_CREAD;
        adc->private.tx_data = _adc_ms5194t_msb_u8_data(adc->config->io_reg);
        adc->private.command_state = MS5194T_ADC_COMMAND_STATE_CS | MS5194T_ADC_COMMAND_STATE_TX_COMM_REG | 
          MS5194T_ADC_COMMAND_STATE_XFER_DATA_8 | MS5194T_ADC_COMMAND_STATE_CS_RELEASE;
        adc->private.state = MS5194T_ADC_STATE_SET_CH_CONFIG;
        break;
      }

      case MS5194T_ADC_STATE_SET_CH_CONFIG: {
        if(cancel) {
          adc->private.state = MS5194T_ADC_STATE_DEINIT;
          break;
        }
        if(adc->private.src_count > 0) {
          adc->private.src_current++;
          if(adc->private.src_current >= adc->private.src_count) {
            adc->private.src_current = 0;
            hdl_time_counter_t *time = (hdl_time_counter_t *)adc->module.dependencies[2];
            adc->private.timestamp = hdl_time_counter_get(time);
          }
          if(adc->config->sources[adc->private.src_current]->options & HDL_ADC_MS5194T_CHANNEL_ENABLE) {
            adc->private.comm_reg = !MS5194T_COMM_REG_WEN | !MS5194T_COMM_REG_READ | MS5194T_COMM_REG_RS_CONFIG_REG | !MS5194T_COMM_REG_CREAD;
            uint16_t cnf = adc->config->sources[adc->private.src_current]->config_reg;
            adc->private.tx_data = _adc_ms5194t_msb_u16_data(cnf);
            adc->private.command_state = MS5194T_ADC_COMMAND_STATE_CS | MS5194T_ADC_COMMAND_STATE_TX_COMM_REG | 
              MS5194T_ADC_COMMAND_STATE_XFER_DATA_16 | MS5194T_ADC_COMMAND_STATE_CS_RELEASE;
            adc->private.state = MS5194T_ADC_STATE_GET_CH_CONFIG;
          }
          else {
            if(adc->private.src_calibrated < (adc->private.src_current + 1)) {
              adc->private.src_calibrated = adc->private.src_current + 1;
            }
          }
        }
        break;
      }

      case MS5194T_ADC_STATE_GET_CH_CONFIG: {
        adc->private.comm_reg = !MS5194T_COMM_REG_WEN | MS5194T_COMM_REG_READ | MS5194T_COMM_REG_RS_CONFIG_REG | !MS5194T_COMM_REG_CREAD;
        uint16_t cnf = adc->config->sources[adc->private.src_current]->config_reg;
        adc->private.tx_data = 0;
        adc->private.command_state = MS5194T_ADC_COMMAND_STATE_CS | MS5194T_ADC_COMMAND_STATE_TX_COMM_REG | 
          MS5194T_ADC_COMMAND_STATE_XFER_DATA_16 | MS5194T_ADC_COMMAND_STATE_CS_RELEASE;
        adc->private.state = MS5194T_ADC_STATE_VALIDATE_CONFIG;
        break;
      }

      case MS5194T_ADC_STATE_VALIDATE_CONFIG: {
        uint16_t cnf = adc->private.rx_data;
        cnf = _adc_ms5194t_msb_u16_data(cnf);
        if(cnf == adc->config->sources[adc->private.src_current]->config_reg) {
          adc->private.state = MS5194T_ADC_STATE_DO_WORK;
          adc->private.state |=  
            (adc->private.src_calibrated < adc->private.src_count)? MS5194T_ADC_TASK_ZS_CALIBR : MS5194T_ADC_TASK_CONVERT;
        }
        else {
          adc->private.state = MS5194T_ADC_STATE_INIT;
        }
        break;
      }

      case MS5194T_ADC_STATE_DO_WORK: {
        adc->private.state = (adc->private.state & ~MS5194T_ADC_STATE_MASK) | MS5194T_ADC_STATE_CS;
        if((adc->private.state & MS5194T_ADC_TASK_MASK) == MS5194T_ADC_TASK_CALIB_COMPLETE) {
          adc->private.state = (adc->private.state & ~MS5194T_ADC_STATE_MASK) | MS5194T_ADC_STATE_SET_CH_CONFIG;
          adc->private.src_calibrated = (adc->private.src_current + 1);
          break;
        }

        if(((adc->private.state & MS5194T_ADC_TASK_MASK) == MS5194T_ADC_TASK_UPDATE_VALUE16) ||
           ((adc->private.state & MS5194T_ADC_TASK_MASK) == MS5194T_ADC_TASK_UPDATE_VALUE24)) {
          
          adc->private.state = (adc->private.state & ~MS5194T_ADC_STATE_MASK) | MS5194T_ADC_STATE_SET_CH_CONFIG;
          uint32_t data = adc->private.rx_data;
          adc->config->values[adc->private.src_current] = ((adc->private.state & MS5194T_ADC_TASK_MASK) == MS5194T_ADC_TASK_UPDATE_VALUE24)? 
                                                  _adc_ms5194t_msb_u24_data(data):
                                                  _adc_ms5194t_msb_u16_data(data);
          break;
        }

        if((adc->private.state & MS5194T_ADC_TASK_MASK) == MS5194T_ADC_TASK_ZS_CALIBR) {
          adc->private.state = (adc->private.state & ~MS5194T_ADC_TASK_MASK) | MS5194T_ADC_TASK_FS_CALIBR;
          if(adc->config->sources[adc->private.src_current]->options & HDL_ADC_MS5194T_CHANNEL_ZERO_SCALE_CALIBRATE) {
            adc->private.comm_reg = !MS5194T_COMM_REG_WEN | !MS5194T_COMM_REG_READ | MS5194T_COMM_REG_RS_MODE_REG | !MS5194T_COMM_REG_CREAD;
            uint16_t mode = adc->config->mode_reg & ~MS5194T_MODE_REG_MS | MS5194T_MODE_REG_MS_CAL_I_ZERO_SCALE;
            adc->private.tx_data = _adc_ms5194t_msb_u16_data(mode);
            adc->private.command_state = MS5194T_ADC_COMMAND_STATE_CS | MS5194T_ADC_COMMAND_STATE_TX_COMM_REG | 
              MS5194T_ADC_COMMAND_STATE_XFER_DATA_16 | MS5194T_ADC_COMMAND_STATE_CS_RELEASE;
            break;
          }
        }

        if((adc->private.state & MS5194T_ADC_TASK_MASK) == MS5194T_ADC_TASK_FS_CALIBR) {
          adc->private.state = (adc->private.state & ~MS5194T_ADC_TASK_MASK) | MS5194T_ADC_TASK_CALIB_COMPLETE;
          if(adc->config->sources[adc->private.src_current]->options & HDL_ADC_MS5194T_CHANNEL_FULL_SCALE_CALIBRATE) {
            adc->private.comm_reg = !MS5194T_COMM_REG_WEN | !MS5194T_COMM_REG_READ | MS5194T_COMM_REG_RS_MODE_REG | !MS5194T_COMM_REG_CREAD;
            uint16_t mode = adc->config->mode_reg & ~MS5194T_MODE_REG_MS | MS5194T_MODE_REG_MS_CAL_I_FULL_SCALE;
            adc->private.tx_data = _adc_ms5194t_msb_u16_data(mode);
            adc->private.command_state = MS5194T_ADC_COMMAND_STATE_CS | MS5194T_ADC_COMMAND_STATE_TX_COMM_REG | 
              MS5194T_ADC_COMMAND_STATE_XFER_DATA_16 | MS5194T_ADC_COMMAND_STATE_CS_RELEASE;
            break;
          }
        }

        if((adc->private.state & MS5194T_ADC_TASK_MASK) == MS5194T_ADC_TASK_CONVERT) {
          adc->private.state = (adc->private.state & ~MS5194T_ADC_TASK_MASK) | MS5194T_ADC_TASK_GET_VALUE;
          adc->private.comm_reg = !MS5194T_COMM_REG_WEN | !MS5194T_COMM_REG_READ | MS5194T_COMM_REG_RS_MODE_REG | !MS5194T_COMM_REG_CREAD;
          uint16_t mode = adc->config->mode_reg & ~MS5194T_MODE_REG_MS | MS5194T_MODE_REG_MS_SINGLE;
          adc->private.tx_data = _adc_ms5194t_msb_u16_data(mode);
          adc->private.command_state = MS5194T_ADC_COMMAND_STATE_CS | MS5194T_ADC_COMMAND_STATE_TX_COMM_REG | 
            MS5194T_ADC_COMMAND_STATE_XFER_DATA_16 | MS5194T_ADC_COMMAND_STATE_CS_RELEASE;
          break;
        }

        if((adc->private.state & MS5194T_ADC_TASK_MASK) == MS5194T_ADC_TASK_GET_VALUE) {
          uint8_t status = adc->private.rx_data;
          if(status & MS5194T_STATUS_REG_ERR) {
            adc->private.rx_data = HDL_ADC_MS5194T_INVALID_VALUE;
          }
          else {
            adc->private.comm_reg = !MS5194T_COMM_REG_WEN | MS5194T_COMM_REG_READ | MS5194T_COMM_REG_RS_DATA_REG | !MS5194T_COMM_REG_CREAD;
            adc->private.tx_data = 0;
            adc->private.command_state = MS5194T_ADC_COMMAND_STATE_CS | MS5194T_ADC_COMMAND_STATE_TX_COMM_REG | 
              MS5194T_ADC_COMMAND_STATE_XFER_DATA_16 | MS5194T_ADC_COMMAND_STATE_CS_RELEASE;
            if(status & MS5194T_STATUS_REG_SR3) 
              adc->private.command_state |= MS5194T_ADC_COMMAND_STATE_XFER_DATA_8; //if AD7794: read 24bit reg
          }
          adc->private.state = MS5194T_ADC_STATE_DO_WORK | 
            ((status & MS5194T_STATUS_REG_SR3)? MS5194T_ADC_TASK_UPDATE_VALUE24: MS5194T_ADC_TASK_UPDATE_VALUE16);;
        }
        break;
      }
      case MS5194T_ADC_STATE_CS: {
        adc->private.state = (adc->private.state & ~MS5194T_ADC_STATE_MASK) | MS5194T_ADC_STATE_CHECK_READY;
        adc->private.tx_data = 0;
        adc->private.rx_data = 0;
        adc->private.command_state = MS5194T_ADC_COMMAND_STATE_CS;
        break;
      }

      case MS5194T_ADC_STATE_CHECK_READY: {
        adc->private.command_state = MS5194T_ADC_COMMAND_STATE_CS_RELEASE;
        adc->private.state = (adc->private.state & ~MS5194T_ADC_STATE_MASK) | MS5194T_ADC_STATE_CS;
        hdl_gpio_pin_t *rdy_pin = (hdl_gpio_pin_t *)adc->module.dependencies[1];
        if(hdl_gpio_read(rdy_pin) == HDL_GPIO_LOW) {
          adc->private.comm_reg = !MS5194T_COMM_REG_WEN | MS5194T_COMM_REG_READ | MS5194T_COMM_REG_RS_STATUS_REG | !MS5194T_COMM_REG_CREAD;
          adc->private.tx_data = 0;
          adc->private.command_state = MS5194T_ADC_COMMAND_STATE_CS | MS5194T_ADC_COMMAND_STATE_TX_COMM_REG | 
            MS5194T_ADC_COMMAND_STATE_XFER_DATA_8 | MS5194T_ADC_COMMAND_STATE_CS_RELEASE;
          adc->private.state = (adc->private.state & ~MS5194T_ADC_STATE_MASK) | MS5194T_ADC_STATE_DO_WORK;
        }
        break;
      }

      case MS5194T_ADC_STATE_DEINIT: {
        adc->private.comm_reg = !MS5194T_COMM_REG_WEN | !MS5194T_COMM_REG_READ | MS5194T_COMM_REG_RS_MODE_REG | !MS5194T_COMM_REG_CREAD;
        uint16_t mode =  MS5194T_MODE_REG_MS_PWR_DWN | MS5194T_MODE_REG_FILTER_RATE(10);
        adc->private.tx_data = _adc_ms5194t_msb_u16_data(mode);
        adc->private.command_state = MS5194T_ADC_COMMAND_STATE_CS | MS5194T_ADC_COMMAND_STATE_TX_COMM_REG | 
          MS5194T_ADC_COMMAND_STATE_XFER_DATA_16 | MS5194T_ADC_COMMAND_STATE_CS_RELEASE;
        adc->private.state = MS5194T_ADC_STATE_UNLOAD;
        break;
      }
      case MS5194T_ADC_STATE_UNLOAD:
      default:
        return HDL_TRUE;
    }
  }
  return HDL_FALSE;
}

hdl_module_state_t hdl_adc_ms5194t(void *desc, uint8_t enable) {
  hdl_adc_ms5194t_private_t *adc = (hdl_adc_ms5194t_private_t *)desc;
  if(enable) {
    adc->private.state = MS5194T_ADC_STATE_INIT;
    adc->private.timestamp = 0;
    adc->private.src_count = 0;
    adc->private.src_calibrated = 0;
    adc->private.command_state = MS5194T_ADC_COMMAND_STATE_IDLE;
    if(adc->config->sources != NULL) {
      hdl_adc_ms5194t_source_t **src = adc->config->sources;
      while (*src != NULL) {
        adc->config->values[adc->private.src_count] = HDL_ADC_MS5194T_INVALID_VALUE;
        adc->private.src_count++;
        src++;
      }
    }
    adc->private.src_current = adc->private.src_count;
    coroutine_add(&adc->private.worker, &_adc_ms5194t_worker, desc);
    return HDL_MODULE_ACTIVE;
  }
  coroutine_state_t cor_state = coroutine_state(&adc->private.worker);
  if(cor_state == CO_ROUTINE_RUN) coroutine_cancel(&adc->private.worker);
  return (cor_state == CO_ROUTINE_CANCELED)? HDL_MODULE_UNLOADED: HDL_MODULE_UNLOADING;
}

uint32_t hdl_adc_ms5194t_get(hdl_adc_ms5194t_t *desc, uint32_t src) {
  hdl_adc_ms5194t_private_t *adc = (hdl_adc_ms5194t_private_t *)desc;
  if((adc != NULL) && (hdl_state(&adc->module) == HDL_MODULE_ACTIVE) && (adc->private.src_count > src)) {
    return adc->config->values[src];
  }
  return HDL_ADC_MS5194T_INVALID_VALUE;
}

uint32_t hdl_adc_ms5194t_age(hdl_adc_ms5194t_t *desc) {
  hdl_adc_ms5194t_private_t *adc = (hdl_adc_ms5194t_private_t *)desc;
  if((adc != NULL) && (hdl_state(&adc->module) == HDL_MODULE_ACTIVE)) {
    return adc->private.timestamp;
  }
  return 0;
}

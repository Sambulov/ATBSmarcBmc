
#include "app.h"
#include "CodeLib.h"

typedef struct {
  hdl_module_t module;
  const hdl_eeprom_i2c_config_t *config;
  struct {
    coroutine_t worker;
    hdl_i2c_message_t i2c_msg;
    hdl_eeprom_i2c_data_t *eeprom_msg;
    uint16_t mem_addr;
    uint8_t state;
    uint32_t burn_time;
  } private;  
} hdl_eeprom_i2c_private_t;

HDL_ASSERRT_STRUCTURE_CAST(hdl_eeprom_i2c_private_t, hdl_eeprom_i2c_t, HDL_EEPROM_I2C_PRV_SIZE, hdl_eeprom_i2c.h);

#define EE_STATE_IDLE                  0
#define EE_STATE_SET_MEM_ADDR_MSG      1
#define EE_STATE_TX_MEM_ADDR_MSG       2
#define EE_STATE_AWAIT_MEM_ADDR_MSG    3
#define EE_STATE_TX_DATA_MSG           4
#define EE_STATE_AWAIT_DATA_MSG        5
#define EE_STATE_AWAIT_BURNING         6


static uint8_t _eeprom_worker(coroutine_t *this, uint8_t cancel, void *arg) {
  hdl_eeprom_i2c_private_t *eeprom = (hdl_eeprom_i2c_private_t *)arg;
  switch (eeprom->private.state) {
    case EE_STATE_SET_MEM_ADDR_MSG: {
      eeprom->private.i2c_msg.address = eeprom->config->chip_address;
      uint16_t addr_msb = eeprom->private.eeprom_msg->address + eeprom->private.eeprom_msg->transferred;
      eeprom->private.mem_addr = (addr_msb >> 8) | (addr_msb << 8);
      eeprom->private.i2c_msg.buffer = (uint8_t *)&(eeprom->private.mem_addr);
      volatile uint8_t x = eeprom->private.i2c_msg.buffer[0];
      volatile uint8_t y = eeprom->private.i2c_msg.buffer[1];
      eeprom->private.i2c_msg.length = 2;
      eeprom->private.i2c_msg.options = HDL_I2C_MESSAGE_START | HDL_I2C_MESSAGE_ADDR;
      eeprom->private.state = EE_STATE_TX_MEM_ADDR_MSG;
      break;
    }

    case EE_STATE_TX_MEM_ADDR_MSG: {
      hdl_i2c_t *bus = (hdl_i2c_t *)eeprom->module.dependencies[0]; 
      if(hdl_i2c_transfer_message(bus, &eeprom->private.i2c_msg)) eeprom->private.state = EE_STATE_AWAIT_MEM_ADDR_MSG;
      break;
    }

    case EE_STATE_AWAIT_MEM_ADDR_MSG: {
      if(eeprom->private.i2c_msg.status & HDL_I2C_MESSAGE_STATUS_COMPLETE) {
        if(eeprom->private.i2c_msg.status & (HDL_I2C_MESSAGE_FAULT_ARBITRATION_LOST | HDL_I2C_MESSAGE_FAULT_BUS_ERROR | HDL_I2C_MESSAGE_FAULT_BAD_STATE)) {
          eeprom->private.eeprom_msg->state |= HDL_EEPROM_MSG_ERROR_BUS | HDL_EEPROM_MSG_STATUS_COMPLETE;
          eeprom->private.state = EE_STATE_IDLE;
          eeprom->private.eeprom_msg = NULL;
        }
        else if(eeprom->private.i2c_msg.status & HDL_I2C_MESSAGE_STATUS_NACK) {
          eeprom->private.eeprom_msg->state |= HDL_EEPROM_MSG_ERROR_NO_RESPONSE | HDL_EEPROM_MSG_STATUS_COMPLETE;
          eeprom->private.state = EE_STATE_IDLE;
          eeprom->private.eeprom_msg = NULL;
        }
        else {
          eeprom->private.i2c_msg.address = eeprom->config->chip_address;
          eeprom->private.i2c_msg.buffer = (eeprom->private.eeprom_msg->buffer + eeprom->private.eeprom_msg->transferred);
          eeprom->private.i2c_msg.length = eeprom->private.eeprom_msg->size - eeprom->private.eeprom_msg->transferred;
          eeprom->private.i2c_msg.options = HDL_I2C_MESSAGE_STOP;
          if(!(eeprom->private.eeprom_msg->state & HDL_EEPROM_MSG_OPTION_WRITE)) {
            eeprom->private.i2c_msg.options |= HDL_I2C_MESSAGE_START | HDL_I2C_MESSAGE_ADDR | HDL_I2C_MESSAGE_MRSW | HDL_I2C_MESSAGE_NACK_LAST;
          }
          else {
            if(eeprom->private.i2c_msg.length > eeprom->config->block_size) {
              eeprom->private.i2c_msg.length = eeprom->config->block_size;
            }
          }
          eeprom->private.state = EE_STATE_TX_DATA_MSG;
        }
      }
      break;
    }

    case EE_STATE_TX_DATA_MSG: {
      hdl_i2c_t *bus = (hdl_i2c_t *)eeprom->module.dependencies[0]; 
      if(hdl_i2c_transfer_message(bus, &eeprom->private.i2c_msg)) eeprom->private.state = EE_STATE_AWAIT_DATA_MSG;
      break;
    }

    case EE_STATE_AWAIT_DATA_MSG: {
      if(eeprom->private.i2c_msg.status & HDL_I2C_MESSAGE_STATUS_COMPLETE) {
        hdl_eeprom_i2c_data_t *data = eeprom->private.eeprom_msg;
        data->transferred += eeprom->private.i2c_msg.transferred;
        eeprom->private.state = EE_STATE_IDLE;
        eeprom->private.eeprom_msg = NULL;
        data->state |= HDL_EEPROM_MSG_STATUS_COMPLETE;
        if(eeprom->private.i2c_msg.status & (HDL_I2C_MESSAGE_FAULT_ARBITRATION_LOST | HDL_I2C_MESSAGE_FAULT_BUS_ERROR | HDL_I2C_MESSAGE_FAULT_BAD_STATE)) {
          data->state |= HDL_EEPROM_MSG_ERROR_BUS;
          break;
        }
        if(eeprom->private.i2c_msg.status & HDL_I2C_MESSAGE_STATUS_NACK) {
          data->state |= HDL_EEPROM_MSG_ERROR_NACK;
          break;
        }
        if(data->state & HDL_EEPROM_MSG_OPTION_WRITE) {
          eeprom->private.state = EE_STATE_AWAIT_BURNING;
          eeprom->private.eeprom_msg = data;
          data->state &= ~HDL_EEPROM_MSG_STATUS_COMPLETE;
          hdl_time_counter_t *time_cnt = (hdl_time_counter_t *)eeprom->module.dependencies[1];
          eeprom->private.burn_time = hdl_time_counter_get(time_cnt);
        }
      }
      break;
    }

    case EE_STATE_AWAIT_BURNING: {
      hdl_time_counter_t *time_cnt = (hdl_time_counter_t *)eeprom->module.dependencies[1];
      uint32_t now = hdl_time_counter_get(time_cnt);
      if(TIME_ELAPSED(eeprom->private.burn_time, eeprom->config->write_time, now)) {
        if(eeprom->private.eeprom_msg->transferred < eeprom->private.eeprom_msg->size) {
          eeprom->private.state = EE_STATE_SET_MEM_ADDR_MSG;
        }
        else {
          eeprom->private.eeprom_msg->state |= HDL_EEPROM_MSG_STATUS_COMPLETE;
          eeprom->private.state = EE_STATE_IDLE;
          eeprom->private.eeprom_msg = NULL;
        }
      }
      break;
    }

    case EE_STATE_IDLE:
    default:
      if(eeprom->private.eeprom_msg != NULL) {
        eeprom->private.state = EE_STATE_SET_MEM_ADDR_MSG;
      }
      break;
  }
  return cancel;
}

hdl_module_state_t hdl_eeprom_i2c(void *desc, uint8_t enable) {
  hdl_eeprom_i2c_private_t *eeprom = (hdl_eeprom_i2c_private_t *)desc;
  if(enable) {
    coroutine_add(&eeprom->private.worker, &_eeprom_worker, (void*)eeprom);
    return HDL_MODULE_ACTIVE;
  }
  coroutine_cancel(&eeprom->private.worker);
  return HDL_MODULE_UNLOADED;
}

static uint8_t _hdl_eeprom_i2c_transfer(hdl_eeprom_i2c_private_t *eeprom, hdl_eeprom_i2c_data_t *data) {
  data->state |= HDL_EEPROM_MSG_STATUS_PROCESSING;
  data->transferred = 0;
  eeprom->private.eeprom_msg = data;
  return HDL_TRUE;
}

static hdl_eeprom_i2c_private_t *_hdl_eeprom_i2c_check(hdl_eeprom_i2c_private_t *eeprom, hdl_eeprom_i2c_data_t *data) {
  if((data == NULL) || (data->size == 0) || (data->buffer == NULL) || 
     (eeprom == NULL) || (hdl_state(&eeprom->module) == HDL_MODULE_FAULT) || (eeprom->private.eeprom_msg != NULL))
    return NULL;
  return eeprom;
}

uint8_t hdl_eeprom_i2c_read(hdl_eeprom_i2c_t *desc, hdl_eeprom_i2c_data_t *data) {
  hdl_eeprom_i2c_private_t *eeprom = _hdl_eeprom_i2c_check((hdl_eeprom_i2c_private_t *)desc, data);
  if(eeprom == NULL) return HDL_FALSE;
  data->state = HDL_EEPROM_MSG_OPTION_READ;
  return _hdl_eeprom_i2c_transfer(eeprom, data);
}

uint8_t hdl_eeprom_i2c_write(hdl_eeprom_i2c_t *desc, hdl_eeprom_i2c_data_t *data) {
  hdl_eeprom_i2c_private_t *eeprom = _hdl_eeprom_i2c_check((hdl_eeprom_i2c_private_t *)desc, data);
  if(eeprom == NULL) return HDL_FALSE;
  data->state = HDL_EEPROM_MSG_OPTION_WRITE;
  return _hdl_eeprom_i2c_transfer(eeprom, data);
}

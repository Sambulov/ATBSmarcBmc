#include "hdl_portable.h"
#include "CodeLib.h"

#define WRK_STATE_START    0
#define WRK_STATE_ADDR     1
#define WRK_STATE_DATA     2
#define WRK_STATE_STOP     3
#define WRK_STATE_COMPLETE 4
#define HDL_I2C_MESSAGE_FAULT_MASK 0x7F00

#define I2C_ERROR_CLEAR_MASK (uint32_t)(I2C_STAT0_SMBALT | I2C_STAT0_SMBTO | I2C_STAT0_PECERR | \
                                        I2C_STAT0_OUERR | I2C_STAT0_AERR | I2C_STAT0_LOSTARB |  \
                                        I2C_STAT0_BERR)   

typedef struct {
  __IO uint32_t CTL0;            /*!< I2C Control register 0,                Address offset: 0x00 */
  __IO uint32_t CTL1;            /*!< I2C Control register 1,                Address offset: 0x04 */
  __IO uint32_t SADDR0;          /*!< I2C Slave address register 0,          Address offset: 0x08 */
  __IO uint32_t SADDR1;          /*!< I2C Slave address register 1,          Address offset: 0x0C */
  __IO uint32_t DATA;            /*!< I2C Transfer buffer register,          Address offset: 0x10 */
  __IO uint32_t STAT0;           /*!< I2C Transfer status register 0,        Address offset: 0x14 */
  __IO uint32_t STAT1;           /*!< I2C Transfer status register 1,        Address offset: 0x18 */
  __IO uint32_t CKCFG;           /*!< I2C Clock configure register,          Address offset: 0x1C */
  __IO uint32_t RT;              /*!< I2C Rise time register,                Address offset: 0x20*/
  __IO uint32_t _dummy24_7C[23]; 
  __IO uint32_t SAMCS;           /*!< I2C SAM control and status register,   Address offset: 0x80 */
  __IO uint32_t _dummy84_8C[3];
  __IO uint32_t FMPCFG;          /*!< I2C Fast mode plus configure register, Address offset: 0x90 */
} i2c_periph_t;

typedef struct {
  hdl_module_t module;
  hdl_i2c_config_t *config;
  struct {
    hdl_delegate_t ev_isr;
    hdl_delegate_t er_isr;
    coroutine_t i2c_worker;
    hdl_i2c_message_t *message;
    uint32_t wc_timeout;
    uint32_t wc_ts;
    __IO uint32_t *wc_reg;
    uint32_t wc_flags;
    uint8_t wc_flags_is_set;
    int8_t wc_state;
    uint8_t wrk_state;
    uint8_t wrk_state_substate;
    hdl_transceiver_t *transceiver;
    uint8_t is_master;
  } private;  
} hdl_i2c_private_t;

HDL_ASSERRT_STRUCTURE_CAST(hdl_i2c_private_t, hdl_i2c_t, HDL_I2C_PRV_SIZE, hdl_i2c.h);

_Static_assert((offsetof(hdl_i2c_private_t, module) == offsetof(hdl_i2c_t, module)) ||
               (offsetof(hdl_i2c_private_t, config) == offsetof(hdl_i2c_t, config)), "In hdl_i2c.h hdl_i2c_t properties order doesn't match");

static void _i2c_clear_error(i2c_periph_t *i2c_periph) {
  i2c_periph->STAT0 &= ~(I2C_ERROR_CLEAR_MASK);
}

static void event_i2c_ev_isr(uint32_t event, void *sender, void *context) {
  hdl_i2c_private_t *i2c = (hdl_i2c_private_t *)context;
  i2c_periph_t *i2c_periph = (i2c_periph_t *)i2c->module.reg;
  uint32_t tmp = i2c_periph->STAT0;
  uint32_t tmp1 = 0;
  uint8_t data = 0;
  /* START condition */
  if(tmp & I2C_STAT0_SBSEND) { 
    i2c_periph->CTL0 |= (I2C_CTL0_ACKEN);
  }
  if(tmp & I2C_STAT0_ADDSEND) {
    /* Clear bit ADDSEND*/
    tmp1 = i2c_periph->STAT1;
    if((i2c->private.transceiver == NULL) || (i2c->private.transceiver->rx_data == NULL) || 
      ((i2c->private.transceiver->rx_available != NULL) && (i2c->private.transceiver->rx_available(i2c->private.transceiver->proto_context) == 0))) {
      i2c_periph->CTL0 &= ~(I2C_CTL0_ACKEN);
    }
  }
  /* Read from slave */
  if(tmp & I2C_STAT0_TBE) {
    if((i2c->private.transceiver != NULL) && (i2c->private.transceiver->tx_empty != NULL)) {
        i2c->private.transceiver->tx_empty(i2c->private.transceiver->proto_context, &data, 1);
        i2c_periph->DATA = data;
    }
  }
  /* Write to slave */
  if(tmp & I2C_STAT0_RBNE) {
    data = i2c_periph->DATA;
    if((i2c->private.transceiver == NULL) || (i2c->private.transceiver->rx_data == NULL) || 
      ((i2c->private.transceiver->rx_available != NULL) && (i2c->private.transceiver->rx_available(i2c->private.transceiver->proto_context) == 0))) {
      i2c_periph->CTL0 &= ~(I2C_CTL0_ACKEN);
    }
    else {
      i2c->private.transceiver->rx_data(i2c->private.transceiver->proto_context, &data, 1);
    }
  }
  /* STOP condition */
  if(tmp & I2C_STAT0_STPDET) { 
    if((i2c->private.transceiver != NULL) && (i2c->private.transceiver->end_of_transmission != NULL))
      i2c->private.transceiver->end_of_transmission(i2c->private.transceiver->proto_context);
    i2c_periph->CTL0 |= (I2C_CTL0_ACKEN);
  }
}

static void event_i2c_er_isr(uint32_t event, void *sender, void *context) {
  hdl_i2c_private_t *i2c = (hdl_i2c_private_t *)context;
  i2c_periph_t *i2c_periph = (i2c_periph_t *)i2c->module.reg;
  if(i2c_periph->STAT0 & I2C_STAT0_AERR) {
    if((i2c->private.transceiver != NULL) && (i2c->private.transceiver->end_of_transmission != NULL))
      i2c->private.transceiver->end_of_transmission(i2c->private.transceiver->proto_context);
    i2c_periph->CTL0 |= (I2C_CTL0_ACKEN);
  }
  _i2c_clear_error(i2c_periph);
}

#define WC_STATE_AWAITING    0
#define WC_STATE_TIMEOUT    -1
#define WC_STATE_HIT         1

static void _i2c_reg_set_wait_condition(hdl_i2c_private_t *i2c, __IO uint32_t *reg, uint32_t flags, uint8_t is_set, uint32_t timeout) {
  hdl_time_counter_t *timer = (hdl_time_counter_t *)i2c->module.dependencies[4];
  i2c->private.wc_ts = hdl_time_counter_get(timer);
  i2c->private.wc_flags = flags;
  i2c->private.wc_flags_is_set = is_set;
  i2c->private.wc_timeout = timeout;
  i2c->private.wc_reg = reg;
  i2c->private.wc_state = WC_STATE_AWAITING;
}

static void _i2c_reg_wait_condition(hdl_i2c_private_t *i2c) {
  if(i2c->private.wc_state == WC_STATE_AWAITING) {
    hdl_time_counter_t *timer = (hdl_time_counter_t *)i2c->module.dependencies[4];
    uint32_t now = hdl_time_counter_get(timer);
    if(i2c->private.wc_flags_is_set) {
      if((*i2c->private.wc_reg & i2c->private.wc_flags) == i2c->private.wc_flags) i2c->private.wc_state = WC_STATE_HIT;
    }
    else {
      uint32_t flags = ~i2c->private.wc_flags;
      if((*i2c->private.wc_reg | flags) == flags) i2c->private.wc_state = WC_STATE_HIT;
    }
    if(TIME_ELAPSED(i2c->private.wc_ts, i2c->private.wc_timeout, now)) i2c->private.wc_state = WC_STATE_TIMEOUT;
  }
}

static void _i2c_set_bus_to_default(i2c_periph_t *i2c_periph) {
  uint32_t tmp = 0;
  i2c_periph->CTL0 &= ~I2C_CTL0_ACKEN;
  tmp = i2c_periph->STAT0;
  tmp = i2c_periph->STAT1; 
  while (i2c_periph->STAT0 & I2C_STAT0_RBNE)
    tmp = i2c_periph->DATA;
}

static uint8_t _i2c_msg_start_handler(hdl_i2c_private_t *i2c) {
  i2c_periph_t *i2c_periph = (i2c_periph_t *)i2c->module.reg;
  if(i2c->private.wrk_state_substate == 1) {
    if(i2c->private.wc_state == WC_STATE_TIMEOUT) {
      i2c_periph->CTL0 &= ~I2C_CTL0_START;
      i2c->private.message->status |= HDL_I2C_MESSAGE_FAULT_BUS_ERROR;
    }
    else
      i2c->private.message->status |= HDL_I2C_MESSAGE_STATUS_START_ON_BUS;
    return HDL_TRUE;
  }
  if(i2c->private.wrk_state_substate == 0) {
    i2c_periph->CTL1 &= ~(I2C_CTL1_BUFIE | I2C_CTL1_EVIE | I2C_CTL1_ERRIE);
    i2c_ack_config(((uint32_t)i2c->module.reg), I2C_ACK_DISABLE);
    i2c_periph->CTL0 &= ~(I2C_CTL0_SS | I2C_CTL0_STOP);
    //_i2c_set_bus_to_default(i2c_periph);
    /* send a start condition to I2C bus */
    i2c_periph->CTL0 |= I2C_CTL0_START;
    /* wait until SBSEND bit is set */
    _i2c_reg_set_wait_condition(i2c, &i2c_periph->STAT0, I2C_STAT0_SBSEND, 1, 10);
    i2c->private.wrk_state_substate = 1;
  }
  return HDL_FALSE;
}

static hdl_i2c_message_status_t _i2c_msg_addr_handler(hdl_i2c_private_t *i2c) {
  hdl_time_counter_t *timer = (hdl_time_counter_t *)i2c->module.dependencies[4];
  i2c_periph_t *i2c_periph = (i2c_periph_t *)i2c->module.reg;
  if(i2c->private.wrk_state_substate == 1) {
    if(i2c_periph->STAT0 & I2C_STAT0_AERR) {
      i2c->private.message->status |= HDL_I2C_MESSAGE_STATUS_NACK;
    }
    i2c->private.message->status |= (i2c->private.wc_state == WC_STATE_TIMEOUT)? HDL_I2C_MESSAGE_FAULT_BUS_ERROR : HDL_I2C_MESSAGE_STATUS_ADDR_SENT;
    return HDL_TRUE;
  }
  if(i2c->private.wrk_state_substate == 0) {
    if(!(i2c_periph->STAT0 & I2C_STAT0_SBSEND)) { 
      i2c->private.message->status |= HDL_I2C_MESSAGE_FAULT_BAD_STATE;
      return HDL_TRUE;
    }
    /* send slave address */
    i2c_periph->DATA = (i2c->private.message->address << 1) | ((i2c->private.message->options & HDL_I2C_MESSAGE_MRSW)? I2C_RECEIVER: 0);
    /* wait until ADDSEND bit is set */
    _i2c_reg_set_wait_condition(i2c, &i2c_periph->STAT0, I2C_STAT0_ADDSEND, 1, 10);
    i2c->private.wrk_state_substate = 1;
  }
  return HDL_FALSE;
}

static uint8_t _i2c_msg_data_receiver_handler(hdl_i2c_private_t *i2c) {
  hdl_time_counter_t *timer = (hdl_time_counter_t *)i2c->module.dependencies[4];
  i2c_periph_t *i2c_periph = (i2c_periph_t *)i2c->module.reg;
  uint32_t tmp = 0;
  if(i2c->private.wrk_state_substate == 1) { /* Awaiting rx byte */
    if(i2c->private.wc_state == WC_STATE_TIMEOUT) {
      i2c->private.message->status |= HDL_I2C_MESSAGE_FAULT_BAD_STATE | HDL_I2C_MESSAGE_FAULT_BUS_ERROR;
      return HDL_TRUE;
    }
    i2c->private.wrk_state_substate = 2;
  }

  if(i2c->private.wrk_state_substate == 2) {
    if(i2c_periph->STAT0 & I2C_STAT0_RBNE) {
      if((i2c->private.message->transferred == (i2c->private.message->length - 3)) && !(i2c->private.message->options & HDL_I2C_MESSAGE_NACK_LAST)) {
        return HDL_TRUE;
      }
      i2c->private.message->buffer[i2c->private.message->transferred] = i2c_periph->DATA;
      i2c->private.message->transferred++;
      if((i2c->private.message->transferred == (i2c->private.message->length - 2)) && (i2c->private.message->options & HDL_I2C_MESSAGE_NACK_LAST)) {
        i2c->private.message->status |= HDL_I2C_MESSAGE_STATUS_NACK;
        i2c_periph->CTL0 &= ~I2C_CTL0_ACKEN;
      }
      i2c->private.message->status |= HDL_I2C_MESSAGE_STATUS_DATA;
      if(i2c->private.message->transferred == i2c->private.message->length) {
        return HDL_TRUE;
      }
    }
    _i2c_reg_set_wait_condition(i2c, &i2c_periph->STAT0, I2C_STAT0_RBNE, 1, 10);
    i2c->private.wrk_state_substate = 1;
  }
  if(i2c->private.wrk_state_substate == 0) {
    i2c->private.message->transferred = 0;
    if(i2c_periph->STAT0 & I2C_STAT0_ADDSEND) {
      i2c_periph->CTL0 |= I2C_CTL0_ACKEN; 
      if((i2c->private.message->options & HDL_I2C_MESSAGE_NACK_LAST) || (i2c->private.message->length > 3)) { 
        // Clear I2C_STAT0_ADDSEND
        tmp = i2c_periph->STAT0;
        tmp = i2c_periph->STAT1;
        _i2c_reg_set_wait_condition(i2c, &i2c_periph->STAT0, I2C_STAT0_RBNE, 1, 10);
      }
      else { 
        i2c->private.message->status |= HDL_I2C_MESSAGE_STATUS_DATA;
        return HDL_TRUE;
      }
    }
    _i2c_reg_set_wait_condition(i2c, &i2c_periph->STAT0, I2C_STAT0_RBNE, 1, 10);
    i2c->private.wrk_state_substate = 1;
  }
  return HDL_FALSE;
}

static uint8_t _i2c_msg_data_transmitter_handler(hdl_i2c_private_t *i2c) {
  i2c_periph_t *i2c_periph = (i2c_periph_t *)i2c->module.reg;
  if(i2c->private.wrk_state_substate == 0) {
    i2c->private.message->transferred = 0;
    if(!(i2c_periph->STAT1 & I2C_STAT1_TR)) {
      i2c->private.message->status |= HDL_I2C_MESSAGE_FAULT_BAD_STATE;
      return HDL_TRUE;
    }
    i2c->private.wrk_state_substate = 2;
  }

  if(i2c->private.wrk_state_substate == 1) { /* Awaiting TBE */
    if(i2c_periph->STAT0 & I2C_STAT0_AERR) {
      i2c->private.message->status |= HDL_I2C_MESSAGE_STATUS_NACK;
      return HDL_TRUE;
    }
    if(i2c->private.wc_state == WC_STATE_TIMEOUT) {
      i2c->private.message->status |= HDL_I2C_MESSAGE_FAULT_BUS_ERROR;
      return HDL_TRUE;
    }
    i2c->private.wrk_state_substate = 2;
  }

  if(i2c->private.wrk_state_substate == 2) {
    while((i2c_periph->STAT0 & I2C_STAT0_TBE) && (i2c->private.message->transferred < i2c->private.message->length)) {
      i2c_periph->DATA = i2c->private.message->buffer[i2c->private.message->transferred];
      i2c->private.message->transferred++;
    }
    if(i2c->private.message->transferred == i2c->private.message->length) {
      i2c->private.message->status |= HDL_I2C_MESSAGE_STATUS_DATA;
      return HDL_TRUE;
    }
    else {
      _i2c_reg_set_wait_condition(i2c, &i2c_periph->STAT0, I2C_STAT0_TBE, 1, 10);
      i2c->private.wrk_state_substate = 1;
    }
  }
  return HDL_FALSE;
}


static uint8_t _i2c_msg_data_handler(hdl_i2c_private_t *i2c) {
  if(i2c->private.message->options & HDL_I2C_MESSAGE_MRSW) {
    return _i2c_msg_data_receiver_handler(i2c);
  }
  else {
    return _i2c_msg_data_transmitter_handler(i2c);
  }
}

static hdl_i2c_message_status_t _i2c_msg_stop_handler(hdl_i2c_private_t *i2c) {
  hdl_time_counter_t *timer = (hdl_time_counter_t *)i2c->module.dependencies[4];
  i2c_periph_t *i2c_periph = (i2c_periph_t *)i2c->module.reg;
  hdl_i2c_message_status_t result = HDL_I2C_MESSAGE_STATUS_INITIAL;
  if(i2c->private.wrk_state_substate == 1) {
    i2c->private.wrk_state_substate = 2;
  }
  if(i2c->private.wrk_state_substate == 0) {
    if(i2c_periph->STAT1 & I2C_STAT1_TR) {
      _i2c_reg_set_wait_condition(i2c, &i2c_periph->STAT0, I2C_STAT0_BTC, 1, 10);
      i2c->private.wrk_state_substate = 1;
    }
    else {
      i2c->private.wrk_state_substate = 2;
    }
  }
  if(i2c->private.wrk_state_substate == 2) {
    _i2c_set_bus_to_default(i2c_periph);
    if(i2c_periph->STAT1 & I2C_STAT1_MASTER) {
      i2c_periph->CTL0 |= I2C_CTL0_STOP;
      i2c->private.message->status |= HDL_I2C_MESSAGE_STATUS_STOP_ON_BUS;
    }
    else 
      i2c->private.message->status |= HDL_I2C_MESSAGE_FAULT_BAD_STATE;
    if(!(i2c->config->stretch_enable))
      i2c_periph->CTL0 |= I2C_SCLSTRETCH_DISABLE;
    i2c_ack_config(((uint32_t)i2c->module.reg), ((i2c->config->addr0 == 0) ? I2C_ACK_DISABLE : I2C_ACK_ENABLE));
    i2c_periph->CTL1 |= (I2C_CTL1_BUFIE | I2C_CTL1_EVIE | I2C_CTL1_ERRIE);
    i2c->private.is_master = HDL_FALSE;
    return HDL_TRUE;
  }
  return HDL_FALSE;
}



static uint8_t _i2c_client_worker(coroutine_t *this, uint8_t cancel, void *arg) {
  hdl_i2c_private_t *i2c = (hdl_i2c_private_t *) arg;
  if(i2c->private.message != NULL) {
    if(i2c->private.wc_state == WC_STATE_AWAITING) {
      _i2c_reg_wait_condition(i2c);
    } else {
      if(i2c->private.wrk_state == WRK_STATE_START)
        if(!(i2c->private.message->options & HDL_I2C_MESSAGE_START) || _i2c_msg_start_handler(i2c)) { i2c->private.wrk_state++; i2c->private.wrk_state_substate = 0; }
      if(i2c->private.message->status & HDL_I2C_MESSAGE_FAULT_MASK) i2c->private.wrk_state = WRK_STATE_STOP;
      else i2c->private.is_master = HDL_TRUE;

      if(i2c->private.wrk_state == WRK_STATE_ADDR)
        if(!(i2c->private.message->options & HDL_I2C_MESSAGE_ADDR) || _i2c_msg_addr_handler(i2c)) { i2c->private.wrk_state++; i2c->private.wrk_state_substate = 0; }
      if(i2c->private.message->status & HDL_I2C_MESSAGE_FAULT_MASK) i2c->private.wrk_state = WRK_STATE_STOP;

      if(i2c->private.wrk_state == WRK_STATE_DATA)
        if(!((i2c->private.message->length > 0) && (i2c->private.message->buffer != NULL)) || _i2c_msg_data_handler(i2c)) { i2c->private.wrk_state++; i2c->private.wrk_state_substate = 0; }
      if(i2c->private.message->status & HDL_I2C_MESSAGE_FAULT_MASK) i2c->private.wrk_state = WRK_STATE_STOP;

      if(i2c->private.wrk_state == WRK_STATE_STOP)
        if(!(i2c->private.message->options & HDL_I2C_MESSAGE_STOP) || _i2c_msg_stop_handler(i2c)) { i2c->private.wrk_state++; i2c->private.wrk_state_substate = 0; }

      if(i2c->private.wrk_state == WRK_STATE_COMPLETE) {
        i2c->private.message->status |= HDL_I2C_MESSAGE_STATUS_COMPLETE;
        i2c->private.message = NULL;
      }
    }
  }
  return cancel;
}

uint8_t hdl_i2c_transfer_message(hdl_i2c_t *i2c, hdl_i2c_message_t *message) {
  hdl_i2c_private_t *_i2c = (hdl_i2c_private_t *)i2c;
  if((message != NULL) && (i2c != NULL) && (hdl_state(&i2c->module) == HDL_MODULE_ACTIVE) && (_i2c->private.message == NULL)) {
    message->status = 0;
    message->transferred = 0;
    _i2c->private.message = message;
    _i2c->private.wrk_state = WRK_STATE_START;
    return HDL_TRUE;
  }
  return HDL_FALSE;
}

hdl_module_state_t hdl_i2c(void *i2c, uint8_t enable) {
  hdl_i2c_private_t *_i2c = (hdl_i2c_private_t *)i2c;
  i2c_periph_t *i2c_periph = (i2c_periph_t *)_i2c->module.reg;
  i2c_deinit((uint32_t)_i2c->module.reg);
  if(enable) {
    rcu_periph_clock_enable(_i2c->config->rcu);
    i2c_clock_config((uint32_t)_i2c->module.reg, _i2c->config->speed, _i2c->config->dtcy);
    i2c_mode_addr_config((uint32_t)_i2c->module.reg, I2C_I2CMODE_ENABLE, 
      (_i2c->config->addr_10_bits? I2C_ADDFORMAT_10BITS: I2C_ADDFORMAT_7BITS), (_i2c->config->addr0 << 1));
    i2c_ackpos_config((uint32_t)_i2c->module.reg, I2C_ACKPOS_CURRENT);
    i2c_periph->SADDR1 = ((_i2c->config->addr1 << 1) & 0xFE);
    i2c_periph->SADDR1 |= (_i2c->config->dual_address? I2C_SADDR1_DUADEN : 0);
    i2c_periph->CTL0 |= 
      (_i2c->config->general_call_enable? I2C_GCEN_ENABLE : I2C_GCEN_DISABLE) |
      (_i2c->config->stretch_enable? I2C_SCLSTRETCH_ENABLE : I2C_SCLSTRETCH_DISABLE) |
      I2C_I2CMODE_ENABLE;   
    _i2c->private.ev_isr.context = i2c;
    _i2c->private.ev_isr.handler = &event_i2c_ev_isr;
    _i2c->private.er_isr.context = i2c;
    _i2c->private.er_isr.handler = &event_i2c_er_isr; 
    hdl_interrupt_controller_t *ic = (hdl_interrupt_controller_t *)_i2c->module.dependencies[3];
    hdl_interrupt_request(ic, _i2c->config->err_interrupt, &_i2c->private.er_isr);
    hdl_interrupt_request(ic, _i2c->config->ev_interrupt, &_i2c->private.ev_isr);

    i2c_periph->CTL1 |= (I2C_CTL1_BUFIE | I2C_CTL1_EVIE | I2C_CTL1_ERRIE);
    i2c_periph->CTL0 |= I2C_CTL0_I2CEN;
    i2c_ack_config(((uint32_t)_i2c->module.reg), ((_i2c->config->addr0 == 0) ? I2C_ACK_DISABLE : I2C_ACK_ENABLE));
    _i2c->private.wc_state = WC_STATE_HIT;
    coroutine_add(&_i2c->private.i2c_worker, &_i2c_client_worker, (void*)_i2c);
    return HDL_MODULE_ACTIVE;
  }
  coroutine_cancel(&_i2c->private.i2c_worker);
  rcu_periph_clock_disable(_i2c->config->rcu);
  return HDL_MODULE_UNLOADED;
}

void hdl_i2c_set_transceiver(hdl_i2c_t *i2c, hdl_transceiver_t *transceiver) {
  if(i2c != NULL) {
    ((hdl_i2c_private_t*)i2c)->private.transceiver = transceiver;
  }
}

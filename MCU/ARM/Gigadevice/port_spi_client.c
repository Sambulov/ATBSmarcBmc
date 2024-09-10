#include "hdl_portable.h"
#include "CodeLib.h"

typedef struct {
  hdl_module_t module;
  /* private */
  coroutine_t ch_worker;
  hdl_spi_message_t *curent_msg;
} hdl_spi_client_ch_private_t;

typedef struct {
  hdl_module_t module;
  hdl_spi_client_config_t *config;
  /* private */
  hdl_delegate_t spi_isr;
  hdl_spi_client_ch_private_t *curent_spi_ch;
  uint16_t rx_cursor;
  uint16_t tx_cursor;
} hdl_spi_client_private_t;

_Static_assert(sizeof(hdl_spi_client_private_t) == sizeof(hdl_spi_client_t), "In port_spi.h data structure size of hdl_spi_client_t doesn't match, check HDl_SPI_CLIENT_PRIVATE_SIZE");
_Static_assert(sizeof(hdl_spi_client_ch_private_t) == sizeof(hdl_spi_client_ch_t), "In port_spi.h data structure size of hdl_spi_client_ch_t doesn't match, check HDl_SPI_CLIENT_CH_PRIVATE_SIZE");

static void event_spi_isr_client(uint32_t event, void *sender, void *context) {
  hdl_spi_client_private_t *spi = (hdl_spi_client_private_t *)context;
  hdl_spi_message_t *msg = spi->curent_spi_ch->curent_msg;

  uint32_t msg_len = msg->rx_skip + msg->rx_take;
  msg_len = MAX(msg->tx_len, msg_len);

  uint32_t state = SPI_STAT((uint32_t)spi->module.reg);
  if ((state & (SPI_ERROR_MASK)) == 0) {
    /* RX ---------------------------------------------------*/
    if (state & SPI_STAT_RBNE) {
      uint16_t data = SPI_DATA((uint32_t)spi->module.reg);
      if(msg->rx_buffer != NULL) {
        int32_t data_offset = ((int32_t)spi->rx_cursor) - msg->rx_skip;
        if((data_offset >= 0) && (data_offset < msg->rx_take)) msg->rx_buffer[data_offset] = data;
      }
      spi->rx_cursor++;
      if(spi->rx_cursor >= msg_len) {
        SPI_CTL1((uint32_t)spi->module.reg) &= ~SPI_CTL1_RBNEIE;
        msg->state |= HDL_SPI_MESSAGE_STATUS_XFER_COMPLETE;
      }
    }

    ///* TX ------------------------------------------------*/
    if((state & SPI_STAT_TBE) && (spi->tx_cursor < msg_len)) {
      uint16_t data = 0;
      if ((msg->tx_buffer != NULL) && (msg->tx_len > 0)) {
        if (spi->tx_cursor < msg->tx_len) {
          data = msg->tx_buffer[spi->tx_cursor];
        }
        else {
          data = msg->tx_buffer[msg->tx_len - 1];
        }
      }
      SPI_DATA((uint32_t)spi->module.reg) = data;
      spi->tx_cursor++;
      if(spi->tx_cursor >= msg_len) {
        SPI_CTL1((uint32_t)spi->module.reg) &= ~SPI_CTL1_TBEIE;
      }
    }
    msg->transfered = MIN(spi->rx_cursor, spi->tx_cursor);
  }
  else {
    hdl_spi_reset_status((uint32_t)spi->module.reg);
  }
}

static uint8_t _spi_ch_worker(coroutine_t *this, uint8_t cancel, void *arg) {
  hdl_spi_client_ch_private_t *ch = (hdl_spi_client_ch_private_t *) arg;
  hdl_spi_client_private_t *spi = (hdl_spi_client_private_t *)ch->module.dependencies[0];
  if((spi->curent_spi_ch == NULL) && (ch->curent_msg != NULL)) {
    spi->curent_spi_ch = ch;
  }
  if (spi->curent_spi_ch == ch) {
    hdl_gpio_pin_t *pin_cs = (hdl_gpio_pin_t *)ch->module.dependencies[1];
    hdl_spi_message_t *msg = ch->curent_msg;
    if(msg != NULL) {
      if (msg->state == HDL_SPI_MESSAGE_STATUS_INITIAL) {
        //TODO: cs delay
        if(msg->options & HDL_SPI_MESSAGE_CH_SELECT) {
          hdl_gpio_set_active(pin_cs);
          msg->state |= HDL_SPI_MESSAGE_STATUS_BUS_HOLD;
        }
        uint32_t msg_len = msg->rx_skip + msg->rx_take;
        msg_len = MAX(msg->tx_len, msg_len);
        if(msg_len > 0) {
          spi->rx_cursor = 0;
          spi->tx_cursor = 0;
          hdl_spi_reset_status((uint32_t)spi->module.reg);
          msg->state |= HDL_SPI_MESSAGE_STATUS_XFER;
          SPI_CTL1((uint32_t)spi->module.reg) |= (SPI_CTL1_TBEIE | SPI_CTL1_RBNEIE);
        }
        else {
          msg->state |= HDL_SPI_MESSAGE_STATUS_XFER_COMPLETE;
        }
      }
      else if(msg->state & HDL_SPI_MESSAGE_STATUS_XFER_COMPLETE) {
        ch->curent_msg = NULL;
        if(!(msg->options & HDL_SPI_MESSAGE_CH_RELEASE)) {
          hdl_gpio_set_inactive(pin_cs);
          msg->state |= HDL_SPI_MESSAGE_STATUS_BUS_RELEASE;
          spi->curent_spi_ch = NULL;
        }
        msg->state |= HDL_SPI_MESSAGE_STATUS_COMPLETE;
      }
    }
  }
  return cancel;
}

hdl_module_state_t hdl_spi_client(void *desc, uint8_t enable) {
  hdl_spi_client_private_t *spi = (hdl_spi_client_private_t*)desc;
  spi_i2s_deinit((uint32_t)spi->module.reg);
  if(enable) {
    rcu_periph_clock_enable(spi->config->rcu);
    spi_parameter_struct init;
    init.device_mode = SPI_MASTER;
    init.trans_mode = SPI_TRANSMODE_FULLDUPLEX;
    init.frame_size = SPI_FRAMESIZE_8BIT;
    init.endian = spi->config->endian;
    init.clock_polarity_phase = spi->config->polarity;
    init.nss = SPI_NSS_SOFT;
    init.prescale = spi->config->prescale;
    spi_init((uint32_t)spi->module.reg, &init);
    //SPI_CTL1((uint32_t)spi->module.reg) |= SPI_CTL1_RBNEIE;
    //SPI_CTL1((uint32_t)spi->module.reg) |= SPI_CTL1_RBNEIE | SPI_CTL1_TBEIE | SPI_CTL1_ERRIE;
    //SPI_CTL0((uint32_t)spi->module.reg) |= SPI_CTL0_SWNSS; 
    spi->spi_isr.context = desc;
    spi->spi_isr.handler = &event_spi_isr_client;
    hdl_interrupt_controller_t *ic = (hdl_interrupt_controller_t *)spi->module.dependencies[4];
    hdl_interrupt_request(ic, spi->config->spi_interrupt, &spi->spi_isr);
    spi_enable((uint32_t)spi->module.reg);
    return HDL_MODULE_ACTIVE;
  }
  rcu_periph_clock_disable(spi->config->rcu);
  return HDL_MODULE_UNLOADED;
}

hdl_module_state_t hdl_spi_ch(void *desc, uint8_t enable) {
  hdl_spi_client_ch_private_t *spi_ch = (hdl_spi_client_ch_private_t*)desc;
  if(enable) {
    coroutine_add(&spi_ch->ch_worker, &_spi_ch_worker, desc);
    return HDL_MODULE_ACTIVE;
  }
  coroutine_cancel(&spi_ch->ch_worker);
  // TODO: free bus
  return HDL_MODULE_UNLOADED;
}

uint8_t hdl_spi_transfer_message(hdl_spi_client_ch_t *spi_ch, hdl_spi_message_t *message) {
  if((spi_ch != NULL) && (hdl_state(&spi_ch->module) == HDL_MODULE_ACTIVE) && (message != NULL)) {
    hdl_spi_client_ch_private_t *spi = (hdl_spi_client_ch_private_t *)spi_ch;
    if(spi->curent_msg == NULL) {
      spi->curent_msg = message;
      message->transfered = 0;
      message->state = HDL_SPI_MESSAGE_STATUS_INITIAL;
      return HDL_TRUE;
    }
  }
  return HDL_FALSE;
}

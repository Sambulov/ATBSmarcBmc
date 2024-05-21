#include "hdl_portable.h"
#include "CodeLib.h"

#define SPI_SERVER_RX_BUFFER_SZ    32
#define SPI_SERVER_TX_BUFFER_SZ    32

typedef struct {
  hdl_module_t module;
  hdl_spi_server_config_t *config;
  hdl_nvic_irq_n_t spi_iterrupt;
  hdl_nvic_irq_n_t nss_iterrupt; /* if HDL_SPI_CS_SOFT in HDL_SPI_SERVER mode */
  /* private */
  simple_circular_buffer_t rx_scb;
  simple_circular_buffer_t tx_scb;
  uint8_t rx_buf[SPI_SERVER_RX_BUFFER_SZ];
  uint8_t tx_buf[SPI_SERVER_TX_BUFFER_SZ];
  __linked_list_object__;

} hdl_spi_server_private_t;

typedef struct {
  hdl_module_t module;
  hdl_spi_client_config_t *config;
  hdl_nvic_irq_n_t spi_iterrupt;
  /* private */
  __linked_list_object__;
} hdl_spi_client_private_t;


typedef struct {
  hdl_spi_message_state_t state; 
  /* public */
  uint8_t *tx_buffer;
  uint8_t *rx_buffer;
  uint32_t tx_len;
  uint32_t rx_skip;
  uint32_t rx_take;
} hdl_spi_message_private_t;

_Static_assert(sizeof(hdl_spi_server_private_t) == sizeof(hdl_spi_server_t), "In port_spi.h data structure size of hdl_spi_server_t doesn't match, check SPI_SERVER_PRIVATE_SIZE");
_Static_assert(sizeof(hdl_spi_client_private_t) == sizeof(hdl_spi_client_t), "In port_spi.h data structure size of hdl_spi_client_t doesn't match, check SPI_CLIENT_PRIVATE_SIZE");
_Static_assert(sizeof(hdl_spi_message_private_t) == sizeof(hdl_spi_message_t), "In hdl_spi.h data structure size of hdl_spi_message_t doesn't match, check HDL_SPI_MESSAGE_PRV_SIZE");


static void event_spi_isr(uint32_t event, void *sender, void *context) {

}

static void _spi_handler(linked_list_item_t *uart_item, void *arg) {

}

static uint8_t _spi_worker(coroutine_desc_t this, uint8_t cancel, void *arg) {
  linked_list_t spis = (linked_list_t)arg;
  linked_list_do_foreach(spis, &_spi_handler, NULL);
  return cancel;
}

hdl_module_state_t hdl_spi(void *desc, uint8_t enable) {
  static coroutine_desc_static_t uart_task_buf;
  static linked_list_t spis = NULL;
  hdl_spi_private_t *spi = (hdl_spi_private_t*)desc;
  rcu_periph_enum rcu;
  switch ((uint32_t)spi->module.reg) {
    case SPI0: rcu = RCU_SPI0; break;
    case SPI1: rcu = RCU_SPI1; break;
    case SPI2: rcu = RCU_SPI2; break;
    case SPI3: rcu = RCU_SPI3; break;
    case SPI4: rcu = RCU_SPI4; break;
    case SPI5: rcu = RCU_SPI5; break;
    default: return HDL_MODULE_INIT_FAILED;
  }
  spi_i2s_deinit((uint32_t)spi->module.reg);
  if(enable) {
    rcu_periph_clock_enable(rcu);
    linked_list_insert_last(&spis, linked_list_item(spi));
    coroutine_add_static(&uart_task_buf, &_spi_worker, (void *)spis);
    spi_parameter_struct init;
    init.device_mode = spi->config->mode;
    init.trans_mode = SPI_TRANSMODE_FULLDUPLEX;
    init.frame_size = SPI_FRAMESIZE_8BIT;
    init.endian = spi->config->endian;
    init.clock_polarity_phase = spi->config->polarity;
    init.nss = spi->config->cs_mode;
    init.prescale = spi->config->prescale;
    spi_init((uint32_t)spi->module.reg, &init);
    spi->transceiver = NULL;
    hdl_interrupt_controller_t *ic = (hdl_interrupt_controller_t *)spi->module.dependencies[4];
    hdl_interrupt_request(ic, spi->spi_iterrupt, &event_spi_isr, desc);
    /* todo: enable interrupts */
    spi_enable((uint32_t)spi->module.reg);
    return HDL_MODULE_INIT_OK;
  }
  linked_list_unlink(linked_list_item(spi));
  rcu_periph_clock_disable(rcu);
  return HDL_MODULE_DEINIT_OK;
}


hdl_module_state_t hdl_spi_server(void *desc, uint8_t enable) {

}

hdl_module_state_t hdl_spi_client(void *desc, uint8_t enable) {

}


hdl_module_state_t hdl_spi_ch(void *desc, uint8_t enable) {
  if(enable) {

    return HDL_MODULE_INIT_OK;
  }
  return HDL_MODULE_DEINIT_OK;
}

hdl_spi_message_state_t hdl_spi_client_xfer(hdl_spi_ch_t *spi_ch, hdl_spi_message_t *message) {
  //TODO
  return HDL_SPI_TRANSACTION_FAILED;
}

#define spi_message_state_valid(state) (state <= HDL_SPI_TRANSACTION_FAILED)

hdl_spi_message_state_t hdl_spi_message_get_state(hdl_spi_message_t *message) {
  hdl_spi_message_private_t *msg = (hdl_spi_message_private_t *)message;
  if((msg != NULL) && spi_message_state_valid(msg->state)) {
    return msg->state;
  }
  return HDL_SPI_TRANSACTION_UNKNOWN;
}

void hdl_spi_server_set_transceiver(hdl_spi_ch_t *spi, hdl_transceiver_t *transceiver) {
  if(spi != NULL) {
    ((hdl_spi_private_t*)spi)->transceiver = transceiver;
  }
}

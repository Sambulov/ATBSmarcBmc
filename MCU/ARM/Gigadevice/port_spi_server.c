#include "hdl_portable.h"
#include "CodeLib.h"

#define SPI_SERVER_RX_BUFFER_SZ    32
#define SPI_SERVER_TX_BUFFER_SZ    32

typedef struct {
  hdl_module_t module;
  const hdl_spi_server_config_t *config;
  struct {
    hdl_delegate_t spi_isr;
    hdl_delegate_t nss_isr;
    hdl_transceiver_t *transceiver;
  } private;  
} hdl_spi_server_private_t;

HDL_ASSERRT_STRUCTURE_CAST(hdl_spi_server_private_t, hdl_spi_server_t, SPI_SERVER_PRIVATE_SIZE, port_spi.h);

static void event_spi_nss(uint32_t event, void *sender, void *context) {
  hdl_spi_server_private_t *spi = (hdl_spi_server_private_t*)context;
  hdl_gpio_pin_t *nss = (hdl_gpio_pin_t *)spi->module.dependencies[3];
  if(event & (uint32_t)nss->module.reg){
    if((spi->private.transceiver != NULL) && (spi->private.transceiver->end_of_transmission != NULL))
      spi->private.transceiver->end_of_transmission(spi->private.transceiver->proto_context);
  }
}

static void event_spi_isr_server(uint32_t event, void *sender, void *context) {
hdl_spi_server_private_t *spi = (hdl_spi_server_private_t *)context;
  uint32_t state = SPI_STAT((uint32_t)spi->module.reg);
  uint32_t cr1 = SPI_CTL1((uint32_t)spi->module.reg);
	if ((state & SPI_ERROR_MASK) == 0) {
		/* spi rx ---------------------------------------------------*/
		if (state & SPI_STAT_RBNE) {
			uint16_t data = SPI_DATA((uint32_t)spi->module.reg);
      if((spi->private.transceiver != NULL) && (spi->private.transceiver->rx_data != NULL)) {  
          spi->private.transceiver->rx_data(spi->private.transceiver->proto_context, (uint8_t *)&data, 1);
      }
		}
		/* spi tx ---------------------------------------------------*/
		if ((state & SPI_STAT_TBE) && (cr1 & SPI_CTL1_TBEIE)) {
      uint16_t data = 0;
      if((spi->private.transceiver != NULL) && (spi->private.transceiver->tx_empty != NULL)) {
        spi->private.transceiver->tx_empty(spi->private.transceiver->proto_context, (uint8_t *)&data, 1);
      }
      SPI_DATA((uint32_t)spi->module.reg) = data;
		}
	}
	else {
		hdl_spi_reset_status((uint32_t)spi->module.reg);
	}
}

hdl_module_state_t hdl_spi_server(void *desc, uint8_t enable) {
  hdl_spi_server_private_t *spi = (hdl_spi_server_private_t*)desc;
  spi_i2s_deinit((uint32_t)spi->module.reg);
  if(enable) {
    rcu_periph_clock_enable(spi->config->rcu);
    spi_parameter_struct init;
    init.device_mode = SPI_SLAVE;
    init.trans_mode = SPI_TRANSMODE_FULLDUPLEX;
    init.frame_size = SPI_FRAMESIZE_8BIT;
    init.endian = spi->config->endian;
    init.clock_polarity_phase = spi->config->polarity;
    init.prescale = HDL_SPI_PSC_2;
    init.nss = SPI_NSS_HARD;
    spi_init((uint32_t)spi->module.reg, &init);
    SPI_CTL1((uint32_t)spi->module.reg) |= SPI_CTL1_RBNEIE | SPI_CTL1_TBEIE | SPI_CTL1_ERRIE;
    //SPI_CTL0((uint32_t)spi->module.reg) |= SPI_CTL0_SWNSS; 
    spi->private.transceiver = NULL;
    spi->private.nss_isr.context = desc;
    spi->private.nss_isr.handler = &event_spi_nss;
    spi->private.spi_isr.context = desc;
    spi->private.spi_isr.handler = &event_spi_isr_server;
    hdl_interrupt_controller_t *ic = (hdl_interrupt_controller_t *)spi->module.dependencies[5];
    hdl_interrupt_request(ic, spi->config->spi_interrupt, &spi->private.spi_isr);
    hdl_interrupt_request(ic, spi->config->nss_interrupt, &spi->private.nss_isr);
    spi_enable((uint32_t)spi->module.reg);
    return HDL_MODULE_ACTIVE;
  }
  rcu_periph_clock_disable(spi->config->rcu);
  return HDL_MODULE_UNLOADED;
}

void hdl_spi_server_set_transceiver(hdl_spi_server_t *spi, hdl_transceiver_t *transceiver) {
  if(spi != NULL) {
    ((hdl_spi_server_private_t*)spi)->private.transceiver = transceiver;
  }
}

#include "hdl_portable.h"
#include "CodeLib.h"

typedef struct {
  hdl_module_t module;
  hdl_spi_server_config_t *config;
  hdl_nvic_irq_n_t spi_iterrupt;
  hdl_nvic_irq_n_t nss_iterrupt;
  hdl_basic_buffer_t rx_mem;
  hdl_basic_buffer_t tx_mem;
  /* private */
  hdl_delegate_t spi_isr;
  hdl_delegate_t nss_isr;
} hdl_spi_mem_server_private_t;

_Static_assert(sizeof(hdl_spi_mem_server_private_t) <= sizeof(hdl_spi_mem_server_t), "In port_spi.h data structure size of hdl_spi_mem_server_t doesn't match, check SPI_MEM_SERVER_PRIVATE_SIZE");

static void _spi_mem_reset_dma(hdl_spi_mem_server_private_t *spi) {
  hdl_dma_channel_t *dma_rx = (hdl_dma_channel_t *)spi->module.dependencies[6];
  hdl_dma_channel_t *dma_tx = (hdl_dma_channel_t *)spi->module.dependencies[7];
  hdl_dma_run(dma_rx, (uint32_t)&SPI_DATA((uint32_t)spi->module.reg), (uint32_t)spi->rx_mem.data, (uint32_t)spi->rx_mem.size);
  hdl_dma_run(dma_tx, (uint32_t)&SPI_DATA((uint32_t)spi->module.reg), (uint32_t)spi->tx_mem.data, (uint32_t)spi->tx_mem.size);
}

static void event_spi_nss(uint32_t event, void *sender, void *context) {
  hdl_spi_mem_server_private_t *spi = (hdl_spi_mem_server_private_t*)context;
  hdl_gpio_pin_t *nss = (hdl_gpio_pin_t *)spi->module.dependencies[3];
  if(event & (uint32_t)nss->module.reg) {
    hdl_spi_reset_status((uint32_t)spi->module.reg);
    _spi_mem_reset_dma;
  }
}

static void event_spi_isr(uint32_t event, void *sender, void *context) {
  hdl_spi_mem_server_private_t *spi = (hdl_spi_mem_server_private_t *)context;
  uint32_t state = SPI_STAT((uint32_t)spi->module.reg);
  uint32_t cr1 = SPI_CTL1((uint32_t)spi->module.reg);
  if ((state & SPI_ERROR_MASK) != 0) {
    hdl_spi_reset_status((uint32_t)spi->module.reg);
  }
}

hdl_module_state_t hdl_spi_memory_server(void *desc, uint8_t enable) {
  hdl_spi_mem_server_private_t *spi = (hdl_spi_mem_server_private_t*)desc;
  rcu_periph_enum rcu;
  switch ((uint32_t)spi->module.reg) {
    case SPI0: rcu = RCU_SPI0; break;
    case SPI1: rcu = RCU_SPI1; break;
    default: return HDL_MODULE_INIT_FAILED;
  }
  spi_i2s_deinit((uint32_t)spi->module.reg);
  if(enable) {
    rcu_periph_clock_enable(rcu);
    spi_parameter_struct init;
    init.device_mode = SPI_SLAVE;
    init.trans_mode = SPI_TRANSMODE_FULLDUPLEX;
    init.frame_size = SPI_FRAMESIZE_8BIT;
    init.endian = spi->config->endian;
    init.clock_polarity_phase = spi->config->polarity;
    init.nss = SPI_NSS_HARD;
    init.prescale = spi->config->prescale;
    spi_init((uint32_t)spi->module.reg, &init);
    SPI_CTL1((uint32_t)spi->module.reg) |= SPI_CTL1_ERRIE;
    spi->nss_isr.context = desc;
    spi->nss_isr.handler = &event_spi_nss;
    spi->spi_isr.context = desc;
    spi->spi_isr.handler = &event_spi_isr;
    hdl_interrupt_controller_t *ic = (hdl_interrupt_controller_t *)spi->module.dependencies[5];
    hdl_interrupt_request(ic, spi->spi_iterrupt, &spi->spi_isr);
    hdl_interrupt_request(ic, spi->nss_iterrupt, &spi->nss_isr);
    _spi_mem_reset_dma(spi);
    spi_enable((uint32_t)spi->module.reg);
    return HDL_MODULE_INIT_OK;
  }
  rcu_periph_clock_disable(rcu);
  return HDL_MODULE_DEINIT_OK;
}

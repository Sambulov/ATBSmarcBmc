#include "hdl_portable.h"
#include "CodeLib.h"

typedef struct {
  hdl_module_t module;
  hdl_uart_config_t *config;
  struct {
    hdl_delegate_t uart_isr;
    hdl_transceiver_t *transceiver;
    coroutine_t worker;
  } private;
} hdl_uart_private_t;

HDL_ASSERRT_STRUCTURE_CAST(hdl_uart_private_t, hdl_uart_t, HDL_UART_PRV_SIZE, port_uart.h);

__STATIC_INLINE uint8_t _hdl_uart_word_len(hdl_uart_private_t *uart) {
  return ((uart->config->word_len == USART_WL_9BIT) && (uart->config->parity==USART_PM_NONE))? 2: 1;
}

static uint8_t _uart_worker(coroutine_t *this, uint8_t cancel, void *arg) {
  hdl_uart_private_t *uart = (hdl_uart_private_t *) arg;
  if((uart->private.transceiver != NULL) && !(USART_CTL0((uint32_t)uart->module.reg) & USART_CTL0_TBEIE)) {
    if((uart->private.transceiver->tx_available != NULL) && (uart->private.transceiver->tx_empty != NULL) && 
       (uart->private.transceiver->tx_available(uart->private.transceiver->proto_context) >= _hdl_uart_word_len(uart))) {
      usart_interrupt_enable((uint32_t)uart->module.reg, USART_INT_TBE);
    }
  }
  return cancel;
}

static void _rst_uart_status(hdl_uart_private_t *uart) {
  USART_INTC((uint32_t)uart->module.reg) |= USART_INTC_PEC | USART_INTC_FEC | USART_INTC_NEC | USART_INTC_OREC;
	__IO uint32_t tmpreg;
	tmpreg = USART_RDATA((uint32_t)uart->module.reg);
	(void)tmpreg;
}

static void event_uart_isr(uint32_t event, void *sender, void *context) {
  hdl_uart_private_t *uart = (hdl_uart_private_t *)context;
  uint32_t periph = (uint32_t)uart->module.reg;
	if (usart_interrupt_flag_get(periph, USART_INT_FLAG_IDLE)) {
    usart_interrupt_flag_clear(periph, USART_INT_FLAG_IDLE);
    if((uart->private.transceiver != NULL) && (uart->private.transceiver->end_of_transmission != NULL))
      uart->private.transceiver->end_of_transmission(uart->private.transceiver->proto_context);
	}
	if (usart_interrupt_flag_get(periph, USART_INT_FLAG_ERR_NERR) ||
      usart_interrupt_flag_get(periph, USART_FLAG_FERR) ||
      usart_interrupt_flag_get(periph, USART_FLAG_PERR)) {
      _rst_uart_status(uart);
      return;
  }
  /* UART in mode Receiver ---------------------------------------------------*/
  if (usart_interrupt_flag_get(periph, USART_INT_FLAG_RBNE)) {
    uint16_t data = usart_data_receive(periph);
    usart_interrupt_enable(periph, USART_INT_IDLE);
    uint8_t wl = _hdl_uart_word_len(uart);
    if((uart->private.transceiver != NULL) && (uart->private.transceiver->rx_available != NULL) && (uart->private.transceiver->rx_data != NULL)) {
      if(uart->private.transceiver->rx_available(uart->private.transceiver->proto_context) >= wl) {          
        uart->private.transceiver->rx_data(uart->private.transceiver->proto_context, (uint8_t *)&data, wl);
      }
    }
  }
  ///* UART in mode Transmitter ------------------------------------------------*/
  if (usart_interrupt_flag_get(periph, USART_INT_FLAG_TBE)) {
    usart_interrupt_disable(periph, USART_INT_TBE);
    uint8_t wl = _hdl_uart_word_len(uart);
    if((uart->private.transceiver != NULL) && (uart->private.transceiver->tx_available != NULL) && (uart->private.transceiver->tx_empty != NULL)) {
      if(uart->private.transceiver->tx_available(uart->private.transceiver->proto_context) >= wl) {
        uint16_t data;
        uart->private.transceiver->tx_empty(uart->private.transceiver->proto_context, (uint8_t *)&data, wl);
        usart_data_transmit(periph, data);
        USART_CTL0(periph) |= USART_CTL0_TBEIE;
      }
    }
  }
}

hdl_module_state_t hdl_uart(void *desc, uint8_t enable) {
  hdl_uart_private_t *uart = (hdl_uart_private_t*)desc;
  uint32_t periph = (uint32_t)uart->module.reg;
  usart_deinit(periph);
  if(enable) {
    rcu_periph_clock_enable(uart->config->rcu);
    usart_baudrate_set(periph, uart->config->baudrate);
    usart_word_length_set(periph, uart->config->word_len);
    usart_parity_config(periph, uart->config->parity);
    usart_stop_bit_set(periph, uart->config->stop_bits);
    usart_transmit_config(periph, USART_TRANSMIT_ENABLE);
    usart_receive_config(periph, USART_RECEIVE_ENABLE);
    coroutine_add(&uart->private.worker, &_uart_worker, desc);
    uart->private.transceiver = NULL;
    hdl_interrupt_controller_t *ic = (hdl_interrupt_controller_t *)uart->module.dependencies[3];
    uart->private.uart_isr.context = desc;
    uart->private.uart_isr.handler = &event_uart_isr;
    hdl_interrupt_request(ic, uart->config->interrupt, &uart->private.uart_isr);
    usart_interrupt_enable(periph, USART_INT_PERR);
    usart_interrupt_enable(periph, USART_INT_RBNE);
    usart_interrupt_enable(periph, USART_INT_ERR);
    // USART_CTL0(periph) |= USART_CTL0_RBNEIE | USART_CTL0_PERRIE;
    // USART_CTL2(periph) |= USART_CTL2_ERRIE;
    usart_enable(periph);
    return HDL_MODULE_ACTIVE;
  }
  coroutine_cancel(&uart->private.worker);
  rcu_periph_clock_disable(uart->config->rcu);
  return HDL_MODULE_UNLOADED;
}

void hdl_uart_set_transceiver(hdl_uart_t *uart, hdl_transceiver_t *transceiver) {
  if(uart != NULL) {
    ((hdl_uart_private_t*)uart)->private.transceiver = transceiver;
  }
}

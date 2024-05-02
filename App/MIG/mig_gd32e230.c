/*
  Module Init Graph (MIG)
*/

#include "hdl.h"

#if defined ( ATB_RK3568J_SMC_R1 )




// #define TEST_NO 7

// /* 
//   1  - pll by hxtal/2, sys clock 108MHz SysTick          | w
//   2  - pll by hxtal/2, sys clock 72MHz  SysTick          | w
//   3  - pll by hxtal/1, sys clock 108MHz SysTick          | w
//   4  - pll by irc8m/2, sys clock 108MHz SysTick          | w
//   5  - pll by irc8m/2, sys clock 72MHz  SysTick          | w
//   6  - pll by irc8m/2, sys clock 36MHz TIMER0            | w
//   7  - pll by irc8m/2, sys clock 36MHz APB2/2 TIMER0*2   | w
//   8  - pll by irc8m/2, sys clock 108MHz APB2/16 TIMER0*2 | w
//   9  - pll by irc8m/2, sys clock 108MHz APB1/16 TIMER1*2 | w
//   10 - pll by irc8m/2, sys clock 52MHz  APB1/1 TIMER1*1  | w
//   11 - pll by irc8m/2, sys clock 52MHz  APB1/2 TIMER1*2  | w
// */


//#define TEST_CLOCK
//#define TEST_CLOCK_NO 1

#ifdef TEST_CLOCK
/*
* 1 - ahb by ck_pll, pll mf 2, hxtal predv 1, sys_clock 32Mhz, SysTick counter_reload 32000
*/

#if TEST_CLOCK_NO == 1

  #define HDL_HXTAL_CLOCK              16000000
  #define HDL_LXTAL_CLOCK              32768
  #define HDL_HXTAL_2_PLLSEL_PREDIV    1
  #define HDL_PLLMUL                   2
  #define HDL_AHB_PREDIV               1
  #define HDL_APB1_PREDIV              1
  #define HDL_APB2_PREDIV              1
  #define HDL_RTC_CLOCK                mod_clock_hxtal           /* Can be clocked by: mod_clock_hxtal, mod_clock_lxtal, mod_clock_irc40k. For mod_clock_hxtal applied prediv 2 */
  #define HDL_PLL_MUL_CLOCK            mod_clock_pll_prescaler   /* Can be clocked by: mod_clock_pll_prescaler, mod_clock_irc8m. For mod_clock_irc8m applied prediv 2 */
  #define HDL_SYS_CLOCK                mod_clock_pll_mul         /* Can be clocked by: mod_clock_pll_mul, mod_clock_irc8m, mod_clock_hxtal */

  #define HDL_SYSTICK_COUNTER_RELOAD  32000 - 1 
#endif

#else

  #define HDL_HXTAL_CLOCK              16000000
  #define HDL_LXTAL_CLOCK              32768
  #define HDL_HXTAL_2_PLLSEL_PREDIV    2
  #define HDL_PLLMUL                   9
  #define HDL_AHB_PREDIV               1
  #define HDL_APB1_PREDIV              1
  #define HDL_APB2_PREDIV              1
  #define HDL_RTC_CLOCK                mod_clock_hxtal           /* Can be clocked by: mod_clock_hxtal, mod_clock_lxtal, mod_clock_irc40k. For mod_clock_hxtal applied prediv 2 */
  #define HDL_PLL_MUL_CLOCK            mod_clock_pll_prescaler   /* Can be clocked by: mod_clock_pll_prescaler, mod_clock_irc8m. For mod_clock_irc8m applied prediv 2 */
  #define HDL_SYS_CLOCK                mod_clock_pll_mul         /* Can be clocked by: mod_clock_pll_mul, mod_clock_irc8m, mod_clock_hxtal */

  #define HDL_SYSTICK_COUNTER_RELOAD  32000 - 1 

#endif

  #define HDL_INTERRUPT_PRIO_GROUP_BITS   __NVIC_PRIO_BITS

  hdl_core_t mod_sys_core = {
    .module.init = &hdl_core,
    .module.dependencies = NULL,
    .module.reg = (void *)SCB_BASE,
    .flash_latency = WS_WSCNT_2 /* WS_WSCNT_0: sys_clock <= 24MHz, WS_WSCNT_1: sys_clock <= 48MHz, WS_WSCNT_2: sys_clock <= 72MHz */
    /* TODO: ... */
  };

  /**************************************************************
   *  NVIC, IRQ, EXTI
   *************************************************************/
  hdl_nvic_interrupt_t mod_irq_systick = {
    .irq_type = HDL_NVIC_EXCEPTION_SysTick,
    .priority = 0,
    .priority_group = 0,
  };

  hdl_nvic_interrupt_t mod_irq_exti_0_1 = {
    .irq_type = HDL_NVIC_IRQ5_EXTI0_1,
    .priority = 0,
    .priority_group = 1,
  };

  hdl_nvic_interrupt_t mod_irq_exti_2_3 = {
    .irq_type = HDL_NVIC_IRQ6_EXTI2_3,
    .priority = 0,
    .priority_group = 2,
  };

  hdl_nvic_interrupt_t mod_irq_8 = {
    .irq_type = HDL_NVIC_IRQ7_EXTI4_15,
    .priority = 0,
    .priority_group = 2,
  };

  hdl_nvic_exti_t mod_nvic_exti_line_0 = {
    .line = HDL_EXTI_LINE_0,
    .mode = HDL_EXTI_MODE_INTERRUPT,
    .source = HDL_EXTI_SOURCE_PA,
    .trigger = HDL_EXTI_TRIGGER_FALLING
  };

  hdl_nvic_exti_t mod_nvic_exti_line_8 = {
  .line = HDL_EXTI_LINE_8,
  .mode = HDL_EXTI_MODE_INTERRUPT,
  .source = HDL_EXTI_SOURCE_PB,
  .trigger = HDL_EXTI_TRIGGER_RISING
};

  hdl_nvic_t mod_nvic = {
    .module.init = &hdl_nvic,
    .module.dependencies = hdl_module_dependencies(&mod_sys_core.module),
    .module.reg = NVIC,
    .prio_bits = HDL_INTERRUPT_PRIO_GROUP_BITS,
    .irq_latency = 0, /* TODO: define static assert */
    .interrupts = hdl_interrupts(&mod_irq_systick, &mod_irq_exti_0_1, &mod_irq_exti_2_3, &mod_irq_8),
    .exti_lines = hdl_exti_lines(&mod_nvic_exti_line_0, &mod_nvic_exti_line_8)
  };

  /**************************************************************
   *  Oscillator
   *************************************************************/
  hdl_clock_t mod_clock_irc8m = {
    .module.init = &hdl_clock_irc8m,
    .module.dependencies = NULL,
    .module.reg = (void *)RCU,
    .freq = 8000000,
    .div = 1
  };

  hdl_clock_t mod_clock_hxtal = {
    .module.init = &hdl_clock_hxtal,
    .module.dependencies = NULL,
    .module.reg = (void *)RCU,
    .freq = HDL_HXTAL_CLOCK,
    .div = 1
  };

  hdl_clock_t mod_clock_lxtal = {
    .module.init = &hdl_clock_lxtal,
    .module.dependencies = NULL,
    .module.reg = (void *)RCU,
    .freq = HDL_LXTAL_CLOCK,
    .div = 1
  };

  hdl_clock_t mod_clock_irc28m = {
    .module.init = &hdl_clock_irc28m,
    .module.dependencies = NULL,
    .module.reg = (void *)RCU,
    .freq = 28000000,
    .div = 1
  };

  hdl_clock_t mod_clock_irc40k = {
    .module.init = &hdl_clock_irc40k,
    .module.dependencies = NULL,
    .freq = 40000,
    .div = 1
  };

  /**************************************************************
   *  PLL prescaler
   *************************************************************/
  hdl_clock_prescaler_t mod_clock_pll_prescaler = {
    .module.init = &hdl_clock_hxtal_prescaler,
    .module.dependencies = hdl_module_dependencies(&mod_clock_hxtal.module),
    .module.reg = (void *)RCU,
    .muldiv_factor = HDL_HXTAL_2_PLLSEL_PREDIV,
  };

  /**************************************************************
   *  Selector PLL source second rang (HXTAL or IRC8M)
   **************************************************************/
  hdl_clock_t mod_clock_pll_selector = {
    .module.init = &hdl_clock_selector_pll,
    /* If source IRC8M before oscillator there is prescaler 2, this logic realized inside driver */
    .module.dependencies = hdl_module_dependencies(&HDL_PLL_MUL_CLOCK.module),
    .module.reg = (void *)RCU
  };

  /**************************************************************
   *  PLL multiply factor
   *************************************************************/
  hdl_clock_prescaler_t mod_clock_pll_mul = {
    .module.init = &hdl_clock_pll,
    .module.dependencies = hdl_module_dependencies(&mod_clock_pll_selector.module),
    .module.reg = (void *)RCU,
    .muldiv_factor = HDL_PLLMUL,
  };

  /**************************************************************
   *  Selector system clock source (IRC8M, CK_PLL, HXTAL)
   **************************************************************/
  hdl_clock_prescaler_t mod_clock_system_clock_source = {
    .module.init = &hdl_clock_system,
    .module.dependencies = hdl_module_dependencies(&mod_sys_core.module, &HDL_SYS_CLOCK.module),
    .module.reg = (void *)RCU,
    .muldiv_factor = 1,
  };

  /**************************************************************
   *  Selector RTC(HXTAL, LXTAL, IRC40K)
   *************************************************************/
  hdl_clock_prescaler_t mod_clock_selector_rtc = {
    .module.init = &hdl_clock_selector_rtc,
    .module.dependencies = hdl_module_dependencies(&HDL_RTC_CLOCK.module),
    .module.reg = (void *)RCU,
    .muldiv_factor = 1,
  };

  /**************************************************************
   *  AHB Prescaler
   *************************************************************/
  hdl_clock_prescaler_t mod_clock_ahb = {
    .module.init = &hdl_clock_ahb,
    .module.dependencies = hdl_module_dependencies(&mod_clock_system_clock_source.module),
    .module.reg = (void *)RCU,
    .muldiv_factor = HDL_AHB_PREDIV,
  };
  /**************************************************************
   *  APB1 Prescaler
   *************************************************************/
  hdl_clock_prescaler_t mod_clock_apb1 = {
    .module.init = &hdl_clock_apb1,
    .module.dependencies = hdl_module_dependencies(&mod_clock_ahb.module),
    .module.reg = (void *)RCU,
    .muldiv_factor = HDL_APB1_PREDIV,
  };
  /**************************************************************
   *  APB2 Prescaler
   *************************************************************/
  hdl_clock_prescaler_t mod_clock_apb2 = {
    .module.init = &hdl_clock_apb2,
    .module.dependencies = hdl_module_dependencies(&mod_clock_ahb.module),
    .module.reg = (void *)RCU,
    .muldiv_factor = HDL_APB2_PREDIV,
  };
  /**************************************************************
   *  DMA
   *************************************************************/
  hdl_dma_t mod_dma = {
    .module.init = &hdl_dma,
    .module.dependencies = hdl_module_dependencies(&mod_clock_ahb.module),
    .module.reg = (void*)DMA_BASE,
  };

hdl_dma_channel_t mod_adc_dma_ch = {
  .module.init = &hdl_dma_ch,
  .module.dependencies = hdl_module_dependencies(&mod_dma.module),
  .module.reg = DMA_CH0,
  .direction = HDL_DMA_DIRECTION_P2M,
  .memory_inc = HDL_DMA_INCREMENT_ON,
  .memory_width = HDL_DMA_SIZE_OF_MEMORY_32_BIT,
  .periph_inc = HDL_DMA_INCREMENT_OFF,
  .periph_width = HDL_DMA_SIZE_OF_MEMORY_16_BIT,
  .mode = HDL_DMA_MODE_CIRCULAR,
  .priority = 0
};

  hdl_clock_prescaler_t mod_clock_timer0 = {
    .module.init = NULL,
    .module.dependencies = hdl_module_dependencies(&mod_clock_ahb.module),
    .module.reg = (void *)RCU,
    .muldiv_factor = 72,
  };

  hdl_clock_counter_t mod_timer0_counter = {
    .module.init = &hdl_clock_counter,
    .module.dependencies = hdl_module_dependencies(&mod_clock_timer0.module),
    .module.reg = (void *)TIMER0,
    .diction = HDL_UP_COUNTER,
    .counter_reload = 1000 - 1
  };

  hdl_clock_counter_t mod_systick_counter = {
    .module.init = &hdl_clock_counter,
    .module.dependencies = hdl_module_dependencies(&mod_clock_ahb.module),
    .module.reg = (void *)SysTick,
    .diction = HDL_DOWN_COUNTER,
    .counter_reload = HDL_SYSTICK_COUNTER_RELOAD
  };

  hdl_timer_t mod_timer_ms = {
    .module.init = hdl_timer,
    .module.dependencies = hdl_module_dependencies(&mod_systick_counter.module, &mod_nvic.module),
    .module.reg = NULL,
    .reload_iterrupt = HDL_NVIC_EXCEPTION_SysTick,
    .val = 0
  };

  /**************************************************************
   *  ADC
   *************************************************************/
  hdl_adc_source_t mod_adc_source_0 = {
    .channel = HDL_ADC_CHANNEL_3,
    .sample_time = HDL_ADC_CHANNEL_SAMPLE_TIME_7P5
  };
  hdl_adc_source_t mod_adc_source_1 = {
    .channel = HDL_ADC_CHANNEL_7,
    .sample_time = HDL_ADC_CHANNEL_SAMPLE_TIME_7P5
  };
  // hdl_adc_channel_source_t mod_adc_channel_7 = {
  //   .channel_number = HDL_ADC_CHANNEL_7,
  //   .channel_sample_time = HDL_ADC_CHANNEL_SAMPLE_TIME_7P5
  // };
  // hdl_adc_channel_source_t mod_adc_channel_8 = {
  //   .channel_number = HDL_ADC_CHANNEL_8,
  //   .channel_sample_time = HDL_ADC_CHANNEL_SAMPLE_TIME_7P5
  // };

  hdl_adc_t mod_adc = {
    .module.init = &hdl_adc,
    .module.dependencies = hdl_module_dependencies(&mod_clock_irc28m.module, &mod_timer_ms.module, &mod_dma.module),
    .module.reg = (void*)ADC,
    .dma_channel = DMA_CH0,
    //.start_triger = HDL_ADC_TRIGER_SOFTWARE,
    //.mode = ADC_OPERATION_MODE_CONTINUOS_SCAN,
    .resolution = HDL_ADC_RESOLUTION_12BIT,
    .data_alignment = HDL_ADC_DATA_ALIGN_RIGHT,
    .init_timeout = 3000,
    .sources = hdl_adc_src(&mod_adc_source_1, &mod_adc_source_0),
  };

  hdl_gpio_port_t hdl_gpio_port_a = {
    .init = &hdl_gpio_port,
    .dependencies = hdl_module_dependencies(&mod_clock_ahb.module),
    .reg = (void *)GPIOA,
  };

  hdl_gpio_port_t hdl_gpio_port_b = {
    .init = &hdl_gpio_port,
    .dependencies = hdl_module_dependencies(&mod_clock_ahb.module),
    .reg = (void *)GPIOB,
  };

  hdl_gpio_port_t hdl_gpio_port_c = {
    .init = &hdl_gpio_port,
    .dependencies = hdl_module_dependencies(&mod_clock_ahb.module),
    .reg = (void *)GPIOC,
  };

  hdl_gpio_port_t hdl_gpio_port_f = {
    .init = &hdl_gpio_port,
    .dependencies = hdl_module_dependencies(&mod_clock_ahb.module),
    .reg = (void *)GPIOF,
  };

  hdl_gpio_mode_t mod_gpio_output_pp_mode = {
    .type = GPIO_MODE_OUTPUT,
    .otype = GPIO_OTYPE_PP,
    .ospeed = GPIO_OSPEED_2MHZ,
  };

  hdl_gpio_mode_t mod_gpio_input_pullup_mode = {
    .type = GPIO_MODE_INPUT,
    .pull = GPIO_PUPD_PULLUP,
  };

  hdl_gpio_mode_t mod_gpio_input_analog = {
    .type = GPIO_MODE_ANALOG,
    .pull = GPIO_PUPD_NONE,
  };

  hdl_gpio_mode_t mod_gpio_alternate_swd_mode = {
    .af = GPIO_AF_0,
    .type = GPIO_MODE_AF,
    .otype = GPIO_OTYPE_PP,
    .ospeed = GPIO_OSPEED_50MHZ,
  };

  hdl_gpio_mode_t mod_gpio_i2c_mode = {
    .af = GPIO_AF_4,
    .type = GPIO_MODE_AF,
    .otype = GPIO_OTYPE_OD,
    .ospeed = GPIO_OSPEED_2MHZ
  };

  // const hdl_gpio_t mod_gpi_pmic_sleep_1v8 = {
  //   .port = &hdl_gpio_port_a,
  //   .pin = GPIO_PIN_15,
  //   .mode = &mod_gpio_input_pullup_mode
  // };

  // const hdl_gpio_t mod_gpi_carrier_lid = {
  //   .port = &hdl_gpio_port_b,
  //   .pin = GPIO_PIN_2,
  //   .mode = &mod_gpio_input_pullup_mode
  // };

  // const hdl_gpio_t mod_gpi_carrier_batlow = {
  //   .port = &hdl_gpio_port_b,
  //   .pin = GPIO_PIN_6,
  //   .mode = &mod_gpio_input_pullup_mode
  // };

  // const hdl_gpio_t mod_gpi_carrier_sleep = {
  //   .port = &hdl_gpio_port_b,
  //   .pin = GPIO_PIN_7,
  //   .mode = &mod_gpio_input_pullup_mode
  // };

  hdl_gpio_pin_t mod_gpi_carrier_wdt_time_out = {
    .module.init = &hdl_gpio_pin,
    .module.dependencies = hdl_module_dependencies(&hdl_gpio_port_b),
    .module.reg = (void *)GPIO_PIN_8,
    .mode = &mod_gpio_output_pp_mode
  };

  hdl_gpio_pin_t mod_gpio_adc_channel_3v3 = {
    .module.init = &hdl_gpio_pin,
    .module.dependencies = hdl_module_dependencies(&hdl_gpio_port_a),
    .module.reg = (void *)GPIO_PIN_7,
    .mode = &mod_gpio_input_analog  
  };

    hdl_gpio_pin_t mod_gpio_adc_channel_1v5 = {
    .module.init = &hdl_gpio_pin,
    .module.dependencies = hdl_module_dependencies(&hdl_gpio_port_a),
    .module.reg = (void *)GPIO_PIN_3,
    .mode = &mod_gpio_input_analog  
  };

  // const hdl_gpio_t mod_gpi_carrier_charging = {
  //   .port = &hdl_gpio_port_f,
  //   .pin = GPIO_PIN_0,
  //   .mode = &mod_gpio_input_pullup_mode
  // };

  // const hdl_gpio_t mod_gpi_carrier_charger_prstn = {
  //   .port = &hdl_gpio_port_f,
  //   .pin = GPIO_PIN_1,
  //   .mode = &mod_gpio_input_pullup_mode
  // };

  hdl_gpio_pin_t mod_gpo_emmc_lock = {
    .module.init = &hdl_gpio_pin,
    .module.dependencies = hdl_module_dependencies(&hdl_gpio_port_a),
    .module.reg = (void *)GPIO_PIN_0,
    .mode = &mod_gpio_output_pp_mode
  };

  hdl_gpio_pin_t mod_gpo_qspi_lock = {
    .module.init = &hdl_gpio_pin,
    .module.dependencies = hdl_module_dependencies(&hdl_gpio_port_a),
    .module.reg = (void *)GPIO_PIN_1,
    .mode = &mod_gpio_output_pp_mode
  };

  hdl_gpio_pin_t mod_gpo_sd_lock = {
    .module.init = &hdl_gpio_pin,
    .module.dependencies = hdl_module_dependencies(&hdl_gpio_port_a),
    .module.reg = (void *)GPIO_PIN_2,
    .mode = &mod_gpio_output_pp_mode
  };

  hdl_gpio_pin_t mod_gpi_carrier_force_recovery = {
    .module.init = &hdl_gpio_pin,
    .module.dependencies = hdl_module_dependencies(&hdl_gpio_port_a),
    .module.reg = (void *)GPIO_PIN_13,
    #if defined(DEBUG)
    .mode = &mod_gpio_alternate_swd_mode
    #else
    .mode = &mod_gpio_input_pullup_mode
    #endif
  };

  hdl_gpio_pin_t mod_gpi_carrier_boot_sel0 = {
    .module.init = &hdl_gpio_pin,
    .module.dependencies = hdl_module_dependencies(&hdl_gpio_port_a),
    .module.reg = (void *)GPIO_PIN_4,
    .mode = &mod_gpio_input_pullup_mode
  };

  hdl_gpio_pin_t mod_gpi_carrier_boot_sel1 = {
    .module.init = &hdl_gpio_pin,
    .module.dependencies = hdl_module_dependencies(&hdl_gpio_port_a),
    .module.reg = (void *)GPIO_PIN_5,
    .mode = &mod_gpio_input_pullup_mode
  };

  hdl_gpio_pin_t mod_gpi_carrier_boot_sel2 = {
    .module.init = &hdl_gpio_pin,
    .module.dependencies = hdl_module_dependencies(&hdl_gpio_port_a),
    .module.reg = (void *)GPIO_PIN_6,
    .mode = &mod_gpio_input_pullup_mode
  };

  hdl_gpio_pin_t mod_gpi_carrier_power_good = {
    .module.init = &hdl_gpio_pin,
    .module.dependencies = hdl_module_dependencies(&hdl_gpio_port_a),
    .module.reg = (void *)GPIO_PIN_0,
    .mode = &mod_gpio_input_pullup_mode
  };

  hdl_gpio_pin_t mod_gpi_carrier_reset_in = {
    .module.init = &hdl_gpio_pin,
    .module.dependencies = hdl_module_dependencies(&hdl_gpio_port_a),
    .module.reg = (void *)GPIO_PIN_7,
    .mode = &mod_gpio_input_pullup_mode
  };

  hdl_gpio_pin_t mod_gpo_carrier_reset_out = {
    .module.init = &hdl_gpio_pin,
    .module.dependencies = hdl_module_dependencies(&hdl_gpio_port_a),
    .module.reg = (void *)GPIO_PIN_8,
    .mode = &mod_gpio_output_pp_mode
  };

  hdl_gpio_pin_t mod_gpo_carrier_pwr_on = {
    .module.init = &hdl_gpio_pin,
    .module.dependencies = hdl_module_dependencies(&hdl_gpio_port_b),
    .module.reg = (void *)GPIO_PIN_1,
    .mode = &mod_gpio_output_pp_mode
  };

  hdl_gpio_pin_t mod_gpo_carrier_stby = {
    .module.init = &hdl_gpio_pin,
    .module.dependencies = hdl_module_dependencies(&hdl_gpio_port_b),
    .module.reg = (void *)GPIO_PIN_3,
    .mode = &mod_gpio_output_pp_mode
  };

  hdl_gpio_pin_t mod_gpi_carrier_power_btn = {
    .module.init = &hdl_gpio_pin,
    .module.dependencies = hdl_module_dependencies(&hdl_gpio_port_b),
    .module.reg = (void *)GPIO_PIN_4,
    .mode = &mod_gpio_input_pullup_mode
  };

  hdl_gpio_pin_t mod_gpo_pmic_soc_rst = {
    .module.init = &hdl_gpio_pin,
    .module.dependencies = hdl_module_dependencies(&hdl_gpio_port_a),
    .module.reg = (void *)GPIO_PIN_3,
    .mode = &mod_gpio_output_pp_mode
  };

  hdl_gpio_pin_t mod_gpo_pmic_power_on = {
    .module.init = &hdl_gpio_pin,
    .module.dependencies = hdl_module_dependencies(&hdl_gpio_port_b),
    .module.reg = (void *)GPIO_PIN_5,
    .mode = &mod_gpio_output_pp_mode
  };

  hdl_gpio_pin_t mod_gpio_soc_scl = {
    .module.init = &hdl_gpio_pin,
    .module.dependencies = hdl_module_dependencies(&hdl_gpio_port_a),
    .module.reg = (void *)GPIO_PIN_9,
    .mode = &mod_gpio_i2c_mode
  };

  hdl_gpio_pin_t mod_gpio_soc_sda = {
    .module.init = &hdl_gpio_pin,
    .module.dependencies = hdl_module_dependencies(&hdl_gpio_port_a),
    .module.reg = (void *)GPIO_PIN_10,
    .mode = &mod_gpio_i2c_mode
  };

  hdl_i2c_hw_t mod_i2c_0 = {
    .module.init = &hdl_i2c,
    .module.dependencies = hdl_module_dependencies(&mod_clock_apb1.module, &mod_gpio_soc_scl.module, &mod_gpio_soc_sda.module),
    //.clock = (hdl_clock_t *)&,
    //.rx_dma_ch = &mod_i2c_rx_dma
    //.tx_dma_ch = &mod_i2c_tx_dma
    //.interrupt = &mod_i2c_int
    .reg = I2C0,
    .speed = 100000,
    .dtcy = I2C_DTCY_2,
    .mode = 3,
    .stretching = 1,
    .general_call = 1,
    .addr_size = I2C_ADDFORMAT_7BITS,
    .addr = 0x3F,
  };

#endif

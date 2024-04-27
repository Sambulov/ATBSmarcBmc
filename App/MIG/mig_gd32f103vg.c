/*
  Module Init Graph (MIG)
*/

#include "bldl.h"

#define TEST_NO 5
/* 
  1  - pll by hxtal/2, sys clock 108MHz SysTick          | w
  2  - pll by hxtal/2, sys clock 72MHz  SysTick          | w
  3  - pll by hxtal/1, sys clock 108MHz SysTick          | w
  4  - pll by irc8m/2, sys clock 108MHz SysTick          | w
  5  - pll by irc8m/2, sys clock 72MHz  SysTick          | w
  6  - pll by irc8m/2, sys clock 108MHz TIMER0           | x
  7  - pll by irc8m/2, sys clock 108MHz APB2/2 TIMER0*2  | x
  8  - pll by irc8m/2, sys clock 108MHz APB2/16 TIMER0*2 | w
  9  - pll by irc8m/2, sys clock 108MHz APB1/16 TIMER1*2 | x
  10 - pll by irc8m/2, sys clock 52MHz  APB1/1 TIMER1*1  | x
  11 - pll by irc8m/2, sys clock 52MHz  APB1/2 TIMER1*2  | w
*/

  #define HDL_HXTAL_CLOCK              8000000
  #define HDL_LXTAL_CLOCK              32768
  //#define HDL_HXTAL_2_PLLSEL_PREDIV    2
  //#define HDL_PLLMUL                   27
  #define HDL_AHB_PREDIV               1
  //#define HDL_APB1_PREDIV              2
  //#define HDL_APB2_PREDIV              1
  #define HDL_RTC_CLOCK                mod_clock_hxtal           /* Can be clocked by: mod_clock_hxtal, mod_clock_lxtal, mod_clock_irc40k. For mod_clock_hxtal applied prediv 2 */
  //#define HDL_PLL_MUL_CLOCK            mod_clock_hxtal_prescaler /* Can be clocked by: mod_clock_hxtal_prescaler, mod_clock_irc8m. For mod_clock_irc8m applied prediv 2 */
  #define HDL_SYS_CLOCK                mod_clock_pll             /* Can be clocked by: mod_clock_pll, mod_clock_irc8m, mod_clock_hxtal */
  //#define MS_TIMER_RELOAD_VAL          108000-1

#if TEST_NO == 1
  #define HDL_HXTAL_2_PLLSEL_PREDIV    2
  #define HDL_PLLMUL                   27
  #define HDL_PLL_MUL_CLOCK            mod_clock_hxtal_prescaler
  #define MS_TIMER_RELOAD_VAL          108000-1
  #define HDL_APB1_PREDIV              2
  #define HDL_APB2_PREDIV              1
#endif

#if TEST_NO == 2
  #define HDL_HXTAL_2_PLLSEL_PREDIV    2
  #define HDL_PLLMUL                   18
  #define HDL_PLL_MUL_CLOCK            mod_clock_hxtal_prescaler
  #define MS_TIMER_RELOAD_VAL          72000-1
  #define HDL_APB1_PREDIV              2
  #define HDL_APB2_PREDIV              1
#endif

#if TEST_NO == 3
  #define HDL_HXTAL_2_PLLSEL_PREDIV    1
  #define HDL_PLLMUL                   9
  #define HDL_PLL_MUL_CLOCK            mod_clock_hxtal_prescaler
  #define MS_TIMER_RELOAD_VAL          72000-1
  #define HDL_APB1_PREDIV              2
  #define HDL_APB2_PREDIV              1
#endif

#if TEST_NO == 4
  #define HDL_HXTAL_2_PLLSEL_PREDIV    2
  #define HDL_PLLMUL                   27
  #define HDL_PLL_MUL_CLOCK            mod_clock_irc8m
  #define MS_TIMER_RELOAD_VAL          108000-1
  #define HDL_APB1_PREDIV              2
  #define HDL_APB2_PREDIV              1
#endif

#if TEST_NO == 5
  #define HDL_HXTAL_2_PLLSEL_PREDIV    2
  #define HDL_PLLMUL                   18
  #define HDL_PLL_MUL_CLOCK            mod_clock_irc8m
  #define MS_TIMER_RELOAD_VAL          72000-1
  #define HDL_APB1_PREDIV              2
  #define HDL_APB2_PREDIV              1
#endif

#if TEST_NO == 6
  #define HDL_HXTAL_2_PLLSEL_PREDIV    2
  #define HDL_PLLMUL                   27
  #define HDL_PLL_MUL_CLOCK            mod_clock_irc8m
  #define MS_TIMER_RELOAD_VAL          108000-1
  #define HDL_APB1_PREDIV              2
  #define HDL_APB2_PREDIV              1
#endif

#if TEST_NO == 7
  #define HDL_HXTAL_2_PLLSEL_PREDIV    2
  #define HDL_PLLMUL                   27
  #define HDL_PLL_MUL_CLOCK            mod_clock_irc8m
  #define MS_TIMER_RELOAD_VAL          108000-1
  #define HDL_APB1_PREDIV              2
  #define HDL_APB2_PREDIV              2
#endif

#if TEST_NO == 8
  #define HDL_HXTAL_2_PLLSEL_PREDIV    2
  #define HDL_PLLMUL                   27
  #define HDL_PLL_MUL_CLOCK            mod_clock_irc8m
  #define MS_TIMER_RELOAD_VAL          13500-1
  #define HDL_APB1_PREDIV              2
  #define HDL_APB2_PREDIV              16
#endif

#if TEST_NO == 9
  #define HDL_HXTAL_2_PLLSEL_PREDIV    2
  #define HDL_PLLMUL                   27
  #define HDL_PLL_MUL_CLOCK            mod_clock_irc8m
  #define MS_TIMER_RELOAD_VAL          13500-1
  #define HDL_APB1_PREDIV              16
  #define HDL_APB2_PREDIV              2
#endif

#if TEST_NO == 10
  #define HDL_HXTAL_2_PLLSEL_PREDIV    2
  #define HDL_PLLMUL                   13
  #define HDL_PLL_MUL_CLOCK            mod_clock_irc8m
  #define MS_TIMER_RELOAD_VAL          52000-1
  #define HDL_APB1_PREDIV              1
  #define HDL_APB2_PREDIV              2
#endif

#if TEST_NO == 11
  #define HDL_HXTAL_2_PLLSEL_PREDIV    2
  #define HDL_PLLMUL                   13
  #define HDL_PLL_MUL_CLOCK            mod_clock_irc8m
  #define MS_TIMER_RELOAD_VAL          52000-1
  #define HDL_APB1_PREDIV              2
  #define HDL_APB2_PREDIV              2
#endif

  #define HDL_INTERRUPT_PRIO_GROUP_BITS   __NVIC_PRIO_BITS

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

  hdl_clock_t mod_clock_irc40k = {
    .module.init = &hdl_clock_irc40k,
    .module.dependencies = NULL,
    .freq = 40000,
    .div = 1
  };

  hdl_clock_prescaler_t mod_clock_hxtal_prescaler = {
    .module.init = &hdl_clock_hxtal_prescaler,
    .module.dependencies = hdl_module_dependencies(&mod_clock_hxtal.module),
    .module.reg = (void *)RCU,
    .muldiv_factor = HDL_HXTAL_2_PLLSEL_PREDIV,
  };

  hdl_clock_t mod_clock_selector_pll = {
    .module.init = &hdl_clock_selector_pll,
    /* If source IRC8M before oscillator there is prescaler 2, this logic realized inside driver */
    .module.dependencies = hdl_module_dependencies(&HDL_PLL_MUL_CLOCK.module),
    .module.reg = (void *)RCU,
  };

  hdl_clock_prescaler_t mod_clock_pll = {
    .module.init = &hdl_clock_pll,
    .module.dependencies = hdl_module_dependencies(&mod_clock_selector_pll.module),
    .module.reg = (void *)RCU,
    .muldiv_factor = HDL_PLLMUL
  };

  hdl_core_t mod_sys_core = {
    .module.init = &hdl_core,
    .module.dependencies = NULL,
    .module.reg = (void *)SCB_BASE,
    .flash_latency = WS_WSCNT_2 /* WS_WSCNT_0: sys_clock <= 24MHz, WS_WSCNT_1: sys_clock <= 48MHz, WS_WSCNT_2: sys_clock <= 72MHz */
    /* TODO: latency */
  };

  hdl_clock_prescaler_t mod_clock_system_clock_source = {
    .module.init = &hdl_clock_system,
    .module.dependencies = hdl_module_dependencies(&mod_sys_core.module, &HDL_SYS_CLOCK.module),
    .module.reg = (void *)RCU,
    .muldiv_factor = 1,
  };

  hdl_nvic_interrupt_t mod_irq_systick = {
    .irq_type = HDL_NVIC_EXCEPTION_SysTick,
    .priority = 0,
    .priority_group = 0,
  };

  hdl_nvic_interrupt_t mod_irq_timer0_update = {
    .irq_type = HDL_NVIC_IRQ25_TIMER0_UP_TIMER9,
    .priority = 0,
    .priority_group = 0,
  };

  hdl_nvic_interrupt_t mod_irq_timer1 = {
    .irq_type = HDL_NVIC_IRQ28_TIMER1,
    .priority = 0,
    .priority_group = 0,
  };

  hdl_nvic_t mod_nvic = {
    .module.init = &hdl_nvic,
    .module.dependencies = hdl_module_dependencies(&mod_sys_core.module),
    .module.reg = NVIC,
    .prio_bits = HDL_INTERRUPT_PRIO_GROUP_BITS,
    .interrupts = hdl_interrupts(&mod_irq_systick, &mod_irq_timer0_update, &mod_irq_timer1),
    //.exti_lines = hdl_exti_lines(&mod_nvic_exti_line_0, &mod_nvic_exti_line_8)
  };

  hdl_clock_prescaler_t mod_clock_ahb = {
    .module.init = &hdl_clock_ahb,
    .module.dependencies = hdl_module_dependencies(&mod_clock_system_clock_source.module),
    .module.reg = (void *)RCU,
    .muldiv_factor = HDL_AHB_PREDIV,
  };

  hdl_clock_prescaler_t mod_clock_apb1 = {
    .module.init = &hdl_clock_apb1,
    .module.dependencies = hdl_module_dependencies(&mod_clock_ahb.module),
    .module.reg = (void *)RCU,
    .muldiv_factor = HDL_APB1_PREDIV,
  };

  hdl_clock_prescaler_t mod_clock_apb2 = {
    .module.init = &hdl_clock_apb2,
    .module.dependencies = hdl_module_dependencies(&mod_clock_ahb.module),
    .module.reg = (void *)RCU,
    .muldiv_factor = HDL_APB2_PREDIV,
  };

  hdl_clock_counter_t mod_systick_counter = {
    .module.init = &hdl_clock_counter,
    .module.dependencies = hdl_module_dependencies(&mod_clock_ahb.module),
    .module.reg = (void *)SysTick,
    .diction = HDL_DOWN_COUNTER,
    .counter_reload = MS_TIMER_RELOAD_VAL
  };

  hdl_clock_counter_t mod_timer0_counter = {
    .module.init = &hdl_clock_counter,
    .module.dependencies = hdl_module_dependencies(&mod_clock_apb2.module),
    .module.reg = (void *)TIMER0,
    .diction = HDL_DOWN_COUNTER,
    .counter_reload = MS_TIMER_RELOAD_VAL
  };

  hdl_clock_counter_t mod_timer1_counter = {
    .module.init = &hdl_clock_counter,
    .module.dependencies = hdl_module_dependencies(&mod_clock_apb1.module),
    .module.reg = (void *)TIMER1,
    .diction = HDL_DOWN_COUNTER,
    .counter_reload = MS_TIMER_RELOAD_VAL
  };

#if TEST_NO >= 9
  hdl_timer_t mod_timer_ms = {
    .module.init = hdl_timer,
    .module.dependencies = hdl_module_dependencies(&mod_timer1_counter.module, &mod_nvic.module),
    .module.reg = NULL,
    .reload_iterrupt = HDL_NVIC_IRQ28_TIMER1,
    .val = 0
  };
#elif TEST_NO >= 6
  hdl_timer_t mod_timer_ms = {
    .module.init = hdl_timer,
    .module.dependencies = hdl_module_dependencies(&mod_timer0_counter.module, &mod_nvic.module),
    .module.reg = NULL,
    .reload_iterrupt = HDL_NVIC_IRQ25_TIMER0_UP_TIMER9,
    .val = 0
  };
#else
  hdl_timer_t mod_timer_ms = {
    .module.init = hdl_timer,
    .module.dependencies = hdl_module_dependencies(&mod_systick_counter.module, &mod_nvic.module),
    .module.reg = NULL,
    .reload_iterrupt = HDL_NVIC_EXCEPTION_SysTick,
    .val = 0
  };
#endif

  hdl_gpio_port_t mod_gpio_a = {
  .init = &hdl_gpio_port,
  .dependencies = hdl_module_dependencies(&mod_clock_ahb.module),
  .reg = (void *)GPIOA,
  .dependencies = NULL
};

hdl_gpio_port_t mod_gpio_b = {
  .init = &hdl_gpio_port,
  .dependencies = hdl_module_dependencies(&mod_clock_ahb.module),
  .reg = (void *)GPIOB,
  .dependencies = NULL
};

hdl_gpio_port_t mod_gpio_c = {
  .init = &hdl_gpio_port,
  .dependencies = hdl_module_dependencies(&mod_clock_ahb.module),
  .reg = (void *)GPIOC,
  .dependencies = NULL
};

hdl_gpio_port_t mod_gpio_d = {
  .init = &hdl_gpio_port,
  .dependencies = hdl_module_dependencies(&mod_clock_ahb.module),
  .reg = (void *)GPIOD,
  .dependencies = NULL
};

hdl_gpio_mode_t mod_gpio_mode_output_pp = {
  .ospeed = GPIO_OSPEED_2MHZ,
  .type = GPIO_MODE_OUT_PP
};

#if defined ( USPD )

hdl_gpio_pin_t mod_gpio_a15_led1 = {
  .module.init = &hdl_gpio_pin,
  .module.dependencies = hdl_module_dependencies(&mod_gpio_a),
  .module.reg = (void *)GPIO_PIN_15,
  .mode = &mod_gpio_mode_output_pp
};

hdl_gpio_pin_t mod_gpio_c10_led2 = {
  .module.init = &hdl_gpio_pin,
  .module.dependencies = hdl_module_dependencies(&mod_gpio_c),
  .module.reg = (void *)GPIO_PIN_10,
  .mode = &mod_gpio_mode_output_pp
};

hdl_gpio_pin_t mod_gpio_c11_led3 = {
  .module.init = &hdl_gpio_pin,
  .module.dependencies = hdl_module_dependencies(&mod_gpio_c),
  .module.reg = (void *)GPIO_PIN_11,
  .mode = &mod_gpio_mode_output_pp
};

hdl_gpio_pin_t mod_gpio_c12_led4 = {
  .module.init = &hdl_gpio_pin,
  .module.dependencies = hdl_module_dependencies(&mod_gpio_c),
  .module.reg = (void *)GPIO_PIN_12,
  .mode = &mod_gpio_mode_output_pp
};

hdl_gpio_pin_t mod_gpio_d0_led5 = {
  .module.init = &hdl_gpio_pin,
  .module.dependencies = hdl_module_dependencies(&mod_gpio_d),
  .module.reg = (void *)GPIO_PIN_0,
  .mode = &mod_gpio_mode_output_pp
};

hdl_gpio_pin_t mod_gpio_d1_led6 = {
  .module.init = &hdl_gpio_pin,
  .module.dependencies = hdl_module_dependencies(&mod_gpio_d),
  .module.reg = (void *)GPIO_PIN_1,
  .mode = &mod_gpio_mode_output_pp
};

hdl_gpio_pin_t mod_gpio_d2_led7 = {
  .module.init = &hdl_gpio_pin,
  .module.dependencies = hdl_module_dependencies(&mod_gpio_d),
  .module.reg = (void *)GPIO_PIN_2,
  .mode = &mod_gpio_mode_output_pp
};

hdl_gpio_pin_t mod_gpio_d3_led8 = {
  .module.init = &hdl_gpio_pin,
  .module.dependencies = hdl_module_dependencies(&mod_gpio_d),
  .module.reg = (void *)GPIO_PIN_3,
  .mode = &mod_gpio_mode_output_pp
};

hdl_gpio_pin_t mod_gpio_d4_led9 = {
  .module.init = &hdl_gpio_pin,
  .module.dependencies = hdl_module_dependencies(&mod_gpio_d),
  .module.reg = (void *)GPIO_PIN_4,
  .mode = &mod_gpio_mode_output_pp
};

hdl_gpio_pin_t mod_gpio_d5_led10 = {
  .module.init = &hdl_gpio_pin,
  .module.dependencies = hdl_module_dependencies(&mod_gpio_d),
  .module.reg = (void *)GPIO_PIN_5,
  .mode = &mod_gpio_mode_output_pp
};

hdl_gpio_pin_t mod_gpio_d6_led11 = {
  .module.init = &hdl_gpio_pin,
  .module.dependencies = hdl_module_dependencies(&mod_gpio_d),
  .module.reg = (void *)GPIO_PIN_6,
  .mode = &mod_gpio_mode_output_pp
};

hdl_gpio_pin_t mod_gpio_d7_led12 = {
  .module.init = &hdl_gpio_pin,
  .module.dependencies = hdl_module_dependencies(&mod_gpio_d),
  .module.reg = (void *)GPIO_PIN_7,
  .mode = &mod_gpio_mode_output_pp
};

#endif

#include "hdl_portable.h"
#include "Macros.h"

typedef struct {
  hdl_module_t module;
  const hdl_clock_config_t *config;
  hdl_clock_freq_t freq;
} hdl_clock_private_t;

HDL_ASSERRT_STRUCTURE_CAST(hdl_clock_private_t, hdl_clock_t, HDL_CLOCK_PRV_SIZE, port_clock.h);

#define IRC28M_STARTUP_TIMEOUT    ((uint32_t)0xFFFF)
#define CK_SYS_STARTUP_TIMEOUT    ((uint32_t)0xFFFF)
#define IRC40K_STARTUP_TIMEOUT    ((uint32_t)0xFFFF)
#define LXTAL_STARTUP_TIMEOUT     ((uint32_t)0xFFFF)
#define CK_SYS_STARTUP_TIMEOUT    ((uint32_t)0xFFFF)
#define PLL_STARTUP_TIMEOUT       ((uint32_t)0xFFFF)

static hdl_module_state_t _hdl_clock_osc_en(rcu_osci_type_enum osc, rcu_flag_enum stb_flag, uint32_t timeout) {
  rcu_osci_on(osc); /* Turn on oscillator */
  uint32_t stb_timer = HXTAL_STARTUP_TIMEOUT;
  FlagStatus osci_statable = RESET;
  while ((RESET == osci_statable) && (stb_timer--)) /* Wait until HXTAL will be stable */
    osci_statable = rcu_flag_get(stb_flag);
  if (osci_statable == RESET)
    return HDL_MODULE_FAULT;
  return HDL_MODULE_ACTIVE;
}

static hdl_module_state_t _hdl_clock_selector_pll(hdl_clock_private_t *clk, uint8_t enable) {
  if (enable) {
    if ((clk->module.dependencies == NULL) || (clk->module.dependencies[0] == NULL)) 
      return HDL_MODULE_FAULT;
    hdl_clock_private_t *clock_src = (hdl_clock_private_t *)clk->module.dependencies[0];
    if(clock_src->config->type == HDL_CLOCK_TYPE_HXTAL) {
      if (clk->config->property.div == 0 || clk->config->property.div > 16) return HDL_MODULE_FAULT;
      HDL_REG_MODIFY(RCU_CFG1, RCU_CFG1_PREDV, (clk->config->property.div - 1) << 0);
      HDL_REG_SET(RCU_CFG0, RCU_PLLSRC_HXTAL);
      hdl_clock_calc_div(&clock_src->freq, clk->config->property.div, &clk->freq);
    }
    else if(clock_src->config->type == HDL_CLOCK_TYPE_IRC8M) {
      HDL_REG_SET(RCU_CFG0, RCU_PLLSRC_IRC8M_DIV2);
      hdl_clock_calc_div(&clock_src->freq, 2, &clk->freq);
    }
    else {
      return HDL_MODULE_FAULT;
    }
    return HDL_MODULE_ACTIVE;
  }
  else {
    HDL_REG_CLEAR(RCU_CFG0, RCU_CFG0_PLLSEL);
    HDL_REG_MODIFY(RCU_CFG1, RCU_CFG1_PREDV, RCU_PLL_PREDV16);
    return HDL_MODULE_UNLOADED;
  }
}

static hdl_module_state_t _hdl_clock_pll(hdl_clock_private_t *clk, uint8_t enable) {
  while (enable) {
    if (clk->module.dependencies == NULL || clk->module.dependencies[0] == NULL) break;
    hdl_clock_private_t *clock_src = (hdl_clock_private_t *)clk->module.dependencies[0];
    uint32_t pll_cnf = clk->config->property.mul;
    if((pll_cnf < 2) || (pll_cnf > 32)) break;
    pll_cnf -= (pll_cnf > 15)? 1: 2;
    pll_cnf = ((pll_cnf & 0x0F) << 18) | ((pll_cnf & 0x10) << (27 - 4));
    HDL_REG_MODIFY(RCU_CFG0, (RCU_CFG0_PLLMF | RCU_CFG0_PLLMF4), pll_cnf);
    if(_hdl_clock_osc_en(RCU_PLL_CK, RCU_FLAG_PLLSTB, PLL_STARTUP_TIMEOUT) != HDL_MODULE_ACTIVE) {
      rcu_osci_off(RCU_PLL_CK);
      break;
    }
    hdl_clock_calc_mul(&clock_src->freq, clk->config->property.mul, &clk->freq);
    return HDL_MODULE_ACTIVE;
  }
  HDL_REG_CLEAR(RCU_CFG0, (RCU_CFG0_PLLMF | RCU_CFG0_PLLMF4));
  return HDL_MODULE_UNLOADED;
}

static hdl_module_state_t _hdl_clock_system_switch(uint32_t src) {
  /* Register start pos 2 bit */
  static const uint32_t rcu_cfg0_scss_shift = 2;
  rcu_system_clock_source_config(src);
  uint32_t stb_cnt = CK_SYS_STARTUP_TIMEOUT;
  while((src != ((RCU_CFG0 & RCU_CFG0_SCSS) >> rcu_cfg0_scss_shift)) && stb_cnt--);
  return (src != ((RCU_CFG0 & RCU_CFG0_SCSS) >> rcu_cfg0_scss_shift)) ? HDL_MODULE_FAULT: HDL_MODULE_ACTIVE;
}

static hdl_module_state_t _hdl_clock_system(hdl_clock_private_t *clk, uint8_t enable) {
  if (enable) {
    if (clk->module.dependencies == NULL || 
        clk->module.dependencies[0] == NULL || 
        clk->module.dependencies[1] == NULL)
      return HDL_MODULE_FAULT;
    hdl_clock_private_t *clock_src = (hdl_clock_private_t *)clk->module.dependencies[2];
    if(clock_src == NULL) clock_src = (hdl_clock_private_t *)clk->module.dependencies[1];
    hdl_clock_calc_div(&clock_src->freq, 1, &clk->freq);
    if(clock_src->config->type == HDL_CLOCK_TYPE_PLL) {
      return _hdl_clock_system_switch(RCU_CKSYSSRC_PLL);
    }
    else if(clock_src->config->type == HDL_CLOCK_TYPE_HXTAL) {
      return _hdl_clock_system_switch(RCU_CKSYSSRC_HXTAL);
    }
    else if(clock_src->config->type == HDL_CLOCK_TYPE_IRC8M) {
      return _hdl_clock_system_switch(RCU_CKSYSSRC_IRC8M);
    }
    /* TODO: config clock mon */
  }
  _hdl_clock_system_switch(RCU_CKSYSSRC_IRC8M);
  return HDL_MODULE_UNLOADED;
}


static hdl_module_state_t _hdl_bus_clock_cnf(hdl_clock_private_t *clk, uint32_t bit_from, uint32_t bit_to, 
                                             uint32_t check_frec, hdl_clock_type_t base_clock) {
  uint32_t factor = clk->config->property.div;
  if(factor >= 64) factor >>= 1;
  if ((factor == 0) || (clk->module.dependencies == NULL) || (clk->module.dependencies[0] == NULL))
    return HDL_MODULE_FAULT;
  hdl_clock_private_t *clock_src = (hdl_clock_private_t *)clk->module.dependencies[0];
  if(clock_src->config->type != base_clock) return HDL_MODULE_FAULT;
  hdl_clock_calc_div(&clock_src->freq, factor, &clk->freq);
  if((clk->freq.num / clk->freq.denom) > check_frec) return HDL_MODULE_FAULT;
  uint32_t div_cnf = 31 - __CLZ(factor);
  if (div_cnf) div_cnf = ((div_cnf - 1) << bit_from) | (1UL << bit_to);
  HDL_REG_MODIFY(RCU_CFG0, BITS(bit_from, bit_to), div_cnf);
  return HDL_MODULE_ACTIVE;
}

static hdl_module_state_t hdl_clock_selector_rtc(hdl_clock_private_t *clk, uint8_t enable) {
  if (enable) {
    if (clk->module.dependencies == NULL || clk->module.dependencies[0] == NULL)
      return HDL_MODULE_FAULT;
    hdl_clock_private_t *clock_src = (hdl_clock_private_t *)clk->module.dependencies[0];
    if(clock_src->config->type == HDL_CLOCK_TYPE_HXTAL) {
      /* RTC is cloking by HXTAL / 32 */
      rcu_rtc_clock_config(RCU_RTCSRC_HXTAL_DIV32);
      hdl_clock_calc_div(&clock_src->freq, 32, &clk->freq);
    }
    else if(clock_src->config->type == HDL_CLOCK_TYPE_LXTAL) {
      rcu_rtc_clock_config(RCU_RTCSRC_LXTAL);
      hdl_clock_calc_div(&clock_src->freq, 1, &clk->freq);
    }
    else if (clock_src->config->type == HDL_CLOCK_TYPE_IRC40K) {
      rcu_rtc_clock_config(RCU_RTCSRC_IRC40K);
      hdl_clock_calc_div(&clock_src->freq, 1, &clk->freq);
    }
    else {
      return HDL_MODULE_FAULT;
    }
    return HDL_MODULE_ACTIVE;
  }
  else {
    rcu_rtc_clock_config(RCU_RTCSRC_NONE);
    return HDL_MODULE_UNLOADED;
  }
}

hdl_module_state_t hdl_clock(void *desc, uint8_t enable) {
  hdl_clock_private_t *clk = (hdl_clock_private_t *)desc;
  clk->freq.denom = 1;
  switch (clk->config->type) {
    case HDL_CLOCK_TYPE_RTC_SEL:
      return hdl_clock_selector_rtc(clk, enable);

    case HDL_CLOCK_TYPE_HXTAL:
      clk->freq.num = clk->config->property.freq;
      if (enable) return _hdl_clock_osc_en(RCU_HXTAL, RCU_FLAG_HXTALSTB, HXTAL_STARTUP_TIMEOUT);
      return HDL_MODULE_UNLOADED;
    
    case HDL_CLOCK_TYPE_LXTAL:
      clk->freq.num = clk->config->property.freq;
      if (enable) return _hdl_clock_osc_en(RCU_LXTAL, RCU_FLAG_LXTALSTB, LXTAL_STARTUP_TIMEOUT);
      return HDL_MODULE_UNLOADED;

    case HDL_CLOCK_TYPE_IRC8M:
      clk->freq.num = 8000000;
      if (enable) return _hdl_clock_osc_en(RCU_IRC8M, RCU_FLAG_IRC8MSTB, IRC8M_STARTUP_TIMEOUT);
      return HDL_MODULE_UNLOADED;
    
    case HDL_CLOCK_TYPE_PLL_SEL:
      return _hdl_clock_selector_pll(clk, enable);

    case HDL_CLOCK_TYPE_PLL:
      return _hdl_clock_pll(clk, enable);

    case HDL_CLOCK_TYPE_SYS_SEL:
      return _hdl_clock_system(clk, enable);

    case HDL_CLOCK_TYPE_AHB:
      if (enable) {
        return _hdl_bus_clock_cnf(clk, 4, 7, MAX_SYS_CLOCK, HDL_CLOCK_TYPE_SYS_SEL);
      }
      rcu_ahb_clock_config(RCU_AHB_CKSYS_DIV512);
      return HDL_MODULE_UNLOADED;

    case HDL_CLOCK_TYPE_APB1:
      if (enable) return _hdl_bus_clock_cnf(clk, 10, 12, MAX_APB1_CLOCK, HDL_CLOCK_TYPE_AHB);
      rcu_apb1_clock_config(RCU_APB1_CKAHB_DIV16);
      return HDL_MODULE_UNLOADED;

    case HDL_CLOCK_TYPE_APB2:
      if (enable) return _hdl_bus_clock_cnf(clk, 13, 15, MAX_APB2_CLOCK, HDL_CLOCK_TYPE_AHB);
      rcu_apb2_clock_config(RCU_APB2_CKAHB_DIV16);
      return HDL_MODULE_UNLOADED;

    case HDL_CLOCK_TYPE_IRC28M:
      clk->freq.num = 28000000;
      if (enable) return _hdl_clock_osc_en(RCU_IRC28M, RCU_FLAG_IRC28MSTB, IRC28M_STARTUP_TIMEOUT);
      return HDL_MODULE_UNLOADED;

    case HDL_CLOCK_TYPE_IRC40K:
      clk->freq.num = 40000;
      if (enable) return _hdl_clock_osc_en(RCU_IRC40K, RCU_FLAG_IRC40KSTB, IRC40K_STARTUP_TIMEOUT);
      return HDL_MODULE_UNLOADED;

    default:
      break;
  }
  return HDL_MODULE_FAULT;
}

void hdl_get_clock(hdl_clock_t *clock, hdl_clock_freq_t *freq) {
  if(freq != NULL) {
    freq->num = 0;
    freq->denom = 1;
    hdl_clock_private_t *clk = (hdl_clock_private_t *)clock;
    if((clk != NULL) && (hdl_state(&clk->module) != HDL_MODULE_FAULT)) {
      freq->num = clk->freq.num;
      freq->denom = clk->freq.denom;
    }
  }
}

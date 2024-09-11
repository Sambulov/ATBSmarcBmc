#include "app.h"

const hdl_uspd_ain_port_config_t hdl_uspd_ain_port_cnf_default = {
  .circuit_config = USPD20K_CIRCUIT_CONFIG_FLOATING
};

hdl_uspd_ain_port_t hdl_uspd_ain_port1 = {
  .module.init = &hdl_uspd_ain_port,
  .module.dependencies = hdl_module_dependencies(
    &uspd20k_ai1_ntc_pu.module, &uspd20k_ai1_cur_scr_high.module, &uspd20k_ai1_cur_scr_low.module, 
    &uspd20k_ai1_4K3_pd.module, &uspd20k_ai1_150r_pd.module, &uspd20k_ai1_1k_pd.module),
  .config = &hdl_uspd_ain_port_cnf_default
};

hdl_uspd_ain_port_t hdl_uspd_ain_port2 = {
  .module.init = &hdl_uspd_ain_port,
  .module.dependencies = hdl_module_dependencies(
    &uspd20k_ai2_ntc_pu.module, &uspd20k_ai2_cur_scr_high.module, &uspd20k_ai2_cur_scr_low.module, 
    &uspd20k_ai2_4K3_pd.module, &uspd20k_ai2_150r_pd.module, &uspd20k_ai2_1k_pd.module),
  .config = &hdl_uspd_ain_port_cnf_default
};

hdl_uspd_ain_port_t hdl_uspd_ain_port3 = {
  .module.init = &hdl_uspd_ain_port,
  .module.dependencies = hdl_module_dependencies(
    &uspd20k_ai3_ntc_pu.module, &uspd20k_ai3_cur_scr_high.module, &uspd20k_ai3_cur_scr_low.module, 
    &uspd20k_ai3_4K3_pd.module, &uspd20k_ai3_150r_pd.module, &uspd20k_ai3_1k_pd.module),
  .config = &hdl_uspd_ain_port_cnf_default
};

hdl_uspd_ain_port_t hdl_uspd_ain_port4 = {
  .module.init = &hdl_uspd_ain_port,
  .module.dependencies = hdl_module_dependencies(    
    &uspd20k_ai4_ntc_pu.module, &uspd20k_ai4_cur_scr_high.module, &uspd20k_ai4_cur_scr_low.module, 
    &uspd20k_ai4_4K3_pd.module, &uspd20k_ai4_150r_pd.module, &uspd20k_ai4_1k_pd.module),
  .config = &hdl_uspd_ain_port_cnf_default
};

hdl_adc_ms5194t_source_t mod_adc_src_1 = {
  .config_reg = MS5194T_CONFIG_REG_VBIAS_DISBL | !MS5194T_CONFIG_REG_BO | MS5194T_CONFIG_REG_UB | 
                !MS5194T_CONFIG_REG_BOOST | MS5194T_CONFIG_REG_IN_AMP(0) | MS5194T_CONFIG_REG_REFSEL_EXTIN1 |
                !MS5194T_CONFIG_REG_REF_DET | MS5194T_CONFIG_REG_BUF | MS5194T_CONFIG_REG_CH_SEL(1)
};

hdl_adc_ms5194t_source_t mod_adc_src_2 = {
  .config_reg = MS5194T_CONFIG_REG_VBIAS_DISBL | !MS5194T_CONFIG_REG_BO | MS5194T_CONFIG_REG_UB | 
                !MS5194T_CONFIG_REG_BOOST | MS5194T_CONFIG_REG_IN_AMP(0) | MS5194T_CONFIG_REG_REFSEL_EXTIN1 |
                !MS5194T_CONFIG_REG_REF_DET | MS5194T_CONFIG_REG_BUF | MS5194T_CONFIG_REG_CH_SEL(2)
};

hdl_adc_ms5194t_source_t mod_adc_src_3 = {
  .config_reg = MS5194T_CONFIG_REG_VBIAS_DISBL | !MS5194T_CONFIG_REG_BO | MS5194T_CONFIG_REG_UB | 
                !MS5194T_CONFIG_REG_BOOST | MS5194T_CONFIG_REG_IN_AMP(0) | MS5194T_CONFIG_REG_REFSEL_EXTIN1 |
                !MS5194T_CONFIG_REG_REF_DET | MS5194T_CONFIG_REG_BUF | MS5194T_CONFIG_REG_CH_SEL(3)
};

hdl_adc_ms5194t_source_t mod_adc_src_4 = {
  .config_reg = MS5194T_CONFIG_REG_VBIAS_DISBL | !MS5194T_CONFIG_REG_BO | MS5194T_CONFIG_REG_UB | 
                !MS5194T_CONFIG_REG_BOOST | MS5194T_CONFIG_REG_IN_AMP(0) | MS5194T_CONFIG_REG_REFSEL_EXTIN1 |
                !MS5194T_CONFIG_REG_REF_DET | MS5194T_CONFIG_REG_BUF | MS5194T_CONFIG_REG_CH_SEL(4)
};

hdl_adc_ms5194t_source_t mod_adc_src_5 = {
  .config_reg = MS5194T_CONFIG_REG_VBIAS_DISBL | !MS5194T_CONFIG_REG_BO | MS5194T_CONFIG_REG_UB | 
                !MS5194T_CONFIG_REG_BOOST | MS5194T_CONFIG_REG_IN_AMP(0) | MS5194T_CONFIG_REG_REFSEL_EXTIN1 |
                !MS5194T_CONFIG_REG_REF_DET | MS5194T_CONFIG_REG_BUF | MS5194T_CONFIG_REG_CH_SEL(5)
};

hdl_adc_ms5194t_source_t mod_adc_src_6 = {
  .config_reg = MS5194T_CONFIG_REG_VBIAS_DISBL | !MS5194T_CONFIG_REG_BO | MS5194T_CONFIG_REG_UB | 
                !MS5194T_CONFIG_REG_BOOST | MS5194T_CONFIG_REG_IN_AMP(0) | MS5194T_CONFIG_REG_REFSEL_EXTIN1 |
                !MS5194T_CONFIG_REG_REF_DET | MS5194T_CONFIG_REG_BUF | MS5194T_CONFIG_REG_CH_SEL(6)
};

const hdl_adc_ms5194t_config_t mod_adc_cnf = {
  .io_reg = MS5194T_IO_REG_DEFAULT,
  .mode_reg = MS5194T_MODE_REG_MS_PWR_DWN | !MS5194T_MODE_REG_PSW | !MS5194T_MODE_REG_AMP_CM | MS5194T_MODE_REG_CLK_INT64K |
              !MS5194T_MODE_REG_CHOP_DIS | MS5194T_MODE_REG_FILTER_RATE(5),
  .sources = hdl_adc_ms5194t_sources(&mod_adc_src_1, &mod_adc_src_2, &mod_adc_src_3, &mod_adc_src_4, &mod_adc_src_5, &mod_adc_src_6)
};

hdl_adc_ms5194t_t mod_adc = {
  .module.init = &hdl_adc_ms5194t,
  .module.dependencies = hdl_module_dependencies(&uspd20k_adc_spi.module, &uspd20k_adc_rdy.module, &mod_timer_ms.module),
  .config = &mod_adc_cnf
};

hdl_module_t mod_app = {
  .dependencies = hdl_module_dependencies(
    &mod_timer_ms.module, 
    
    &uspd20k_i2c_som.module, &uspd20k_som_int.module,

    &uspd20k_i2c_eeprom.module,

    &mod_adc.module,
    &hdl_uspd_ain_port1.module, &hdl_uspd_ain_port2.module, &hdl_uspd_ain_port3.module, &hdl_uspd_ain_port4.module)
};

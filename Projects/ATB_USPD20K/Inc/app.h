#ifndef APP_H_
#define APP_H_

#include "hdl.h"

#include "uspd_ain_port.h"
#include "som_link.h"
#include "median_filter.h"

#include "mig.h"

typedef struct {
  uint8_t *data_ptr;
  uint16_t map_addr_start;
  uint16_t map_addr_end;
} app_mem_map_item_t;

#define mem_map_item(data, addr, offset)     {.data_ptr = (uint8_t*)&data, .map_addr_start = addr + offset, .map_addr_end = addr + sizeof(data) - 1}


#define APP_AIN_PORTS_AMOUNT          4
#define APP_ADC_CHANNELS_AMOUNT       6

extern const uint8_t app_uspd_ain_port_to_adc_ch_map[APP_AIN_PORTS_AMOUNT];

typedef struct {
  hdl_adc_ms5194t_source_t src_active[APP_ADC_CHANNELS_AMOUNT];
  uint8_t adc_io_active;
  uint16_t adc_mode_active;
  hdl_adc_ms5194t_source_t src_user[APP_ADC_CHANNELS_AMOUNT];
  uint8_t adc_io_user;
  uint16_t adc_mode_user;
  uint32_t sync_key;
} app_adc_config_t;

#define ADC_CONFIG_SYNC_KEY       0xABCDEFAB

typedef struct {
  uint32_t ain[6];
  uint32_t timestamp;
  uint32_t valid;
} app_adc_measure_t;

#define MEASURE_VALID   0xCAFEFEED

typedef struct {
  hdl_uspd20k_circuit_cofig_t ai[APP_AIN_PORTS_AMOUNT];
} circuit_config_ports_t;

typedef struct {
  circuit_config_ports_t active;
  circuit_config_ports_t user;
  uint32_t sync_key;
} app_circuit_config_t;

#define CIRCUIT_CONFIG_SYNC_KEY       0x12345678

typedef struct {
  uint16_t current_address;
  uint8_t state;
} som_proto_context_t;

typedef struct {
  hdl_uspd20k_circuit_cofig_t circuit_config;
  hdl_adc_ms5194t_source_t ch_config;
} app_adc_config_preset_t;

typedef struct {
  uint8_t port_preset_selection[APP_AIN_PORTS_AMOUNT];
  uint32_t sync_key;
} app_adc_preset_config_t;

#define PRESET_CONFIG_SYNC_KEY       0x87654321

#define MAP_ADDR_ADC_LOG             0x0000
#define MAP_ADDR_CIRCUIT_CONFIG      0x1000
#define MAP_ADDR_ADC_CONFIG          0x1100
#define MAP_ADDR_ADC_PRESET_CONFIG   0x1200


#define APP_ADC_LOG_SIZE    64

extern bldl_uspd_ain_port_t app_uspd_ain_port[APP_AIN_PORTS_AMOUNT];

extern hdl_module_t mod_app;

extern hdl_eeprom_i2c_t mod_eeprom;

extern app_adc_config_t adc_config;
extern hdl_adc_ms5194t_config_t mod_adc_cnf;

extern app_circuit_config_t ai_circuit_config;
extern app_adc_measure_t adc_log[APP_ADC_LOG_SIZE];
extern app_adc_measure_t primary_filter_val;
extern app_adc_measure_t secondary_filter_val;

#define APP_ADC_PRESETS_AMOUNT   7

extern const app_adc_config_preset_t *app_adc_presets[APP_ADC_PRESETS_AMOUNT];
extern app_adc_preset_config_t app_adc_preset_config;

#define PRIMARY_FILTER_SIZE  16
#define SECONDARY_FILTER_SIZE  8
#define MAP_ADDR_SECONDARY_FILTER_ADC_VAL      0x950
#define MAP_ADDR_PRIMARY_FILTER_ADC_VAL      0x900


#define APP_R_MEM_MAP_SIZE       6
#define APP_W_MEM_MAP_SIZE       3

extern const app_mem_map_item_t app_r_mem_map[APP_R_MEM_MAP_SIZE];
extern const app_mem_map_item_t app_w_mem_map[APP_W_MEM_MAP_SIZE];


void main();

#endif /* APP_H_ */

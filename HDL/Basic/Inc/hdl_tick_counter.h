#ifndef HDL_TICK_COUNTER_H_
#define HDL_TICK_COUNTER_H_

#include "port_tick_counter.h"

typedef struct {
  hdl_module_t module;
  const void *config;
} hdl_tick_counter_t;

hdl_module_state_t hdl_tick_counter(void *desc, const uint8_t enable);
uint32_t hdl_tick_counter_get(hdl_tick_counter_t *desc);

#endif /* HDL_TICK_COUNTER_H_ */ 

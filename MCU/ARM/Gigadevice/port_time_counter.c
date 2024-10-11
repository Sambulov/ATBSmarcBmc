#include "hdl_portable.h"

typedef struct {
  hdl_module_t module;
  const hdl_time_counter_config_t *config;
  struct {
    hdl_delegate_t reload_isr;
    uint32_t count;
  } private;  

} hdl_time_counter_private_t;

HDL_ASSERRT_STRUCTURE_CAST(hdl_time_counter_private_t, hdl_time_counter_t, HDL_TIME_COUNTER_PRV_SIZE, hdl_time_counter.h);

static void event_timer_isr(uint32_t event, void *sender, void *context) {
  ((hdl_time_counter_private_t *)context)->private.count++;
}

hdl_module_state_t hdl_time_counter(void *desc, const uint8_t enable) {
  hdl_time_counter_private_t *timer = (hdl_time_counter_private_t *)desc;
  if(enable) {
    hdl_interrupt_controller_t *ic = (hdl_interrupt_controller_t *)timer->module.dependencies[1];
    timer->private.reload_isr.context = desc;
    timer->private.reload_isr.handler = &event_timer_isr;
    timer->private.count = 0;
    if(hdl_interrupt_request(ic, timer->config->reload_interrupt, &timer->private.reload_isr))
      return HDL_MODULE_ACTIVE;
  }
  return HDL_MODULE_UNLOADED;
}

uint32_t hdl_time_counter_get(hdl_time_counter_t *desc) {
  if((desc != NULL) && (hdl_state(&desc->module) == HDL_MODULE_ACTIVE)) {
    return ((hdl_time_counter_private_t *)desc)->private.count;
  }
  return 0;
}

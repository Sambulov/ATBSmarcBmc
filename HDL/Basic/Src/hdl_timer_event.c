#include "hdl.h"
#include "CodeLib.h"

typedef enum {
  HDL_BTN_O_DEFAULT = 0,
  HDL_BTN_O_RELEASE,
  HDL_BTN_O_PRESS,
  HDL_BTN_O_CLICK_PRESS,
  HDL_BTN_O_CLICK_DEBOUNCE,
} hdl_btn_output_state_t;

typedef struct {
  hdl_module_t module;
  hdl_event_t event;
  uint32_t time_stamp;
  uint32_t delay;
  hdl_timer_event_mode_t mode;
  coroutine_desc_static_t timer_events_worker;
} hdl_timer_event_private_t;

_Static_assert(sizeof(hdl_timer_event_private_t) == sizeof(hdl_timer_event_t), "In hdl_timer_event.h data structure size of hdl_button_t doesn't match, check HDL_TIMER_EVENT_PRV_SIZE");
_Static_assert(offsetof(hdl_timer_event_private_t, event) == offsetof(hdl_timer_event_t, event), "In hdl_timer_event.h hdl_timer_event_t properties order doesn't match");

static uint8_t _timer_events_handler(coroutine_desc_t this, uint8_t cancel, void *arg) {
  hdl_timer_event_private_t *timer_event = (hdl_timer_event_private_t *)arg;
  hdl_timer_t *timer = (hdl_timer_t *)timer_event->module.dependencies[0];
  if(timer_event->mode != HDL_TIMER_EVENT_IDLE) {
    uint32_t now = hdl_timer_get(timer);
    if(TIME_ELAPSED(timer_event->time_stamp, timer_event->delay, now)) {
      timer_event->time_stamp += timer_event->delay;
      if(timer_event->mode == HDL_TIMER_EVENT_SINGLE) {
        timer_event->mode = HDL_TIMER_EVENT_IDLE;
      }
      hdl_event_raise(&timer_event->event, (void *)timer_event, 0);
    }
  }
  return cancel;
}

hdl_module_state_t hdl_timer_event(void *desc, uint8_t enable) {
  hdl_timer_event_private_t *timer_event = (hdl_timer_event_private_t *)desc;
  if(desc != NULL) {
    if(enable) {
      timer_event->mode = HDL_TIMER_EVENT_IDLE;
      coroutine_add_static(&timer_event->timer_events_worker, &_timer_events_handler, (void*)timer_event);
      return HDL_MODULE_INIT_OK;
    }
    coroutine_cancel(&timer_event->timer_events_worker);
  }
  return HDL_MODULE_DEINIT_OK;
}

uint8_t hdl_timer_event_set(hdl_timer_event_t *timer, uint32_t delay, hdl_timer_event_mode_t mode) {
  hdl_timer_event_private_t *timer_event = (hdl_timer_event_private_t *)timer;
  if((timer_event != NULL) && (hdl_state(&timer_event->module) == HDL_MODULE_INIT_OK)) {
    hdl_timer_t *timer = (hdl_timer_t *)timer_event->module.dependencies[0];
    timer_event->time_stamp = hdl_timer_get(timer);
    timer_event->mode = mode;
    timer_event->delay = delay;
    return HDL_TRUE;
  }
  return HDL_FALSE;
}

hdl_timer_event_mode_t hdl_timer_event_mode(hdl_timer_event_t *timer) {
  hdl_timer_event_private_t *timer_event = (hdl_timer_event_private_t *)timer;
  if((timer_event != NULL) && (hdl_state(&timer_event->module) == HDL_MODULE_INIT_OK)) {
    return timer_event->mode;
  }
  return HDL_TIMER_EVENT_IDLE;
}

uint32_t hdl_timer_event_time_left(hdl_timer_event_t *timer) {
  hdl_timer_event_private_t *timer_event = (hdl_timer_event_private_t *)timer;
  if((timer_event != NULL) && (hdl_state(&timer_event->module) == HDL_MODULE_INIT_OK)) {
    hdl_timer_t *timer = (hdl_timer_t *)timer_event->module.dependencies[0];
    return timer_event->delay - (timer_event->mode != HDL_TIMER_EVENT_IDLE)? (hdl_timer_get(timer) - timer_event->time_stamp): 0;
  }
  return 0;
}

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
  uint32_t debounce_delay;
  uint32_t hold_delay;
  hdl_event_t event;
  hdl_btn_state_t input_state;
  hdl_btn_output_state_t output_state;
  uint32_t output_change_time;
  uint32_t input_change_time;
  __linked_list_object__;
} hdl_button_private_t;

_Static_assert(sizeof(hdl_button_private_t) == sizeof(hdl_button_t), "In hdl_button.h data structure size of hdl_button_t doesn't match, check HDL_BUTTON_DESC_SIZE");
_Static_assert(offsetof(hdl_button_private_t, event) == offsetof(hdl_button_t, event), "In hdl_button.h hdl_button_t properties order doesn't match");


static void _button_handler(LinkedListItem_t *item, void *arg) {
  hdl_button_private_t *btn = linked_list_get_object(hdl_button_private_t, item);
  hdl_gpio_pin_t *btn_gpio = (hdl_gpio_pin_t *)btn->module.dependencies[0];
  hdl_timer_t *btn_timer = (hdl_timer_t *)btn->module.dependencies[1];
  switch (btn->output_state) {
    case HDL_BTN_O_CLICK_PRESS:
      btn->output_state = HDL_BTN_O_CLICK_DEBOUNCE;
      btn->output_change_time = hdl_timer_get(btn_timer);
    case HDL_BTN_O_PRESS:
      hdl_gpio_write(btn_gpio, !btn_gpio->inactive_default);
      break;
    case HDL_BTN_O_CLICK_DEBOUNCE: {
      uint8_t debounce_timeout = TIME_ELAPSED(btn->output_change_time, btn->debounce_delay, hdl_timer_get(btn_timer));
      if(debounce_timeout) {
        btn->output_state = HDL_BTN_O_RELEASE;
      }
      break;
    }
    case HDL_BTN_O_RELEASE:
      hdl_gpio_write(btn_gpio, btn_gpio->inactive_default);
      btn->output_state = HDL_BTN_O_DEFAULT;
    case HDL_BTN_O_DEFAULT:
    default:
      break;
  }

  uint8_t btn_active = 0;
  btn_active = (hdl_gpio_read(btn_gpio) != btn_gpio->inactive_default);
  switch (btn->input_state) {
    case HDL_BTN_RELEASED:
      if(!btn_active) 
        break;
      btn->input_change_time = hdl_timer_get(btn_timer);
      btn->input_state = HDL_BTN_PRESS_DEBOUNCE;
    case HDL_BTN_PRESS_DEBOUNCE: {
      if(!btn_active) {
        btn->input_state = HDL_BTN_RELEASED;
        break;
      }
      uint8_t debounce_timeout = TIME_ELAPSED(btn->input_change_time, btn->debounce_delay, hdl_timer_get(btn_timer));
      if(debounce_timeout) {
        btn->input_state = HDL_BTN_PRESSED;
        hdl_event_raise(&btn->event, (void *)btn, HDL_BTN_EVENT_PRESS);
      }
    }
    case HDL_BTN_PRESSED:
      if(btn_active) {
        uint8_t hold_timeout = btn->hold_delay && TIME_ELAPSED(btn->input_change_time, btn->hold_delay, hdl_timer_get(btn_timer));
        if(hold_timeout) {
          btn->input_state = HDL_BTN_HOLDING;
          hdl_event_raise(&btn->event, (void *)btn, HDL_BTN_EVENT_HOLD);
        }
      }
      else {
        btn->input_state = HDL_BTN_RELEASED;
        hdl_event_raise(&btn->event, (void *)btn, HDL_BTN_EVENT_RELEASE);
        hdl_event_raise(&btn->event, (void *)btn, HDL_BTN_EVENT_CLICK);
      }
      break;
    case HDL_BTN_HOLDING:
    default:
      if(!btn_active) {
        btn->input_state = HDL_BTN_RELEASED;
        hdl_event_raise(&btn->event, (void *)btn, HDL_BTN_EVENT_RELEASE);
      }
      break;
  }
}

static uint8_t _buttons_handler(coroutine_desc_t this, uint8_t cancel, void *arg) {
  linked_list_t btn_list = (linked_list_t)arg;
  linked_list_do_foreach(btn_list, &_button_handler, NULL);
  return cancel || (btn_list == NULL);
}

hdl_module_state_t hdl_button(void *desc, uint8_t enable) {
  static coroutine_desc_static_t buttons_worker;
  static linked_list_t buttons;
  hdl_button_private_t *btn = (hdl_button_private_t *)desc;
  if(desc != NULL) {
    if(enable) {
      btn->input_state = HDL_BTN_RELEASED;
      btn->output_state = HDL_BTN_O_DEFAULT;
      linked_list_insert_last(&buttons, linked_list_item(btn));
      coroutine_add_static(&buttons_worker, &_buttons_handler, (void*)buttons);
      return HDL_MODULE_INIT_OK;
    }
    linked_list_unlink(linked_list_item(btn));
  }
  return HDL_MODULE_DEINIT_OK;
}

hdl_btn_state_t hdl_btn_state_get(hdl_button_t *desc) {
  hdl_button_private_t *btn = (hdl_button_private_t *)desc;
  return (desc != NULL)? btn->input_state: HDL_BTN_UNKNOWN;
}

static uint8_t _hdl_btn_sw_event(hdl_button_private_t *btn, hdl_btn_output_state_t check_state, hdl_btn_output_state_t set_state) {
  if((btn == NULL) || (btn->output_state != check_state))
    return HDL_FALSE;
  btn->output_state = set_state;
  return HDL_TRUE;
}

uint8_t hdl_btn_sw_press(hdl_button_t *desc) {
  return _hdl_btn_sw_event((hdl_button_private_t *)desc, HDL_BTN_O_DEFAULT, HDL_BTN_O_PRESS);
}

uint8_t hdl_btn_click(hdl_button_t *desc) {
  return _hdl_btn_sw_event((hdl_button_private_t *)desc, HDL_BTN_O_DEFAULT, HDL_BTN_O_CLICK_PRESS);
}

uint8_t hdl_btn_release(hdl_button_t *desc) {
  return _hdl_btn_sw_event((hdl_button_private_t *)desc, HDL_BTN_O_PRESS, HDL_BTN_O_RELEASE);
}

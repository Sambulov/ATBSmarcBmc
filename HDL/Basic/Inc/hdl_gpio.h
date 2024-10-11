#ifndef HDL_GPIO_H_
#define HDL_GPIO_H_

#include "port_gpio.h"

typedef enum {
  HDL_GPIO_LOW = 0,
  HDL_GPIO_HIGH = !HDL_GPIO_LOW
} hdl_gpio_state;

_Static_assert(HDL_GPIO_LOW == !HDL_GPIO_HIGH, "Expression (HDL_GPIO_LOW == !HDL_GPIO_HIGH) must always be true");

typedef struct {
  const hdl_gpio_port_hw_config_t *hwc;
} hdl_gpio_port_config_t;

#define hdl_gpio_port_config(...) (hdl_gpio_port_config_t []){{__VA_ARGS__}}

typedef struct {
  hdl_module_t module;
  hdl_gpio_port_config_t *config;
} hdl_gpio_port_t;

typedef struct {
  hdl_gpio_state inactive_default;
  const hdl_gpio_pin_hw_config_t *hwc;
} hdl_gpio_pin_config_t;

#define hdl_gpio_pin_config(...) (const hdl_gpio_pin_config_t []){{__VA_ARGS__}}

typedef struct {
  hdl_module_t module;
  const hdl_gpio_pin_config_t *config;
} hdl_gpio_pin_t;

#define hdl_gpio_set_inactive(gpio)            (hdl_gpio_write((gpio), (gpio)->config->inactive_default))
#define hdl_gpio_set_active(gpio)              (hdl_gpio_write((gpio), !((gpio)->config->inactive_default)))
#define hdl_gpio_is_inactive(gpio)             (hdl_gpio_read((gpio)) == (gpio)->config->inactive_default)
#define hdl_gpio_is_active(gpio)               (!hdl_gpio_is_inactive(gpio))
#define hdl_gpio_is_set_as_inactive(gpio)      (hdl_gpio_read_output(gpio) == gpio->config->inactive_default)
#define hdl_gpio_is_set_as_active(gpio)        (!hdl_gpio_is_set_as_inactive(gpio))

hdl_module_state_t hdl_gpio_pin(void *desc, const uint8_t enable);
hdl_module_state_t hdl_gpio_port(void *desc, const uint8_t enable);

hdl_gpio_state hdl_gpio_read(const hdl_gpio_pin_t *gpio);
hdl_gpio_state hdl_gpio_read_output(const hdl_gpio_pin_t *gpio);
void hdl_gpio_write(const hdl_gpio_pin_t *gpio, const hdl_gpio_state state);
void hdl_gpio_toggle(const hdl_gpio_pin_t *gpio);

#endif // HDL_GPIO_H_

#ifndef BLDL_USPD_IO_H_
#define BLDL_USPD_IO_H_

/* USPD discrete open draine port depends on:
- gpio input pin (direct);
- gpio output pin (inverse);
*/
typedef hdl_plc_port_t bldl_uspd_discrete_port_t;

/* USPD analog input port depends on:
- analog input pin;
- discrete output pin (active high) pull up control;
- discrete output pin (active high) pull down control;
- discrete output pin (active high) current shunt control;
*/
typedef hdl_plc_port_t bldl_uspd_ain_port_t;

/* USPD led port is discrete output pin (active low)
*/
typedef hdl_gpio_pin_t bldl_uspd_led_port_t;

hdl_module_state_t bldl_uspd_ain_port(void *desc, uint8_t enable);
hdl_module_state_t bldl_uspd_discrete_port(void *desc, uint8_t enable);

#endif // BLDL_USPD_IO_H_

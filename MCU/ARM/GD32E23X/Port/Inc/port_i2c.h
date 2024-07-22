#ifndef PORT_I2C_H_
#define PORT_I2C_H_

#define HDL_I2C_CLIENT_PRV_SIZE                    105

typedef struct {
  const uint32_t dtcy;         /* I2C_DTCY_2 or I2C_DTCY_16_9 */
  const uint32_t speed;        /* Max 1000000 */
  const uint8_t general_call_enable : 1,
                addr_10_bits        : 1,
                dual_address        : 1,
                stretch_enable      : 1;
  const uint16_t addr0;
  const uint16_t addr1;
  const hdl_nvic_irq_n_t err_interrupt;
  const hdl_nvic_irq_n_t ev_interrupt;
} hdl_i2c_config_t;


#endif // PORT_I2C_H_

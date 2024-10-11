#ifndef HDL_EEPROM_I2C_H_
#define HDL_EEPROM_I2C_H_

#define HDL_EEPROM_I2C_PRV_SIZE  52

typedef struct {
  uint16_t chip_address;
  uint16_t block_size;
  uint32_t write_time;
  uint32_t size;
} hdl_eeprom_i2c_config_t;

/* depends on
  hdl_i2c
  hdl_time_counter
*/
typedef struct {
  hdl_module_t module;
  const hdl_eeprom_i2c_config_t *config;
  PRIVATE(hdl, HDL_EEPROM_I2C_PRV_SIZE);
} hdl_eeprom_i2c_t;

typedef enum {
  HDL_EEPROM_MSG_OPTION_READ       = 0x00,
  HDL_EEPROM_MSG_OPTION_WRITE      = 0x01,
  HDL_EEPROM_MSG_STATUS_PROCESSING = 0x02,
  HDL_EEPROM_MSG_STATUS_COMPLETE   = 0x08,
  HDL_EEPROM_MSG_ERROR_NO_RESPONSE = 0x10,
  HDL_EEPROM_MSG_ERROR_BUS         = 0x20,
  HDL_EEPROM_MSG_ERROR_NACK        = 0x40,
} hdl_eeprom_i2c_data_state_t;

typedef struct {
  uint8_t *buffer;
  uint32_t size;
  uint32_t transferred;
  uint16_t address;
  hdl_eeprom_i2c_data_state_t state;
} hdl_eeprom_i2c_data_t;

hdl_module_state_t hdl_eeprom_i2c(void *desc, uint8_t enable);

uint8_t hdl_eeprom_i2c_read(hdl_eeprom_i2c_t *desc, hdl_eeprom_i2c_data_t *data);
uint8_t hdl_eeprom_i2c_write(hdl_eeprom_i2c_t *desc, hdl_eeprom_i2c_data_t *data);

#endif /* HDL_EEPROM_I2C_H_ */

#include "bldl.h"

void bldl_som_boot_dev_en(const bldl_som_boot_lock_t *desc, const bldl_boot_dev_t dev) {
  static uint8_t init = 1;
  if(desc == NULL)
    return;
  if(init) {
    hdl_gpio_init(desc->lock_pin_sd);
    hdl_gpio_init(desc->lock_pin_emmc);
    hdl_gpio_init(desc->lock_pin_qspi);
    init = 0;
  }
  hdl_gpio_write(desc->lock_pin_sd, ((dev & BLDL_BOOT_DEV_SD) != 0)? HDL_GPIO_LOW: HDL_GPIO_HIGH);
  hdl_gpio_write(desc->lock_pin_emmc, ((dev & BLDL_BOOT_DEV_EMMC) != 0)? HDL_GPIO_LOW: HDL_GPIO_HIGH);
  hdl_gpio_write(desc->lock_pin_qspi, ((dev & BLDL_BOOT_DEV_SPI) != 0)? HDL_GPIO_LOW: HDL_GPIO_HIGH);
}
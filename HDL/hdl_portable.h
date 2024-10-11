/* 
    HAL dependent methods

*/

#ifndef HDL_PORTABLE_H_
#define HDL_PORTABLE_H_

#define HDL_FALSE         0
#define HDL_TRUE          !HDL_FALSE

#define PRIVATE(prefix, size)                          uint32_t __##prefix##_private[(size + sizeof(void *) - 1) >> 2]

#define HDL_ASSERRT_STRUCTURE_CAST(private_type, public_type, prv_size_def, def_file)   _Static_assert(sizeof(private_type) == sizeof(public_type), "In "#def_file" data structure size of "#public_type" doesn't match, check "#prv_size_def)

#include <stddef.h>
#include __MCU_HAL_HDR__

#include "hdl_transceiver.h"
#include "hdl_buffer.h"
#include "hdl_event.h"
#include "hdl_module.h"
#include "hdl_core.h"
#include "hdl_clock.h"
#include "hdl_dma.h"
#include "hdl_adc.h"
#include "hdl_tick_counter.h"
#include "hdl_time_counter.h"
#include "hdl_gpio.h"
#include "hdl_spi.h"
#include "hdl_i2c.h"
#include "hdl_uart.h"

#endif // HDL_PORTABLE_H_

/*
    Harware Driver Layer
    
*/

#ifndef HDL_H_
#define HDL_H_

#ifdef __cplusplus
extern "C" {
#endif

#define HDL_FALSE   0
#define HDL_TRUE    !HDL_FALSE

#define HDL_REG32(addr) ((volatile uint32_t *)(uint32_t)(addr))
#define HDL_REG16(addr) ((volatile uint16_t *)(uint32_t)(addr))

#define BITS_COUNT_U16(x)               ((x >> 0 ) & 1) + ((x >> 1 ) & 1) + ((x >> 2 ) & 1) + ((x >> 3 ) & 1) + \
                                        ((x >> 4 ) & 1) + ((x >> 5 ) & 1) + ((x >> 6 ) & 1) + ((x >> 7 ) & 1) + \
                                        ((x >> 8 ) & 1) + ((x >> 9 ) & 1) + ((x >> 10) & 1) + ((x >> 11) & 1) + \
                                        ((x >> 12) & 1) + ((x >> 13) & 1) + ((x >> 14) & 1) + ((x >> 15) & 1)

#include "hdl_portable.h"

#include "hdl_button.h"
#include "hdl_shift_reg.h"
#include "hdl_timer.h"

#include "hdl_eeprom_i2c.h"
#include "ms5194t.h"

#include "RK809.h"
#include "smarc_carrier.h"

#ifdef __cplusplus
}
#endif

void test(void);

#endif // HDL_H_

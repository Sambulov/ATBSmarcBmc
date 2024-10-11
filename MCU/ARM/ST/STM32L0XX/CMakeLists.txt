

file(GLOB MCU_PORT
  "./Port/*.c"
  "./Port/Inc/*.h"
  "./Sys/*.c"
  "./Sys/Inc/*.h"
  "./Drivers/Src/*.c"
  "./Drivers/Inc/*.h"
)

target_sources(McuHalPort PUBLIC ${MCU_PORT})

add_compile_definitions("__MCU_HAL_HDR__=<stm32l0xx_hal.h>")

set(LINKER_SCRIPT "./Res/linkerscript.ld")

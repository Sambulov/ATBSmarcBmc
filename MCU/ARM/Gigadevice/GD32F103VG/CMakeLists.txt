

file(GLOB MCU_PORT
  "./Port/*.c"
  "./Port/Inc/*.h"
  "./Sys/*.c"
  "./Sys/Inc/*.h"
  "./Drivers/Src/*.c"
  "./Drivers/Inc/*.h"
)

target_sources(McuHalPort PUBLIC ${MCU_PORT})

add_compile_definitions("__MCU_HAL_HDR__=<gd32f10x.h>") 

set(LINKER_SCRIPT "./Res/linkerscript.ld")

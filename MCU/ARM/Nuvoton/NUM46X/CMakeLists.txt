

file(GLOB MCU_PORT
  "./Port/*.c"
  "./Port/Inc/*.h"
  "./Sys/*.c"
  "./Sys/Inc/*.h"
  "./Drivers/Src/*.c"
  "./Drivers/Inc/*.h"
)

target_sources(McuHalPort PUBLIC ${MCU_PORT})

add_compile_definitions("__MCU_HAL_HDR__=<m460.h>")

set(LINKER_SCRIPT "./Res/linkerscript.ld")

# Uncomment for hardware floating point
# add_compile_options(-mfloat-abi=hard -mfpu=fpv4-sp-d16)
# add_link_options(-mfloat-abi=hard -mfpu=fpv4-sp-d16)
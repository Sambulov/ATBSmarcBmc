

file(GLOB ST_COMMON
  "./*.c"
  "./Inc/*.h"
)

target_sources(McuHalPort PUBLIC ${ST_COMMON})

if (MCU STREQUAL "STM32L0XX")
  add_subdirectory("./STM32L0XX")
  set(MCPU cortex-m0)
else()
  message(SEND_ERROR "MCU ${MCU} unsupported!")
endif()


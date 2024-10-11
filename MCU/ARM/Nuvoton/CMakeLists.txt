

file(GLOB NU_COMMON
  "./*.c"
  "./Inc/*.h"
)

target_sources(McuHalPort PUBLIC ${NU_COMMON})

if (MCU STREQUAL "NUM46X")
  add_subdirectory("./NUM46X")
  set(MCPU cortex-m4)
else()
  message(SEND_ERROR "MCU ${MCU} unsupported!")
endif()




file(GLOB GD_COMMON
  "./*.c"
  "./Inc/*.h"
)

target_sources(McuHalPort PUBLIC ${GD_COMMON})

if (MCU STREQUAL "GD32E23X")
  add_subdirectory("./GD32E23X")
  set(MCPU cortex-m23)
elseif(MCU STREQUAL "GD32F103VG")
  add_subdirectory("./GD32F103VG")
  set(MCPU cortex-m3)
elseif(MCU STREQUAL "GD32F450ZI")
  add_subdirectory("./GD32F450ZI")
  set(MCPU cortex-m4)
else()
  message(SEND_ERROR "MCU ${MCU} unsupported!")
endif()


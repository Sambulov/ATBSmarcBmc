

file(GLOB ARM_MCU_SOURCES
  "./*.c"
  "./CMSIS/*.c"
  "./CMSIS/*.h"
)

add_library(McuHalPort STATIC ${ARM_MCU_SOURCES})

if (MANUFACTURER STREQUAL "Gigadevice")
  add_subdirectory("./Gigadevice")
elseif(MANUFACTURER STREQUAL "Nuvoton")
  add_subdirectory("./Nuvoton")
elseif(MANUFACTURER STREQUAL "ST")
  add_subdirectory("./ST")
else()
  message(SEND_ERROR "Manufacturer ${MANUFACTURER} unsupported!")
endif()

add_compile_options(-mfloat-abi=soft -mthumb -mlittle-endian -mthumb-interwork -ffunction-sections -fdata-sections -fno-common -fmessage-length=0)
add_compile_options(-mcpu=${MCPU} )
add_link_options(-mcpu=${MCPU} -T ${LINKER_SCRIPT})
add_link_options(-Wl,-gc-sections,--print-memory-usage,-Map=${PROJECT_BINARY_DIR}/${PROJECT_NAME}.map)

set(DEFINES "ARM_MATH_CM3;ARM_MATH_MATRIX_CHECK;ARM_MATH_ROUNDING;")
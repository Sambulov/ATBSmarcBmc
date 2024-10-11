cmake_minimum_required(VERSION 3.6)
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_VERSION 1)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set linker options
add_link_options(-specs=nano.specs -lstdc++ -mthumb -mlittle-endian -mthumb-interwork)

# Set defines
set(DEFINES "ARM_MATH_CM3;ARM_MATH_MATRIX_CHECK;ARM_MATH_ROUNDING;")

# Set toolchain dir, empty - use default
set(TOOLCHAIN "")

# Set toolchain prefix
set(TOOLCHAIN_PREFIX arm-none-eabi-)

set(CMAKE_C_COMPILER ${TOOLCHAIN}${TOOLCHAIN_PREFIX}gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN}${TOOLCHAIN_PREFIX}g++)
set(CMAKE_ASM_COMPILER ${TOOLCHAIN}${TOOLCHAIN_PREFIX}gcc)
set(CMAKE_AR ${TOOLCHAIN}${TOOLCHAIN_PREFIX}ar)
set(CMAKE_OBJCOPY ${TOOLCHAIN}${TOOLCHAIN_PREFIX}objcopy)
set(CMAKE_OBJDUMP ${TOOLCHAIN}${TOOLCHAIN_PREFIX}objdump)
set(CMAKE_SIZE ${TOOLCHAIN}${TOOLCHAIN_PREFIX}size)

# Set project name
project(firmware)
enable_language(C CXX ASM)

# Set C standart
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 11)
#set(CMAKE_C_STANDARD_REQUIRED ON)
#set(CMAKE_C_EXTENSIONS OFF)

# Set default target MCU name
if(NOT DEFINED BOARD) 
  set(BOARD "ATB_RK3568J_SMC_R3")
endif()

# Set terget core name
if(BOARD STREQUAL "ATB_RK3568J_SMC_R3")
  set(VENDOR "Nuvoton")
  set(MCU  "NUM46X")
  set(CORE "ARM")
  set(MCPU cortex-m4)
  set(PROJECT "ATB_RK3568J_SMC")
  # set(SPECIFIC_BUSINESS_LOGIC_INC_PATH ...)
  # set(SPECIFIC_BUSINESS_LOGIC_SRC_PATH ...)
  add_compile_definitions("__MCU_HAL_HDR__=<m460.h>")
elseif(BOARD STREQUAL "ATB_RK3568J_SMC_R1")
  set(VENDOR "Gigadevice")
  set(MCU "GD32E23X")
  set(CORE "ARM")
  set(MCPU cortex-m23)
  set(PROJECT "ATB_RK3568J_SMC")
  # set(SPECIFIC_BUSINESS_LOGIC_INC_PATH ...)
  # set(SPECIFIC_BUSINESS_LOGIC_SRC_PATH ...)
  add_compile_definitions("__MCU_HAL_HDR__=<gd32e23x.h>")
elseif(BOARD STREQUAL "ATB_RK3568J_SMC_R0")
  set(VENDOR "ST")
  set(MCU "STM32L0XX")
  set(CORE "ARM")
  set(MCPU cortex-m0)
  set(PROJECT "ATB_RK3568J_SMC")
  # set(SPECIFIC_BUSINESS_LOGIC_INC_PATH ...)
  # set(SPECIFIC_BUSINESS_LOGIC_SRC_PATH ...)
  add_compile_definitions("__MCU_HAL_HDR__=<stm32l0xx_hal.h>")
  add_compile_definitions("STM32L071KB")
  add_compile_definitions("STM32L071xx")
elseif(BOARD STREQUAL "ATB_USPD_MPC")
  set(VENDOR "Gigadevice")
  set(MCU "GD32F103VG")
  set(CORE "ARM")
  set(MCPU cortex-m3)
  set(PROJECT "ATB_USPD_MPC")
  # set(SPECIFIC_BUSINESS_LOGIC_INC_PATH ...)
  # set(SPECIFIC_BUSINESS_LOGIC_SRC_PATH ...)
  add_compile_definitions("__MCU_HAL_HDR__=<gd32f10x.h>")
  add_compile_definitions("GD32F10X_XD")
elseif(BOARD STREQUAL "ATB_USPD_MPC2")
  set(VENDOR "Gigadevice")
  set(MCU "GD32E23X")
  set(CORE "ARM")
  set(MCPU cortex-m23)
  set(PROJECT "ATB_USPD_MPC2")
  # set(SPECIFIC_BUSINESS_LOGIC_INC_PATH ...)
  # set(SPECIFIC_BUSINESS_LOGIC_SRC_PATH ...)
  add_compile_definitions("__MCU_HAL_HDR__=<gd32e23x.h>")  
elseif(BOARD STREQUAL "ATB_USPD20K")
  set(VENDOR "Gigadevice")
  set(MCU "GD32E23X")
  set(CORE "ARM")
  set(MCPU cortex-m23)
  set(PROJECT "ATB_USPD20K")
  # set(SPECIFIC_BUSINESS_LOGIC_INC_PATH ...)
  # set(SPECIFIC_BUSINESS_LOGIC_SRC_PATH ...)
  add_compile_definitions("__MCU_HAL_HDR__=<gd32e23x.h>")  
elseif(BOARD STREQUAL "ATB_3500")
  set(VENDOR "Gigadevice")
  set(MCU "GD32F450ZI")
  set(CORE "ARM")
  set(MCPU cortex-m4)
  set(PROJECT "ATB3500")
  # set(SPECIFIC_BUSINESS_LOGIC_INC_PATH ...)
  # set(SPECIFIC_BUSINESS_LOGIC_SRC_PATH ...)
  add_compile_definitions("__MCU_HAL_HDR__=<gd32f4xx.h>")
  add_compile_definitions("GD32F450")
elseif(BOARD STREQUAL "ATB_MUVV")
  set(VENDOR "Gigadevice")
  set(MCU "GD32E23X")
  set(CORE "ARM")
  set(MCPU cortex-m23)
  set(PROJECT "ATB_MUVV")
  # set(SPECIFIC_BUSINESS_LOGIC_INC_PATH ...)
  # set(SPECIFIC_BUSINESS_LOGIC_SRC_PATH ...)
  add_compile_definitions("__MCU_HAL_HDR__=<gd32e23x.h>")  
elseif(BOARD STREQUAL "GD32E23X_DEMO")
  set(VENDOR "Gigadevice")
  set(MCU "GD32E23X")
  set(CORE "ARM")
  set(MCPU cortex-m23)
  set(PROJECT "GD32E23X_DEMO")
  # set(SPECIFIC_BUSINESS_LOGIC_INC_PATH ...)
  # set(SPECIFIC_BUSINESS_LOGIC_SRC_PATH ...)
  add_compile_definitions("__MCU_HAL_HDR__=<gd32e23x.h>")  
else()
  message( SEND_ERROR "Board not supported!" )
endif()

add_compile_definitions("${BOARD}")

if (CORE STREQUAL "ARM")
  # Set linker script file
  set(LINKER_SCRIPT "${CMAKE_SOURCE_DIR}/MCU/ARM/${VENDOR}/${MCU}/Res/linkerscript.ld")

  # Set compiler options
  add_compile_options(-mfloat-abi=soft -mthumb -mlittle-endian -mthumb-interwork -ffunction-sections -fdata-sections -fno-common -fmessage-length=0)

  # Uncomment for hardware floating point
  # add_compile_options(-mfloat-abi=hard -mfpu=fpv4-sp-d16)
  # add_link_options(-mfloat-abi=hard -mfpu=fpv4-sp-d16)

  # Set linker options
  add_link_options(-specs=nano.specs -lstdc++ -mthumb -mlittle-endian -mthumb-interwork)

  # Set defines
  set(DEFINES "ARM_MATH_CM3;ARM_MATH_MATRIX_CHECK;ARM_MATH_ROUNDING;")

  # Set toolchain dir, empty - use default
  set(TOOLCHAIN "")

  # Set toolchain prefix
  set(TOOLCHAIN_PREFIX arm-none-eabi-)

  set(CMAKE_C_COMPILER ${TOOLCHAIN}${TOOLCHAIN_PREFIX}gcc)
  set(CMAKE_CXX_COMPILER ${TOOLCHAIN}${TOOLCHAIN_PREFIX}g++)
  set(CMAKE_ASM_COMPILER ${TOOLCHAIN}${TOOLCHAIN_PREFIX}gcc)
  set(CMAKE_AR ${TOOLCHAIN}${TOOLCHAIN_PREFIX}ar)
  set(CMAKE_OBJCOPY ${TOOLCHAIN}${TOOLCHAIN_PREFIX}objcopy)
  set(CMAKE_OBJDUMP ${TOOLCHAIN}${TOOLCHAIN_PREFIX}objdump)
  set(CMAKE_SIZE ${TOOLCHAIN}${TOOLCHAIN_PREFIX}size)

  #set(HEADER_FILE_ONLY ON)

  # Uncomment to mitigate c++17 absolute addresses warnings
  # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-register")
  if("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
      message(STATUS "Maximum optimization for speed")
      add_compile_options(-Ofast)
  elseif("${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo")
      message(STATUS "Maximum optimization for speed, debug info included")
      add_compile_options(-Ofast -g)
  elseif("${CMAKE_BUILD_TYPE}" STREQUAL "MinSizeRel")
      message(STATUS "Maximum optimization for size")
      add_compile_options(-Os)
  else()
      message(STATUS "Minimal optimization, debug info included")
      add_compile_definitions("DEBUG")
      add_compile_options(-O0 -ggdb)
  endif()
  
  # Set default version
  set(GIT_COMMIT_DATE "0")

  if(MINGW OR CYGWIN OR WIN32)
      # Если возникает ошибка PSSecurityException и скрипт не выполняется, то:
      # 1. Start Windows PowerShell with the "Run as Administrator" option. Only members of the Administrators group on the computer can change the execution policy.
      # 2. Enable running unsigned scripts by entering: set-executionpolicy remotesigned
      # This will allow running unsigned scripts that you write on your local computer and signed scripts from Internet.
      execute_process(
          COMMAND ./git-get-date.ps1
          WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
          OUTPUT_VARIABLE GIT_COMMIT_DATE
      )
  elseif(UNIX OR APPLE)
      execute_process(
          COMMAND ./git-get-date.sh
          WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
          OUTPUT_VARIABLE GIT_COMMIT_DATE
      )
  endif()

  # Add C defines
  add_compile_definitions(FW_VERSION=${GIT_COMMIT_DATE})
  add_compile_definitions(${MCU})

  # Add Include directories
  include_directories(
    "${CMAKE_SOURCE_DIR}/CL"
    "${CMAKE_SOURCE_DIR}/CL/Inc"
    "${CMAKE_SOURCE_DIR}/CL/DataStructures/Inc"
    "${CMAKE_SOURCE_DIR}/CL/Workflow/Inc"
    "${CMAKE_SOURCE_DIR}/CL/Math/Inc"
    "${CMAKE_SOURCE_DIR}/HDL"
    "${CMAKE_SOURCE_DIR}/HDL/Basic/Inc"
    "${CMAKE_SOURCE_DIR}/HDL/ATB/Inc"
    "${CMAKE_SOURCE_DIR}/HDL/PMIC/Inc"
    "${CMAKE_SOURCE_DIR}/HDL/Smarc/Inc"
    "${CMAKE_SOURCE_DIR}/HDL/ADC/Inc"
    "${CMAKE_SOURCE_DIR}/HDL/EEPROM/Inc"
    "${CMAKE_SOURCE_DIR}/MCU/ARM/CMSIS"
    "${CMAKE_SOURCE_DIR}/MCU/ARM/CMSIS"
    "${CMAKE_SOURCE_DIR}/MCU/ARM/${VENDOR}/Inc"
    "${CMAKE_SOURCE_DIR}/MCU/ARM/${VENDOR}/${MCU}/Port/Inc"
    "${CMAKE_SOURCE_DIR}/MCU/ARM/${VENDOR}/${MCU}/Sys/Inc"
    "${CMAKE_SOURCE_DIR}/MCU/ARM/${VENDOR}/${MCU}/Sys/Drivers/Inc"
    "${CMAKE_SOURCE_DIR}/Projects/${PROJECT}/Inc"
    "${CMAKE_SOURCE_DIR}/Projects/${PROJECT}/BLDL/Inc"
    ${SPECIFIC_BUSINESS_LOGIC_INC_PATH}
  )

  # Add you source file
  file(GLOB SOURCES
  "${CMAKE_SOURCE_DIR}/CL/Src/*.c"
  "${CMAKE_SOURCE_DIR}/CL/DataStructures/Src/*.c"
  "${CMAKE_SOURCE_DIR}/CL/Workflow/Src/*.c"
  "${CMAKE_SOURCE_DIR}/CL/Math/Src/*.c"
  "${CMAKE_SOURCE_DIR}/HDL/Basic/Src/*.c"
  "${CMAKE_SOURCE_DIR}/HDL/ATB/Src/*.c"
  "${CMAKE_SOURCE_DIR}/HDL/PMIC/Src/*.c"
  "${CMAKE_SOURCE_DIR}/HDL/Smarc/Src/*.c"
  "${CMAKE_SOURCE_DIR}/HDL/ADC/Src/*.c"
  "${CMAKE_SOURCE_DIR}/HDL/EEPROM/Src/*.c"
  "${CMAKE_SOURCE_DIR}/MCU/ARM/*.c"
  "${CMAKE_SOURCE_DIR}/MCU/ARM/${VENDOR}/*.c"
  "${CMAKE_SOURCE_DIR}/MCU/ARM/${VENDOR}/${MCU}/Port/*.c"
  "${CMAKE_SOURCE_DIR}/MCU/ARM/${VENDOR}/${MCU}/Sys/*.c"
  "${CMAKE_SOURCE_DIR}/MCU/ARM/${VENDOR}/${MCU}/Sys/Drivers/Src/*.c"
  "${CMAKE_SOURCE_DIR}/Projects/${PROJECT}/*.c"
  "${CMAKE_SOURCE_DIR}/Projects/${PROJECT}/BLDL/*.c"
   ${SPECIFIC_BUSINESS_LOGIC_SRC_PATH}
  )

  add_compile_options(-mcpu=${MCPU} )
  add_link_options(-mcpu=${MCPU} -T ${LINKER_SCRIPT})
  add_link_options(-Wl,-gc-sections,--print-memory-usage,-Map=${PROJECT_BINARY_DIR}/${PROJECT_NAME}.map)

  #set(STARTUP_FILE "${CMAKE_SOURCE_DIR}/MCU/ARM/${MCU}/Sys/startup.c")

  message("CMAKE_HOST_SYSTEM_NAME = ${CMAKE_HOST_SYSTEM_NAME}")
  message("CMAKE_HOST_SYSTEM_VERSION = ${CMAKE_HOST_SYSTEM_VERSION}")
  message("GIT_COMMIT_DATE = ${GIT_COMMIT_DATE}")
  #message("CMAKE_C_FLAGS = ${CMAKE_C_FLAGS}")
  #message("CMAKE_CXX_FLAGS = ${CMAKE_CXX_FLAGS}")
  message("DEFINES = ${DEFINES}")

  set(EXECUTABLE ${PROJECT_NAME}.elf)

  #add_executable(${EXECUTABLE} ${SOURCES} ${STARTUP_FILE} ${LINKER_SCRIPT})
  add_executable(${EXECUTABLE} ${SOURCES} ${LINKER_SCRIPT})

  set(HEX_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.hex)
  set(BIN_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.bin)

  add_custom_command(TARGET ${EXECUTABLE} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex ${EXECUTABLE} ${HEX_FILE}
    COMMAND ${CMAKE_OBJCOPY} -O binary ${EXECUTABLE} ${BIN_FILE}
    COMMENT "Building ${HEX_FILE} Building ${BIN_FILE}")


  # Print executable size
  add_custom_command(TARGET ${EXECUTABLE} POST_BUILD
    COMMAND ${CMAKE_SIZE} ${EXECUTABLE})
endif() #ARM



# Write to flash
# set(OPEN_OCD C:\\NuOpenOCD\\bin\\openocd.exe)

# add_custom_command(TARGET ${EXECUTABLE} POST_BUILD
#         COMMENT "write to flash ..."
#         COMMAND ${OPEN_OCD} -s "${CMAKE_SOURCE_DIR}/${MCU}/Res" 
#         -f "${CMAKE_SOURCE_DIR}/${MCU}/Res/tool.cfg" 
#         -f "${CMAKE_SOURCE_DIR}/${MCU}/Res/mcu.cfg" 
#         -c "init" -c "halt"
#         -c "flash write_image erase ${CMAKE_SOURCE_DIR}/build/bmc.hex"
#         -c "reset run")

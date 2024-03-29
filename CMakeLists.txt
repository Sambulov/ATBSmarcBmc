cmake_minimum_required(VERSION 3.6)
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_VERSION 1)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set project name
project(bmc)
enable_language(C CXX ASM)

# Set C standart
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 11)
#set(CMAKE_C_STANDARD_REQUIRED ON)
#set(CMAKE_C_EXTENSIONS OFF)

# Set default target MCU name
if(NOT DEFINED MCU) 
set(MCU "NUM463KG")
endif()

# Set terget core name
if(MCU STREQUAL "NUM463KG") 
  set(MCPU cortex-m4)
elseif(MCU STREQUAL "GD32E230")
  set(MCPU cortex-m23)
endif()

# Set linker script file
set(LINKER_SCRIPT "${CMAKE_SOURCE_DIR}/${MCU}/Res/linkerscript.ld")

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
set(DEFINES "${DEFINES}FW_VERSION=${GIT_COMMIT_DATE}")
add_compile_definitions(${DEFINES})

# Add Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/App
    ${CMAKE_SOURCE_DIR}/ARM/CMSIS
    ${CMAKE_SOURCE_DIR}/${MCU}/Sys/inc
    ${CMAKE_SOURCE_DIR}/${MCU}/Sys/Drivers/inc
)

# Add you source file
file(GLOB SOURCES
    "${CMAKE_SOURCE_DIR}/App/*.c"
    "${CMAKE_SOURCE_DIR}/${MCU}/*.c"
    "${CMAKE_SOURCE_DIR}/${MCU}/Sys/*.c"
    "${CMAKE_SOURCE_DIR}/${MCU}/Sys/Drivers/src/*.c"
)

add_compile_options(-mcpu=${MCPU} )
add_link_options(-mcpu=${MCPU} -T ${LINKER_SCRIPT})
add_link_options(-Wl,-gc-sections,--print-memory-usage,-Map=${PROJECT_BINARY_DIR}/${PROJECT_NAME}.map)

set(STARTUP_FILE ${CMAKE_SOURCE_DIR}/${MCU}/Sys/startup.c)

message("CMAKE_HOST_SYSTEM_NAME = ${CMAKE_HOST_SYSTEM_NAME}")
message("CMAKE_HOST_SYSTEM_VERSION = ${CMAKE_HOST_SYSTEM_VERSION}")
message("GIT_COMMIT_DATE = ${GIT_COMMIT_DATE}")
#message("CMAKE_C_FLAGS = ${CMAKE_C_FLAGS}")
#message("CMAKE_CXX_FLAGS = ${CMAKE_CXX_FLAGS}")
message("DEFINES = ${DEFINES}")

set(EXECUTABLE ${PROJECT_NAME}.elf)

add_executable(${EXECUTABLE} ${SOURCES} ${STARTUP_FILE} ${LINKER_SCRIPT})

set(HEX_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.hex)
set(BIN_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.bin)

add_custom_command(TARGET ${EXECUTABLE} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex ${EXECUTABLE} ${HEX_FILE}
    COMMAND ${CMAKE_OBJCOPY} -O binary ${EXECUTABLE} ${BIN_FILE}
    COMMENT "Building ${HEX_FILE} Building ${BIN_FILE}")


# Print executable size
add_custom_command(TARGET ${EXECUTABLE} POST_BUILD
        COMMAND ${CMAKE_SIZE} ${EXECUTABLE})



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

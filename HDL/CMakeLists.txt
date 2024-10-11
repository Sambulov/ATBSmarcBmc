

file(GLOB HDL_SOURCES
  "./Basic/Src/*.c"
  "./ADC/Src/*.c"
  "./EEPROM/Src/*.c"
  "./PMIC/Src/*.c"
  "./Smarc/Src/*.c"
)

add_library(HDL STATIC ${HDL_SOURCES})

target_include_directories(HDL PUBLIC
  "./"
  "./Basic/Inc"
  "./ADC/Inc"
  "./EEPROM/Inc"
  "./PMIC/Inc"
  "./Smarc/Inc"
)

target_link_libraries(HDL PUBLIC CodeLib)

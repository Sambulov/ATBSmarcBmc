

file(GLOB LIB_SOURCES
  "./DataStructures/Src/*.c"
  "./Workflow/Src/*.c"
  "./Math/Src/*.c"
)

add_library(CodeLib STATIC ${LIB_SOURCES})

target_include_directories(CodeLib PUBLIC
  "."
  "./DataStructures/Inc"
  "./Workflow/Inc"
  "./Math/Inc"
)

cmake_minimum_required(VERSION 3.20)

project(bbs LANGUAGES C)

add_executable(bbs src/bbs.c)

set_target_properties(bbs PROPERTIES
  C_STANDARD 11
  C_STANDARD_REQUIRED ON
  C_EXTENSIONS ON
  ENABLE_EXPORTS ON
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

foreach(cfg Debug Release RelWithDebInfo MinSizeRel)
  string(TOUPPER "${cfg}" cfg_upper)
  set_target_properties(bbs PROPERTIES "RUNTIME_OUTPUT_DIRECTORY_${cfg_upper}" "${CMAKE_BINARY_DIR}")
endforeach()

if(MSVC)
  set_property(TARGET bbs PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
  target_compile_definitions(bbs PRIVATE _CRT_SECURE_NO_WARNINGS)
  target_compile_options(bbs PRIVATE /W4)
else()
  target_compile_options(bbs PRIVATE -Wall -Wpedantic -Wno-unused-function)
endif()

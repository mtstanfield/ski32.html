cmake_minimum_required(VERSION 3.22)
project(skifree C)
set(CMAKE_C_STANDARD 99)
option(SKI_DETERMINISTIC "frozen RNG seed (harness builds)" ON)
option(SKI_HARNESS "per-tick framebuffer dump hooks" OFF)
add_compile_definitions(
  WIN32
  SKI_DETERMINISTIC=$<IF:$<BOOL:${SKI_DETERMINISTIC}>,1,0>
  SKI_HARNESS=$<IF:$<BOOL:${SKI_HARNESS}>,1,0>
)
set(SKI_SOURCES src/ski_main.c src/ski_core.c)
if(EMSCRIPTEN)
  file(GLOB SHIM_SOURCES shim/*.c)
  list(APPEND SKI_SOURCES ${SHIM_SOURCES})
endif()
add_executable(ski ${SKI_SOURCES})
if(EMSCRIPTEN)
  target_link_options(ski PRIVATE
    -sENVIRONMENT=web
    -sINITIAL_MEMORY=16MB
    -sALLOW_MEMORY_GROWTH=1
    -sEXPORT_KEEPALIVE=1
    -sMODULARIZE=1 -sEXPORT_NAME=createSki
    -o $<TARGET_FILE_DIR:ski>/ski.js
  )
else()
  target_compile_options(ski PRIVATE -O2 -mfpmath=387 -Wall)
  target_link_options(ski PRIVATE -mwindows)
endif()

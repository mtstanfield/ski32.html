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
# The game executable builds for wasm (EMSCRIPTEN) or Windows (a mingw
# cross compiler, named *-mingw*-*, or an explicit CMAKE_SYSTEM_NAME).
# CMAKE_CROSSCOMPILING is NOT auto-set for the i686 mingw toolchain on
# this host, so the compiler name is the detection signal. A native Linux
# host defines only the shim unit tests below.
get_filename_component(SKI_CC_NAME "${CMAKE_C_COMPILER}" NAME)
string(FIND "${SKI_CC_NAME}" "mingw" SKI_CC_MINGW)
set(SKI_NATIVE_HOST FALSE)
set(SKI_SOURCES src/ski_win.c src/ski_core.c)
if(EMSCRIPTEN)
  file(GLOB SHIM_SOURCES shim/*.c)
  list(APPEND SKI_SOURCES ${SHIM_SOURCES})
elseif(SKI_CC_MINGW GREATER -1 OR CMAKE_SYSTEM_NAME STREQUAL "Windows")
  # Embed bitmaps + string table. CMake >= 3.14 normally auto-compiles a
  # .rc in the sources with CMAKE_RC_COMPILER, but this toolchain's CMake
  # does not detect the RC compiler, so invoke windres explicitly and link
  # the produced object. windres resolves bitmap paths relative to CWD;
  # give it the source root so res/bmp_NNN.bmp resolves.
  set(SKI_RC_OBJ "${CMAKE_CURRENT_BINARY_DIR}/resources.rc.o")
  add_custom_command(
    OUTPUT "${SKI_RC_OBJ}"
    COMMAND "${CMAKE_RC_COMPILER}" --include-dir=${CMAKE_CURRENT_SOURCE_DIR}
            -O coff -o "${SKI_RC_OBJ}"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/resources.rc"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/resources.rc"
    COMMENT "windres: compiling src/resources.rc -> resources.rc.o"
    VERBATIM
  )
  list(APPEND SKI_SOURCES "${SKI_RC_OBJ}")
else()
  set(SKI_SOURCES "")
  set(SKI_NATIVE_HOST TRUE)
endif()
if(SKI_SOURCES)
  add_executable(ski ${SKI_SOURCES})
  if(EMSCRIPTEN)
    # `#include <windows.h>` in the unmodified game sources must resolve to
    # shim/windows.h (API.md "Build notes").
    target_include_directories(ski PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/shim)
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
    target_link_libraries(ski PRIVATE winmm) # PlaySoundA
  endif()
endif()

if(NOT EMSCRIPTEN)
  enable_testing()
  add_executable(test_surface test/test_surface.c shim/surface.c)
  target_include_directories(test_surface PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} shim)
  add_test(NAME surface COMMAND test_surface)
  if(SKI_NATIVE_HOST)
    # Host tests run natively: prove clipping does not overflow (ASan) and
    # keep the build warning-clean.
    target_compile_options(test_surface PRIVATE -Wall -fsanitize=address,undefined)
    target_link_options(test_surface PRIVATE -fsanitize=address,undefined)
  endif()
endif()

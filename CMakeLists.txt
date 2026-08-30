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
set(SKI_SOURCES src/ski_win.c src/ski_core.c)
if(EMSCRIPTEN)
  file(GLOB SHIM_SOURCES shim/*.c)
  list(APPEND SKI_SOURCES ${SHIM_SOURCES})
else()
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
  target_link_libraries(ski PRIVATE winmm) # PlaySoundA
endif()

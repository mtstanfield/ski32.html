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
  # ${CMAKE_RC_COMPILER} has no default: a FRESH configure (no cache
  # variable) would fail at resources.rc (T20 review). Default it from
  # the C compiler's triple (i686-w64-mingw32-gcc ->
  # i686-w64-mingw32-windres), falling back to a bare windres.
  if(NOT CMAKE_RC_COMPILER)
    get_filename_component(SKI_CC_BASE "${CMAKE_C_COMPILER}" NAME)
    string(REGEX REPLACE "(.*)gcc$" "\\1windres" SKI_RC_GUESS
           "${SKI_CC_BASE}")
    find_program(CMAKE_RC_COMPILER NAMES "${SKI_RC_GUESS}" windres)
    if(NOT CMAKE_RC_COMPILER)
      message(FATAL_ERROR
        "no RC compiler found (tried ${SKI_RC_GUESS}, windres); "
        "set CMAKE_RC_COMPILER")
    endif()
  endif()
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
    # HEAPU8 (canvas.c's blit/png EM_JS reads Module.HEAPU8),
    # UTF8ToString + _malloc (the C-buffer debug hooks — ski_window_png
    # writes its dataURL into a caller-_malloc'd buffer; the T21 harness
    # reads it back with UTF8ToString).
    target_link_options(ski PRIVATE
      -sENVIRONMENT=web
      -sINITIAL_MEMORY=16MB
      -sALLOW_MEMORY_GROWTH=1
      -sEXPORT_KEEPALIVE=1
      -sEXPORTED_RUNTIME_METHODS=HEAPU8,UTF8ToString,stringToUTF8
      # _main stays listed: an explicit EXPORTED_FUNCTIONS without it
      # makes emcc warn (-Wunused-main); INVOKE_RUN (default) still runs
      # main at instantiation either way. _free: the T21 harness
      # allocates the ski_window_png buffer per tick.
      -sEXPORTED_FUNCTIONS=_malloc,_free,_main
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
  if(SKI_CC_MINGW GREATER -1)
    # The i686 mingw driver appends .exe to an extensionless -o name, but
    # CMake (which sees no cross-compile on this host) does not detect
    # that: pin the suffix so the recorded target file matches what the
    # linker actually writes.
    set_target_properties(test_surface PROPERTIES SUFFIX ".exe")
  endif()
  # $<TARGET_FILE:...> instead of a bare target name: on a non-Windows
  # host a bare name resolves without the .exe suffix and ctest cannot
  # find the cross-built binary.
  add_test(NAME surface COMMAND $<TARGET_FILE:test_surface>)
  if(SKI_NATIVE_HOST)
    # Host tests run natively: prove clipping does not overflow (ASan) and
    # keep the build warning-clean.
    target_compile_options(test_surface PRIVATE -Wall -fsanitize=address,undefined)
    target_link_options(test_surface PRIVATE -fsanitize=address,undefined)
  endif()
endif()

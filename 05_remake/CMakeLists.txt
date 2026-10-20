# Recoil (1999) remake - Stage 2 build (STAGE2.md, TODO_STAGE2.md P0.5).
# Decision D1: 32-bit x86, x87 maths (/arch:IA32), C++20, MSVC. Configure through build.bat, which
# sets up the MSVC x86 environment and uses Ninja.
cmake_minimum_required(VERSION 3.24)
project(RecoilRemake LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_SIZEOF_VOID_P EQUAL 4)
  message(FATAL_ERROR "The remake is built 32-bit (decision D1): run build.bat (MSVC x64_x86 environment).")
endif()

# RECOIL_PLAYER_RELEASE: the exe of the players' download - no build-machine paths compiled in, no resources copied (the player
# Setup, tools/setup/recoil_setup.py, copies them from the player's own Recoil.exe). Use with the runtime data image
# (gen_data_image.py --runtime), so the exe holds no bytes of Recoil.exe.
option(RECOIL_PLAYER_RELEASE "build the exe for the players' download (Setup adds the original's resources)" OFF)
# The players' exe carries its C runtime (static CRT), so players need no Visual C++ redistributable.
if(RECOIL_PLAYER_RELEASE)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
endif()

# x87 code generation, strict floating point (no contraction/reassociation), UTF-8 sources.
add_compile_options(/arch:IA32 /fp:strict /utf-8 /W4 /permissive- /Zc:__cplusplus)
# The original calls the plain CRT functions (_controlfp, sprintf...); keep the same calls.
add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
# Coverage measurement build only (build.bat Coverage): entry counters in instruction-level ports.
option(RECOIL_ENTRY_COUNTS "Count entries of ported functions (coverage measurement build)" OFF)
if(RECOIL_ENTRY_COUNTS)
  add_compile_definitions(RECOIL_ENTRY_COUNTS)
endif()

# The player's own copy of the game (nothing from Recoil is shipped with this source):
#   RECOIL_ORIGINAL_EXE      their Recoil.exe (1999-01-29 build) - resources copied into the remake, oracle tests
#   RECOIL_DEFAULT_DDRAW_DIR a folder with DDraw.dll (dgVoodoo) the launcher uses when RECOIL_DDRAW_DIR is not set
# tools/build_remake.py fills these in; the data image (src/platform/image/original_data.cpp) is generated from the same
# exe by tools/asm_port/gen_data_image.py --source <your copy>.
set(RECOIL_ORIGINAL_EXE "${CMAKE_SOURCE_DIR}/../00_original/game_install/Recoil.exe" CACHE FILEPATH "the player's Recoil.exe (1999-01-29 build)")
set(RECOIL_DEFAULT_DDRAW_DIR "" CACHE PATH "folder holding DDraw.dll (dgVoodoo) used when RECOIL_DDRAW_DIR is not set")
if(NOT EXISTS ${CMAKE_SOURCE_DIR}/src/platform/image/original_data.cpp)
  message(FATAL_ERROR "src/platform/image/original_data.cpp is missing. It is generated from your own copy of Recoil:\n"
                      "  python tools/asm_port/gen_data_image.py --source <Recoil CD / image / installed folder>\n"
                      "(or run tools/build_remake.py, which does everything).")
endif()

# Layer A/B/C translated code. The source tree mirrors the original's files (ledger column orig_file);
# functions with no attributed file live in src/unattributed/<subsystem>.cpp.
file(GLOB_RECURSE RECOIL_CORE_SOURCES CONFIGURE_DEPENDS
  ${CMAKE_SOURCE_DIR}/src/Battlesport/*.cpp
  ${CMAKE_SOURCE_DIR}/src/GameZRecoil/*.cpp
  ${CMAKE_SOURCE_DIR}/src/unattributed/*.cpp)
add_library(recoil_core STATIC ${RECOIL_CORE_SOURCES})
target_include_directories(recoil_core PUBLIC ${CMAKE_SOURCE_DIR}/src)

# Layer D: platform replacements (window, GPU, audio, input devices, files). Headless recording stubs
# are the default so the simulation can run and be compared without a window.
file(GLOB_RECURSE RECOIL_PLATFORM_SOURCES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/src/platform/*.cpp)
add_library(recoil_platform STATIC ${RECOIL_PLATFORM_SOURCES})
target_include_directories(recoil_platform PUBLIC ${CMAKE_SOURCE_DIR}/src)
# Translated code calls the platform layer (e.g. the MSVCRT import slots used by instruction-level ports).
target_link_libraries(recoil_core PUBLIC recoil_platform)
# DirectInput data formats (c_dfDIKeyboard / c_dfDIMouse / c_dfDIJoystick2, platform/dinput_formats.h) - PLATFORM
target_link_libraries(recoil_platform PUBLIC dinput8 dxguid bcrypt)   # bcrypt: SHA-256 check of the player's Recoil.exe (runtime image) - PLATFORM
# The data image (src/platform/image, decision D7) relocates pointers to ported functions: platform -> core too.
target_link_libraries(recoil_platform PUBLIC recoil_core)

# Tests (numbers here are scaffolding; re_status skips tests/ for the untagged-constant check).
file(GLOB RECOIL_TEST_SOURCES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/tests/*.cpp)
add_executable(recoil_tests ${RECOIL_TEST_SOURCES})
target_link_libraries(recoil_tests PRIVATE recoil_core recoil_platform)
target_compile_definitions(recoil_tests PRIVATE RECOIL_VECTORS_DIR="${CMAKE_SOURCE_DIR}/tests/vectors"
  RECOIL_ORIGINAL_EXE="${RECOIL_ORIGINAL_EXE}")
# Native L1 oracle (tests/native_oracle.h) maps the original image at 0x00400000: keep that range free.
target_link_options(recoil_tests PRIVATE /BASE:0x60000000 /DYNAMICBASE:NO /MAP /SAFESEH:NO)  # /MAP: names crash addresses in tests; /SAFESEH:NO: ported SEH frames push ported C++ EH handlers (the original exe had no SafeSEH table) - PLATFORM

enable_testing()
add_test(NAME recoil_tests COMMAND recoil_tests)

# Boot probe (Stage 2 integration check, 05_remake/boot): starts the remake from the ported CRT entry point in the game
# directory; the first thing it cannot do is the next blocker. Not a mirrored original file.
add_executable(recoil_boot boot/boot_main.cpp boot/virtual_cd.cpp redux/redux.cpp redux/redux_video.cpp)
target_include_directories(recoil_boot PRIVATE ${CMAKE_SOURCE_DIR})   # redux/redux.h
target_link_libraries(recoil_boot PRIVATE recoil_core recoil_platform winmm windowscodecs ole32 shell32 mfplat mfreadwrite mfuuid vfw32 propsys)   # windowscodecs: Redux RX-10 PNG texture packs (WIC)
# the probe runs in a writable COPY of the game folder (05_remake/build/game_sandbox): the ported code writes files there - it once
# overwrote zbd\m1\gamez.zbd in 00_original (2026-09-30, restored from the user's runtime copy). boot_main refuses 00_original.
if(RECOIL_PLAYER_RELEASE)
  target_compile_definitions(recoil_boot PRIVATE RECOIL_GAME_DIR="." RECOIL_DEFAULT_DDRAW_DIR="")
  # a Windows program, no console: with a console, FreeConsole left msvcrt's stdout/stderr on stale handle values; the original's
  # freopen("recoil.out"/"recoil.err") then closed a handle Windows had reused for inputhost's ALPC port -> CoreMessaging
  # fail-fast 0xe0464645 a few seconds after start (user's first Setup install, 2026-10-07)
  target_link_options(recoil_boot PRIVATE /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup /MANIFEST:NO)
  # version information + manifest: an exe without them was taken for malware by Defender's ML check (2026-10-20)
  target_sources(recoil_boot PRIVATE boot/player_version.rc)
else()
  target_compile_definitions(recoil_boot PRIVATE RECOIL_GAME_DIR="${CMAKE_SOURCE_DIR}/build/game_sandbox"
    RECOIL_DEFAULT_DDRAW_DIR="${RECOIL_DEFAULT_DDRAW_DIR}")
endif()
target_link_options(recoil_boot PRIVATE /DYNAMICBASE:NO /MAP /SAFESEH:NO)  # /SAFESEH:NO: ported SEH frames push ported C++ EH handlers - PLATFORM
# The original's resources (menu, dialogs, strings, icons, bitmap) copied into the probe after every link: the ported code loads them
# from its own module (tools/copy_resources.py, CONFIRMED-DATA bytes of Recoil.exe's .rsrc)
find_package(Python3 COMPONENTS Interpreter)
if(Python3_FOUND AND NOT RECOIL_PLAYER_RELEASE)
  add_custom_command(TARGET recoil_boot POST_BUILD
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/../tools/copy_resources.py $<TARGET_FILE:recoil_boot> ${RECOIL_ORIGINAL_EXE}
    VERBATIM)
endif()

# The player Setup (Setup.exe in the players' download, setup/setup_main.cpp): installs the remake from the player's own copy
# of Recoil. Contains no Recoil bytes; static C runtime so it runs on a bare Windows. PLATFORM, not a mirrored original file.
add_executable(recoil_setup WIN32 setup/setup_main.cpp setup/source.cpp setup/inflate.cpp setup/upscale.cpp)
set_target_properties(recoil_setup PROPERTIES OUTPUT_NAME Setup MSVC_RUNTIME_LIBRARY MultiThreaded)
target_compile_definitions(recoil_setup PRIVATE UNICODE _UNICODE)
target_link_libraries(recoil_setup PRIVATE comctl32 shell32 ole32 comdlg32 bcrypt uuid)

@echo off
rem Recoil remake build: MSVC 2022 x86 (decision D1), CMake + Ninja, then the tests.
rem usage: build.bat [Debug|Release|Coverage] [TEST...]    (default Release; TEST: name or prefix*, default all)
rem   The tests run as parallel processes (tools/run_tests.py: one per test, fuzz tests sharded, hang timeout).
rem   Coverage: a Release build with entry counters in the instruction-level ports; the tests then report
rem   which ported functions each composite test reached.
setlocal
set CFG=%1
if "%CFG%"=="" set CFG=Release
set TYPE=%CFG%
set EXTRA=
if /i "%CFG%"=="Coverage" (set TYPE=Release& set EXTRA=-DRECOIL_ENTRY_COUNTS=ON)
set "PATH=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer;%PATH%"
for /f "usebackq tokens=*" %%i in (`vswhere -latest -products * -property installationPath`) do set VSROOT=%%i
if not defined VSROOT (echo MSVC not found & exit /b 1)
call "%VSROOT%\VC\Auxiliary\Build\vcvarsall.bat" x64_x86 >nul || exit /b 1
cd /d "%~dp0"
rem the data image must match the ledger (a newly ported function must replace the unported-code trap)
python ..\tools\asm_port\gen_data_image.py --check || exit /b 1
cmake -S . -B build\%CFG% -G Ninja -DCMAKE_BUILD_TYPE=%TYPE% %EXTRA% >nul || exit /b 1
cmake --build build\%CFG% || exit /b 1
python ..\tools\run_tests.py build\%CFG%\recoil_tests.exe %2 %3 %4 %5 %6 %7 %8 %9 || exit /b 1

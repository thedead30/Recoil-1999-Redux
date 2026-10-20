@echo off
rem Incremental Release build of the remake, no tests (used by tools/llm_pipeline.py for mutant runs).
set "PATH=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer;%PATH%"
for /f "usebackq tokens=*" %%i in (`vswhere -latest -products * -property installationPath`) do set VSROOT=%%i
call "%VSROOT%\VC\Auxiliary\Build\vcvarsall.bat" x64_x86 >nul || exit /b 1
cd /d "%~dp0..\05_remake"
rem RECOIL_BUILD_DIR: another configured build directory (e.g. Port, while a test run holds build\Release)
if not defined RECOIL_BUILD_DIR set RECOIL_BUILD_DIR=Release
if not exist build\%RECOIL_BUILD_DIR%\build.ninja cmake -S . -B build\%RECOIL_BUILD_DIR% -G Ninja -DCMAKE_BUILD_TYPE=Release >nul
cmake --build build\%RECOIL_BUILD_DIR%

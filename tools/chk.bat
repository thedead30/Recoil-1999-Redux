@echo off
rem Check build of the remake (own experiments while build\Release belongs to llm_pipeline or port_loop).
set "PATH=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer;%PATH%"
for /f "usebackq tokens=*" %%i in (`vswhere -latest -products * -property installationPath`) do set VSROOT=%%i
call "%VSROOT%\VC\Auxiliary\Build\vcvarsall.bat" x64_x86 >nul || exit /b 1
cd /d "%~dp0..\05_remake"
cmake -S . -B build\Check >nul || exit /b 1
cmake --build build\Check

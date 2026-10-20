@echo off
rem build_player.bat - Release build of the players' download (build\Player): Setup.exe + recoil_boot.exe
rem (RECOIL_PLAYER_RELEASE: static CRT, Windows subsystem, no build paths, no copied resources). Used by tools/make_release.py.
set "PATH=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer;%PATH%"
for /f "usebackq tokens=*" %%i in (`vswhere -latest -products * -property installationPath`) do set VSROOT=%%i
call "%VSROOT%\VC\Auxiliary\Build\vcvarsall.bat" x64_x86 >nul || exit /b 1
cd /d "%~dp0..\05_remake"
cmake -S . -B build\Player -G Ninja -DCMAKE_BUILD_TYPE=Release -DRECOIL_PLAYER_RELEASE=ON >nul || exit /b 1
cmake --build build\Player --target recoil_boot recoil_setup

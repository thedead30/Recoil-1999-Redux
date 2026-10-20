@echo off
rem play_remake.bat - start the remake for testing: mission 1 direct, no CD, FULL SCREEN by default (windowed is opt-in:
rem set RECOIL_WINDOWED=1 first). Runs a COPY of build\Port\recoil_boot.exe so a running game never blocks a rebuild.
rem Game files: build\game_sandbox. Switches (launcher only, KG-44..48): RECOIL_START_MISSION=0 for the normal start-up.
setlocal
set "SRC=%~dp0..\05_remake\build\Port\recoil_boot.exe"
set "RUN=%TEMP%\recoil_play"
if not exist "%RUN%" mkdir "%RUN%"
copy /y "%SRC%" "%RUN%\recoil_boot.exe" >nul || (echo cannot copy %SRC% & pause & exit /b 1)
if not defined RECOIL_START_MISSION set RECOIL_START_MISSION=1
set RECOIL_NOCD=1
rem Windowed only on request: dgVoodoo copy with a VALID OutputAPI (the runtime copy's OutputAPI = direct3d11 is invalid,
rem so dgVoodoo ignores FullScreenMode = false).
if "%RECOIL_WINDOWED%"=="1" if not defined RECOIL_DDRAW_DIR if exist "%USERPROFILE%\Desktop\Recoil dgVoodoo Windowed\DDraw.dll" set "RECOIL_DDRAW_DIR=%USERPROFILE%\Desktop\Recoil dgVoodoo Windowed"
start "" "%RUN%\recoil_boot.exe"

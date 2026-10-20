@echo off
rem Builds tools\sidebyside\dinput.dll (32-bit) - the original's side-by-side wrapper.
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars32.bat" >nul
cd /d "%~dp0"
cl /nologo /O2 /EHa /LD /MT rcl_dinput.cpp /link /DEF:rcl_dinput.def /OUT:dinput.dll user32.lib

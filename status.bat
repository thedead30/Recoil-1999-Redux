@echo off
REM Project status. Run from the project folder: status
pushd "%~dp0"
python "03_re/scripts/re_status.py"
popd

@echo off
rem play_near_turret.bat - like play_remake.bat, but teleports the tank 40 units in front of the turret nearest the mission-1 start
rem (launcher test shortcut, KG-58). Change the distance with RECOIL_NEAR_TURRET.
if not defined RECOIL_NEAR_TURRET set RECOIL_NEAR_TURRET=40
call "%~dp0play_remake.bat"

@echo off
REM Launch Ghidra with inline scripting enabled for this process only.
REM
REM The GhidraMCP plugin reads GHIDRA_MCP_ALLOW_SCRIPTS via System.getenv() at
REM startup (SecurityConfig.java:89), so it must be set for the GHIDRA process -
REM setting it on the MCP bridge has no effect.
REM
REM This makes NO permanent change to your system environment. Close Ghidra and
REM start it normally to revert.

set GHIDRA_MCP_ALLOW_SCRIPTS=1
echo GHIDRA_MCP_ALLOW_SCRIPTS=%GHIDRA_MCP_ALLOW_SCRIPTS%
echo Starting Ghidra...
start "" "C:\ghidra_12.1.2_PUBLIC\ghidraRun.bat"

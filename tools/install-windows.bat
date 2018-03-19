@ECHO OFF

REM Load configuration
CALL config-windows.bat

REM Copy tests
XCOPY "../test/gm_socketio-test.lua" "%DIRECTORY_GMOD%/garrysmod/lua/autorun/gm_socketio-test.lua"

REM Copy library
XCOPY "../bin/gm_socket.io_win32.dll" "%DIRECTORY_GMOD%/garrysmod/lua/bin/gmcl_socket.io_win32.dll"
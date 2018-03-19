@echo off

rem Load configuration
call "config-windows.bat"

set DIRECTORY_PROJECT=%cd%/../project
set DIRECTORY_ORIGINAL=%cd%

rem Load Visual Studio dev environment
if not defined DevEnvDir (
  call "%DIRECTORY_MSVC%/VsDevCmd.bat"
)

msbuild %DIRECTORY_PROJECT%/gm_socket.io.sln 

cd /D %DIRECTORY_ORIGINAL%
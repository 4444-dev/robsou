@echo off
setlocal
cd /d "%~dp0"

if not exist "builds\ioctl-injector.exe" (
  echo Missing builds\ioctl-injector.exe. Run _orelia_build.bat first.
  exit /b 1
)

if not defined ORELIA_TARGET set "ORELIA_TARGET=ROB RUN.exe"
echo Launching Nexus with target %ORELIA_TARGET%...
start "" /D "%~dp0" "%~dp0builds\ioctl-injector.exe"
exit /b 0

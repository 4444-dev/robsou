@echo off
setlocal EnableExtensions
cd /d "%~dp0"

set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
set "MSB="
if exist "%VSWHERE%" for /f "usebackq delims=" %%I in (`"%VSWHERE%" -latest -prerelease -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe`) do set "MSB=%%I"
if not defined MSB if exist "%ProgramFiles%\Microsoft Visual Studio\2026\Community\MSBuild\Current\Bin\amd64\MSBuild.exe" set "MSB=%ProgramFiles%\Microsoft Visual Studio\2026\Community\MSBuild\Current\Bin\amd64\MSBuild.exe"
if not defined MSB (
  echo Visual Studio MSBuild not found. Install Visual Studio with C++ and MSBuild.
  exit /b 1
)

set "PLAT=x64"
set "CONF=Release"

call :build "orelia-internal\orelia-internal.vcxproj" || exit /b 1
call :build "orelia-ioctl\orelia-ioctl.vcxproj" || exit /b 1
call :build "orelia-mapper\orelia-mapper.vcxproj" || exit /b 1

powershell -NoProfile -Command "Add-Type -TypeDefinition 'using System;using System.IO;public class ResEnc{public static void Run(string src,string dst){byte[] b=File.ReadAllBytes(src);uint seed=(uint)new Random().Next(1,0x7FFFFFFE);uint st=seed^0xA5F19C3Du;byte[] o=new byte[b.Length+8];Buffer.BlockCopy(BitConverter.GetBytes(0x4E585231u),0,o,0,4);Buffer.BlockCopy(BitConverter.GetBytes(seed),0,o,4,4);unchecked{for(int i=0;i<b.Length;i++){st=st*1664525u+1013904223u;byte k=(byte)((st>>16)^((uint)(i*0x45)&0xFFu));o[i+8]=(byte)(b[i]^k);}}File.WriteAllBytes(dst,o);}}'; [ResEnc]::Run('builds\orelia-ioctl.sys','builds\enc_drv.bin'); [ResEnc]::Run('builds\orelia-mapper.exe','builds\enc_map.bin'); [ResEnc]::Run('builds\orelia-internal.dll','builds\enc_dll.bin')" || exit /b 1

call :build "orelia-injector\orelia-injector.vcxproj" || exit /b 1

del /f /q "builds\enc_drv.bin" "builds\enc_map.bin" "builds\enc_dll.bin" 2>nul
del /f /q "builds\*.pdb" "builds\*.exp" "builds\*.lib" 2>nul

echo Build complete: builds\ioctl-injector.exe
exit /b 0

:build
"%MSB%" %1 /t:Build /p:Configuration=%CONF% /p:Platform=%PLAT% /p:ApiValidator_Enable=false /m:8 /v:minimal /nr:false
if errorlevel 1 (
  echo Failed: %1
  exit /b 1
)
exit /b 0

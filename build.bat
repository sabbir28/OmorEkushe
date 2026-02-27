@echo off
setlocal ENABLEEXTENSIONS ENABLEDELAYEDEXPANSION

rem Build script for OmorEkushe native C++ application.
rem Usage examples:
rem   build.bat
rem   build.bat Release Win32
rem   build.bat Debug x64 "Visual Studio 17 2022"

set "CONFIG=%~1"
if "%CONFIG%"=="" set "CONFIG=Release"

set "PLATFORM=%~2"
if "%PLATFORM%"=="" set "PLATFORM=Win32"

set "GENERATOR=%~3"
if "%GENERATOR%"=="" set "GENERATOR=Visual Studio 17 2022"

set "SCRIPT_DIR=%~dp0"
set "BUILD_DIR=%SCRIPT_DIR%build\%GENERATOR: =_%-%PLATFORM%-%CONFIG%"

where cmake >nul 2>nul
if errorlevel 1 (
  echo [ERROR] CMake was not found in PATH.
  echo Install CMake and retry.
  exit /b 1
)

echo [INFO] Generator : %GENERATOR%
echo [INFO] Platform  : %PLATFORM%
echo [INFO] Config    : %CONFIG%
echo [INFO] Build dir : %BUILD_DIR%

if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

cmake -S "%SCRIPT_DIR%" -B "%BUILD_DIR%" -G "%GENERATOR%" -A "%PLATFORM%"
if errorlevel 1 (
  echo [ERROR] CMake configure failed.
  exit /b 1
)

cmake --build "%BUILD_DIR%" --config "%CONFIG%"
if errorlevel 1 (
  echo [ERROR] Build failed.
  exit /b 1
)

echo [SUCCESS] Build completed.
echo [SUCCESS] Output: "%BUILD_DIR%\%CONFIG%\OmorEkushe.exe"
exit /b 0

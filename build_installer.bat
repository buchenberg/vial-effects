@echo off
REM =============================================================================
REM Vial Effects - Full build + installer script
REM
REM Run this from an "x64 Native Tools Command Prompt for VS 2022".
REM (Start Menu → Visual Studio 2022 → x64 Native Tools Command Prompt)
REM
REM Prerequisites:
REM   1. Node 18+ on PATH
REM   2. WebView2 NuGet package extracted to %WEBVIEW2_DIR%
REM   3. Inno Setup 6 installed (https://jrsoftware.org/isinfo.php)
REM   4. ../vial/JUCE checkout present
REM =============================================================================
setlocal

REM Set WEBVIEW2_DIR via environment variable or use default (third_party/webview2)
if "%WEBVIEW2_DIR%"=="" set "WEBVIEW2_DIR=%~dp0third_party\webview2"
set "BUILD_DIR=build"
set "BUILD_TYPE=Release"

echo ===== Building web UI =====
cd /d "%~dp0ui"
call npm install
call npm run build
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: UI build failed
    exit /b 1
)

echo ===== Configuring CMake =====
cd /d "%~dp0"
cmake -B %BUILD_DIR% -G Ninja -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
      -DJUCE_WEBVIEW2_PACKAGE_LOCATION=%WEBVIEW2_DIR%
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: CMake configure failed
    exit /b 1
)

echo ===== Building plugin =====
cmake --build %BUILD_DIR%
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Build failed
    exit /b 1
)

echo ===== Building installer =====
iscc "/DConfiguration=%BUILD_TYPE%" installer\VialEffects.iss
if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ======================================================
    echo Installer build failed.
    echo If iscc was not found, install Inno Setup 6 from:
    echo   https://jrsoftware.org/isinfo.php
    echo Then re-run this script.
    echo ======================================================
    exit /b 1
)

echo.
echo ===== Done! =====
echo Installer: %BUILD_DIR%\installer\VialEffects-*-win64.exe
echo VST3:       %BUILD_DIR%\VialEffects_artefacts\%BUILD_TYPE%\VST3\Vial Effects.vst3\
echo Standalone: %BUILD_DIR%\VialEffects_artefacts\%BUILD_TYPE%\Standalone\Vial Effects.exe
endlocal

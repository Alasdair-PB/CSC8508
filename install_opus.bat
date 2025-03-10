@echo off
setlocal enabledelayedexpansion

:: Set the paths
set OPUS_DIR=%CD%\ThirdParty\opus
set OPUS_BUILD_DIR=%OPUS_DIR%\build
set OPUS_INSTALL_DIR=%CD%\ThirdParty\opus_install

echo ==============================================
echo Installing Opus in %OPUS_INSTALL_DIR%
echo ==============================================

:: Step 1: Ensure Git and CMake are installed
where git >nul 2>nul
if %ERRORLEVEL% neq 0 (
    echo Error: Git is not installed or not in PATH. Please install Git.
    exit /b 1
)

where cmake >nul 2>nul
if %ERRORLEVEL% neq 0 (
    echo Error: CMake is not installed or not in PATH. Please install CMake.
    exit /b 1
)

:: Step 2: Delete existing Opus files if they exist
if exist "%OPUS_DIR%" (
    echo Deleting existing Opus directory...
    rmdir /S /Q "%OPUS_DIR%"
)
if exist "%OPUS_INSTALL_DIR%" (
    echo Deleting existing Opus installation...
    rmdir /S /Q "%OPUS_INSTALL_DIR%"
)

:: Step 3: Clone Opus Repository
echo Cloning Opus repository...
git clone https://gitlab.xiph.org/xiph/opus.git "%OPUS_DIR%"
if %ERRORLEVEL% neq 0 (
    echo Error: Failed to clone Opus repository.
    exit /b 1
)

:: Step 4: Create Build Directory
echo Creating build directory...
mkdir "%OPUS_BUILD_DIR%"
cd "%OPUS_BUILD_DIR%"

:: Step 5: Configure Opus with CMake
echo Configuring Opus...
cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_INSTALL_PREFIX="%OPUS_INSTALL_DIR%" -DBUILD_SHARED_LIBS=ON -DOPUS_BUILD_PROGRAMS=OFF -DOPUS_BUILD_TESTS=OFF
if %ERRORLEVEL% neq 0 (
    echo Error: CMake configuration failed.
    exit /b 1
)

:: Step 6: Build Opus
echo Building Opus...
cmake --build . --config Release
if %ERRORLEVEL% neq 0 (
    echo Error: Opus build failed.
    exit /b 1
)

:: Step 7: Install Opus
echo Installing Opus...
cmake --install . --prefix "%OPUS_INSTALL_DIR%"
if %ERRORLEVEL% neq 0 (
    echo Error: Opus installation failed.
    exit /b 1
)

:: Step 8: Verify Installation
echo Verifying installation...
if exist "%OPUS_INSTALL_DIR%\lib" (
    echo Opus installation completed successfully! 🎉
) else (
    echo Error: Opus library folder not found.
    exit /b 1
)

endlocal
echo Done!
exit /b 0

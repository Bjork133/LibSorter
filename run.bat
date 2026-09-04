windows_script = '''@echo off
setlocal enabledelayedexpansion

:: Check if CMake is installed
where cmake >nul 2>&1
if %errorlevel% neq 0 (
    echo CMake is not installed.
    echo Would you like to install CMake? (Y/N)
    set /p INSTALL_CMAKE=
    if /i "!INSTALL_CMAKE!"=="Y" (
        echo Installing CMake...
        echo Please download and install CMake from: https://cmake.org/download/
        echo After installation, please restart this script.
        pause
        exit /b 1
    ) else (
        echo CMake is required to build the project. Exiting.
        pause
        exit /b 1
    )
)

:: Check if Ninja is installed
where ninja >nul 2>&1
if %errorlevel% neq 0 (
    echo Ninja is not installed.
    echo Would you like to install Ninja? (Y/N)
    set /p INSTALL_NINJA=
    if /i "!INSTALL_NINJA!"=="Y" (
        echo Installing Ninja...
        echo Please download and install Ninja from: https://ninja-build.org/
        echo After installation, please restart this script.
        pause
        exit /b 1
    ) else (
        echo Ninja is required to build the project. Exiting.
        pause
        exit /b 1
    )
)

:menu
cls
echo ================================
echo   LibSorter Launcher
echo ================================
echo.
echo Select an option:
echo 1. Start Server
echo 2. Start Client
echo 3. Rebuild Project (CMake)
echo 4. Exit
echo.
set /p CHOICE="Enter your choice (1-4): "

if "%CHOICE%"=="1" goto start_server
if "%CHOICE%"=="2" goto start_client
if "%CHOICE%"=="3" goto rebuild
if "%CHOICE%"=="4" goto exit_program
echo Invalid choice. Please try again.
pause
goto menu

:start_server
echo Starting server...
cd /d "%USERPROFILE%\Projects\LibSorter"
if exist "build\libsorter_server.exe" (
    start "" "build\libsorter_server.exe" ".\data" "9443"
    echo Server started in a new window.
) else (
    echo Error: Server executable not found. Please rebuild the project first.
)
pause
goto menu

:start_client
echo Starting client...
cd /d "%USERPROFILE%\Projects\LibSorter"
if exist "build\libsorter_client.exe" (
    start "" "build\libsorter_client.exe"
    echo Client started in a new window.
) else (
    echo Error: Client executable not found. Please rebuild the project first.
)
pause
goto menu

:rebuild
echo Rebuilding project...
cd /d "%USERPROFILE%\Projects\LibSorter"
if exist "build" (
    ninja -C build
    if %errorlevel% equ 0 (
        echo Build completed successfully.
    ) else (
        echo Build failed with error code %errorlevel%.
    )
) else (
    echo Build directory not found. Running initial CMake configuration...
    mkdir build
    cd build
    cmake .. -G Ninja
    cd ..
    if %errorlevel% equ 0 (
        echo Initial configuration complete. Building...
        ninja -C build
    ) else (
        echo CMake configuration failed.
    )
)
pause
goto menu

:exit_program
echo Goodbye!
exit /b 0
'''

with open('launch_windows.bat', 'w', encoding='utf-8') as f:
    f.write(windows_script)

print("Windows script created: launch_windows.bat")

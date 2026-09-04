@echo off
chcp 65001 >nul
title LibSorter Launcher
color 0A

echo ====================================================
echo            LibSorter Launcher (C++ / Qt6)
echo ====================================================
echo.

:: Проверяем наличие папки build
if not exist "build" (
    echo [!] Папка 'build' не найдена.
    echo     Хотите собрать проект сейчас? (Y/N)
    set /p "build_now="
    if /i "%build_now%"=="Y" (
        call :build_project
    ) else (
        echo [!] Сборка отменена. Запуск невозможен.
        pause
        exit /b 1
    )
)

:menu
echo Выберите действие:
echo   [1] Запустить сервер (libsorter_server)
echo   [2] Запустить клиент (libsorter_client)
echo   [3] Пересобрать проект (CMake)
echo   [4] Запустить тесты
echo   [0] Выход
echo.
set /p "choice=Ваш выбор: "

if "%choice%"=="1" goto run_server
if "%choice%"=="2" goto run_client
if "%choice%"=="3" goto build_project
if "%choice%"=="4" goto run_tests
if "%choice%"=="0" exit /b 0

echo [!] Неверный выбор.
timeout /t 1 >nul
cls
goto menu

:run_server
echo.
echo [^>] Запуск сервера...
if exist "build\Release\libsorter_server.exe" (
    build\Release\libsorter_server.exe
) else if exist "build\Debug\libsorter_server.exe" (
    build\Debug\libsorter_server.exe
) else if exist "build\libsorter_server.exe" (
    build\libsorter_server.exe
) else (
    echo [!] Файл libsorter_server.exe не найден. Сначала выполните сборку (пункт 3).
)
echo.
pause
goto menu

:run_client
echo.
echo [^>] Запуск клиента...
if exist "build\Release\libsorter_client.exe" (
    start "" "build\Release\libsorter_client.exe"
) else if exist "build\Debug\libsorter_client.exe" (
    start "" "build\Debug\libsorter_client.exe"
) else if exist "build\libsorter_client.exe" (
    start "" "build\libsorter_client.exe"
) else (
    echo [!] Файл libsorter_client.exe не найден. Сначала выполните сборку (пункт 3).
)
goto menu

:build_project
echo.
echo [^>] Конфигурация и сборка проекта...
cmake -B build -S .
if %errorlevel% neq 0 goto build_error
cmake --build build --config Release
if %errorlevel% neq 0 goto build_error
echo [^+] Сборка успешно завершена!
pause
goto menu

:build_error
echo [!] Ошибка при сборке. Проверьте наличие Qt6, Boost и OpenSSL в системе.
pause
goto menu

:run_tests
echo.
echo [^>] Запуск тестов...
ctest --test-dir build --output-on-failure
pause
goto menu

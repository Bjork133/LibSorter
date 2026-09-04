#!/bin/bash

echo "===================================================="
echo "           LibSorter Launcher (C++ / Qt6)"
echo "===================================================="
echo ""

# Функция сборки
build_project() {
    echo "[>] Конфигурация и сборка проекта..."
    cmake -B build -S .
    if [ $? -ne 0 ]; then
        echo "[!] Ошибка конфигурации CMake."
        return 1
    fi
    cmake --build build --config Release
    if [ $? -ne 0 ]; then
        echo "[!] Ошибка компиляции."
        return 1
    fi
    echo "[+] Сборка успешно завершена!"
    return 0
}

# Проверка наличия папки build
if [ ! -d "build" ]; then
    echo "[!] Папка 'build' не найдена."
    read -p "Хотите собрать проект сейчас? (y/N): " build_now
    if [[ "$build_now" =~ ^[Yy]$ ]]; then
        build_project
    else
        echo "[!] Сборка отменена."
        exit 1
    fi
fi

while true; do
    echo "Выберите действие:"
    echo "  [1] Запустить сервер (libsorter_server)"
    echo "  [2] Запустить клиент (libsorter_client)"
    echo "  [3] Пересобрать проект (CMake)"
    echo "  [4] Запустить тесты"
    echo "  [0] Выход"
    echo ""
    read -p "Ваш выбор: " choice

    case $choice in
        1)
            echo ""
            echo "[>] Запуск сервера..."
            if [ -f "build/libsorter_server" ]; then
                ./build/libsorter_server
            elif [ -f "build/Release/libsorter_server" ]; then
                ./build/Release/libsorter_server
            elif [ -f "build/Debug/libsorter_server" ]; then
                ./build/Debug/libsorter_server
            else
                echo "[!] Файл libsorter_server не найден. Сначала выполните сборку (пункт 3)."
            fi
            echo ""
            read -p "Нажмите Enter для продолжения..."
            ;;
        2)
            echo ""
            echo "[>] Запуск клиента..."
            if [ -f "build/libsorter_client" ]; then
                ./build/libsorter_client &
            elif [ -f "build/Release/libsorter_client" ]; then
                ./build/Release/libsorter_client &
            elif [ -f "build/Debug/libsorter_client" ]; then
                ./build/Debug/libsorter_client &
            else
                echo "[!] Файл libsorter_client не найден. Сначала выполните сборку (пункт 3)."
            fi
            ;;
        3)
            build_project
            read -p "Нажмите Enter для продолжения..."
            ;;
        4)
            echo ""
            echo "[>] Запуск тестов..."
            ctest --test-dir build --output-on-failure
            read -p "Нажмите Enter для продолжения..."
            ;;
        0)
            echo "Выход."
            exit 0
            ;;
        *)
            echo "[!] Неверный выбор."
            ;;
    esac
    clear
done

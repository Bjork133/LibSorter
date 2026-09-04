#!/bin/bash

# Function to check if a command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Check if CMake is installed
if ! command_exists cmake; then
    echo "CMake is not installed."
    read -p "Would you like to install CMake? (y/n): " INSTALL_CMAKE
    
    if [[ "$INSTALL_CMAKE" =~ ^[Yy]$ ]]; then
        echo "Installing CMake..."
        
        # Detect OS
        if [[ "$OSTYPE" == "linux-gnu"* ]]; then
            # Linux
            if command_exists apt-get; then
                sudo apt-get update && sudo apt-get install -y cmake
            elif command_exists dnf; then
                sudo dnf install -y cmake
            elif command_exists pacman; then
                sudo pacman -S --noconfirm cmake
            else
                echo "Unsupported package manager. Please install CMake manually from: https://cmake.org/download/"
                exit 1
            fi
        elif [[ "$OSTYPE" == "darwin"* ]]; then
            # macOS
            if command_exists brew; then
                brew install cmake
            else
                echo "Homebrew is not installed. Please install Homebrew first or install CMake manually from: https://cmake.org/download/"
                exit 1
            fi
        else
            echo "Unsupported operating system. Please install CMake manually from: https://cmake.org/download/"
            exit 1
        fi
        
        echo "CMake installation complete."
    else
        echo "CMake is required to build the project. Exiting."
        exit 1
    fi
fi

# Check if Ninja is installed
if ! command_exists ninja; then
    echo "Ninja is not installed."
    read -p "Would you like to install Ninja? (y/n): " INSTALL_NINJA
    
    if [[ "$INSTALL_NINJA" =~ ^[Yy]$ ]]; then
        echo "Installing Ninja..."
        
        # Detect OS
        if [[ "$OSTYPE" == "linux-gnu"* ]]; then
            # Linux
            if command_exists apt-get; then
                sudo apt-get update && sudo apt-get install -y ninja-build
            elif command_exists dnf; then
                sudo dnf install -y ninja-build
            elif command_exists pacman; then
                sudo pacman -S --noconfirm ninja
            else
                echo "Unsupported package manager. Please install Ninja manually from: https://ninja-build.org/"
                exit 1
            fi
        elif [[ "$OSTYPE" == "darwin"* ]]; then
            # macOS
            if command_exists brew; then
                brew install ninja
            else
                echo "Homebrew is not installed. Please install Homebrew first or install Ninja manually from: https://ninja-build.org/"
                exit 1
            fi
        else
            echo "Unsupported operating system. Please install Ninja manually from: https://ninja-build.org/"
            exit 1
        fi
        
        echo "Ninja installation complete."
    else
        echo "Ninja is required to build the project. Exiting."
        exit 1
    fi
fi

# Main menu function
show_menu() {
    clear
    echo "================================"
    echo "  LibSorter Launcher"
    echo "================================"
    echo ""
    echo "Select an option:"
    echo "1. Start Server"
    echo "2. Start Client"
    echo "3. Rebuild Project (CMake)"
    echo "4. Exit"
    echo ""
    read -p "Enter your choice (1-4): " CHOICE
}

# Start server function
start_server() {
    echo "Starting server..."
    cd ~/Projects/LibSorter || { echo "Error: Could not change to project directory"; return; }
    
    if [ -f "./build/libsorter_server" ]; then
        ./build/libsorter_server ./data 9443 &
        echo "Server started in background with PID $!"
    else
        echo "Error: Server executable not found. Please rebuild the project first."
    fi
    read -p "Press Enter to continue..."
}

# Start client function
start_client() {
    echo "Starting client..."
    cd ~/Projects/LibSorter || { echo "Error: Could not change to project directory"; return; }
    
    if [ -f "./build/libsorter_client" ]; then
        ./build/libsorter_client &
        echo "Client started in background with PID $!"
    else
        echo "Error: Client executable not found. Please rebuild the project first."
    fi
    read -p "Press Enter to continue..."
}

# Rebuild project function
rebuild_project() {
    echo "Rebuilding project..."
    cd ~/Projects/LibSorter || { echo "Error: Could not change to project directory"; return; }
    
    if [ -d "build" ]; then
        ninja -C build
        if [ $? -eq 0 ]; then
            echo "Build completed successfully."
        else
            echo "Build failed with error code $?."
        fi
    else
        echo "Build directory not found. Running initial CMake configuration..."
        mkdir -p build
        cd build
        cmake .. -G Ninja
        cd ..
        if [ $? -eq 0 ]; then
            echo "Initial configuration complete. Building..."
            ninja -C build
        else
            echo "CMake configuration failed."
        fi
    fi
    read -p "Press Enter to continue..."
}

# Main loop
while true; do
    show_menu
    
    case $CHOICE in
        1) start_server ;;
        2) start_client ;;
        3) rebuild_project ;;
        4) echo "Goodbye!"; exit 0 ;;
        *) echo "Invalid choice. Please try again."; sleep 2 ;;
    esac
done

print("Unix script created: launch_unix.sh")
print("\nNote: Make the script executable with: chmod +x launch_unix.sh")

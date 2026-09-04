cmake_minimum_required(VERSION 3.21)
project(LibSorter VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
cmake_policy(SET CMP0135 NEW)
cmake_policy(SET CMP0167 NEW)

option(BUILD_SERVER "Build server" ON)
option(BUILD_CLIENT "Build GUI client" ON)
option(BUILD_TESTS  "Build tests" ON)

include(FetchContent)

FetchContent_Declare(json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(json)

if(BUILD_SERVER AND NOT ANDROID)
    find_package(Boost REQUIRED)
    find_package(OpenSSL REQUIRED)
endif()

if(BUILD_CLIENT)
    find_package(Qt6 REQUIRED COMPONENTS Core Gui Quick Qml Network WebSockets)
    qt_standard_project_setup()
endif()

add_library(libsorter_core STATIC
    src/core/models/book.cpp
    src/core/models/location.cpp
    src/core/models/user.cpp
    src/core/storage/json_storage.cpp
    src/core/search/fuzzy_search.cpp
    src/common/protocol.cpp
)
target_include_directories(libsorter_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(libsorter_core PUBLIC nlohmann_json::nlohmann_json)

if(BUILD_SERVER AND NOT ANDROID)
    add_executable(libsorter_server
        src/server/main.cpp
        src/server/ws_server.cpp
        src/server/auth_middleware.cpp
        src/server/handlers/book_handler.cpp
        src/server/handlers/auth_handler.cpp
    )
    target_link_libraries(libsorter_server PRIVATE
        libsorter_core
        Boost::headers
        OpenSSL::SSL OpenSSL::Crypto
    )
    target_compile_definitions(libsorter_server PRIVATE BOOST_ASIO_NO_DEPRECATED)
endif()

if(BUILD_CLIENT)
    qt_add_executable(libsorter_client src/client/main.cpp)
    qt_add_qml_module(libsorter_client
        URI LibSorter VERSION 1.0
        QML_FILES src/client/qml/Main.qml
    )
    target_link_libraries(libsorter_client PRIVATE
        libsorter_core
        Qt6::Core Qt6::Gui Qt6::Quick Qt6::Network Qt6::WebSockets
    )
endif()

if(BUILD_TESTS)
    add_library(doctest INTERFACE)
    target_include_directories(doctest INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/tests)
    enable_testing()
    foreach(test_name book json_storage fuzzy_search protocol)
        add_executable(test_${test_name} tests/test_${test_name}.cpp)
        target_link_libraries(test_${test_name} PRIVATE libsorter_core doctest)
        add_test(NAME ${test_name} COMMAND test_${test_name})
    endforeach()
endif()

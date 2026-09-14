cmake_minimum_required(VERSION 3.20)
project(matalasMap LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Default to installed Qt 6.8.2 if CMAKE_PREFIX_PATH not passed
if(NOT DEFINED CMAKE_PREFIX_PATH AND EXISTS "C:/Qt/6.8.2/msvc2022_64")
    set(CMAKE_PREFIX_PATH "C:/Qt/6.8.2/msvc2022_64")
endif()

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui Quick Qml QuickControls2)

# Sources
set(CPP_SOURCES
    cpp/src/main.cpp
    cpp/src/bridge/WorldEditorBridge.h
    cpp/src/bridge/WorldEditorBridge.cpp
    cpp/src/bridge/CountryListModel.h
    cpp/src/bridge/CountryListModel.cpp
    cpp/src/bridge/MapCanvasItem.h
    cpp/src/bridge/MapCanvasItem.cpp
    cpp/src/bridge/matalas_ffi.h
)

# Rust Core Library paths
set(RUST_TARGET_DIR "${CMAKE_SOURCE_DIR}/target/release")

add_executable(matalasMap ${CPP_SOURCES})

target_include_directories(matalasMap PRIVATE
    cpp/src
    cpp/src/bridge
)

target_link_libraries(matalasMap PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Quick
    Qt6::Qml
    Qt6::QuickControls2
    "${RUST_TARGET_DIR}/matalas_core.dll.lib"
)

# Copy matalas_core.dll to build directory next to executable
add_custom_command(TARGET matalasMap POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${RUST_TARGET_DIR}/matalas_core.dll"
        "$<TARGET_FILE_DIR:matalasMap>"
)

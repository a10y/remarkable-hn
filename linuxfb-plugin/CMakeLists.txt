cmake_minimum_required(VERSION 3.16)

project(hn_reader_linuxfb_plugin LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

set(QTBASE_SOURCE_DIR "" CACHE PATH "Path to the Qt 6.8.2 qtbase source tree")
if(NOT EXISTS "${QTBASE_SOURCE_DIR}/src/plugins/platforms/linuxfb/main.cpp")
    message(FATAL_ERROR "QTBASE_SOURCE_DIR must point to the Qt 6.8.2 qtbase source tree")
endif()

find_package(Qt6 REQUIRED COMPONENTS Core Gui)
find_package(Qt6FbSupportPrivate REQUIRED)
find_package(Qt6InputSupportPrivate REQUIRED)

set(LINUXFB_DIR "${QTBASE_SOURCE_DIR}/src/plugins/platforms/linuxfb")
add_library(qlinuxfb MODULE
    "${LINUXFB_DIR}/main.cpp"
    "${LINUXFB_DIR}/qlinuxfbintegration.cpp"
    "${LINUXFB_DIR}/qlinuxfbintegration.h"
    "${LINUXFB_DIR}/qlinuxfbscreen.cpp"
    "${LINUXFB_DIR}/qlinuxfbscreen.h"
)

target_compile_definitions(qlinuxfb PRIVATE QT_NO_FOREACH)
target_link_libraries(qlinuxfb PRIVATE
    Qt6::Core
    Qt6::CorePrivate
    Qt6::FbSupportPrivate
    Qt6::Gui
    Qt6::GuiPrivate
    Qt6::InputSupportPrivate
)

set_target_properties(qlinuxfb PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/platforms"
    OUTPUT_NAME qlinuxfb
    PREFIX "lib"
)

cmake_minimum_required(VERSION 3.16)

project(hn_reader VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick Network)
qt_standard_project_setup(REQUIRES 6.5)

qt_add_executable(hn-reader
    src/main.cpp
    src/hnbackend.cpp
    src/hnbackend.h
    src/readability.cpp
    src/readability.h
)

qt_add_qml_module(hn-reader
    URI HnReader
    VERSION 1.0
    QML_FILES Main.qml
)

target_link_libraries(hn-reader PRIVATE Qt6::Quick Qt6::Network)

install(TARGETS hn-reader RUNTIME DESTINATION bin)

cmake_minimum_required(VERSION 3.20)

project(CreatorForge
    VERSION 0.1.0
    DESCRIPTION "Unified creative editing suite for Linux"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.4 REQUIRED COMPONENTS Widgets Multimedia MultimediaWidgets)

add_executable(creatorforge
    src/main.cpp
    src/gui/MainWindow.cpp
    src/gui/MainWindow.hpp
    src/gui/TimelineWidget.cpp
    src/gui/TimelineWidget.hpp
)

target_link_libraries(creatorforge PRIVATE Qt6::Widgets Qt6::Multimedia Qt6::MultimediaWidgets)

target_include_directories(creatorforge PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_options(creatorforge PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
)

install(TARGETS creatorforge
    RUNTIME DESTINATION bin
)

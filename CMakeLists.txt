cmake_minimum_required(VERSION 3.20)
project(InstallerCreator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)


find_package(Qt6 COMPONENTS
        Core
        Gui
        Widgets
        REQUIRED)

add_executable(InstallerCreator
        src/main.cpp
        src/widgets/MainWindow.cpp
        src/widgets/MainWindow.h
        src/widgets/MainWindow.ui
)

target_link_libraries(InstallerCreator
        Qt::Core
        Qt::Gui
        Qt::Widgets
)


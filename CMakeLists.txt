cmake_minimum_required(VERSION 3.16)

project(AirQualityMonitor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Network
    Charts
    Test  
)

# Główna aplikacja
set(SOURCES
    main.cpp
    MainWindow.cpp
    AirQualityManager.cpp
)

set(HEADERS
    MainWindow.h
    AirQualityManager.h
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_link_libraries(${PROJECT_NAME}
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    Qt6::Charts
)

# Testy jednostkowe
set(TEST_SOURCES
    tests.cpp
    MainWindow.cpp
    AirQualityManager.cpp
)

set(TEST_HEADERS
    MainWindow.h
    AirQualityManager.h
)

add_executable(AirQualityMonitorTests ${TEST_SOURCES} ${TEST_HEADERS})

target_link_libraries(AirQualityMonitorTests
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    Qt6::Charts
    Qt6::Test  
)

cmake_minimum_required(VERSION 3.16)

# Qt Creator + Ninja / Makefile : le « Type de compilation » peut être vide sans ceci (avant project).
if(NOT CMAKE_CONFIGURATION_TYPES)
    if(NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")
        set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Debug, Release, etc." FORCE)
    endif()
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release RelWithDebInfo MinSizeRel)
endif()

project(ProjetCpp VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Gui PrintSupport Charts Sql Network SerialPort)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Gui PrintSupport Charts Sql Network SerialPort)

set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    logo.qrc
    connexion.h
    connexion.cpp
    ordrefabrication.h
    ordrefabrication.cpp
    etape.h
    etape.cpp
    matierepremiere.h
    matierepremiere.cpp
    depot.h
    depot.cpp
    produit.h
    produit.cpp
    costintelligenceengine.h
    costintelligenceengine.cpp
    productanalyzer.h
    productanalyzer.cpp
    tensionevaluator.h
    tensionevaluator.cpp
    strategyengine.h
    strategyengine.cpp
    executivesummarybuilder.h
    executivesummarybuilder.cpp
    decisionengine.h
    decisionengine.cpp
    costengine.h
    costengine.cpp
    optimizationengine.h
    optimizationengine.cpp
    insightengine.h
    insightengine.cpp
    scoreengine.h
    scoreengine.cpp
    simulationengine.h
    simulationengine.cpp
    sensitivityengine.h
    sensitivityengine.cpp
    breakpointengine.h
    breakpointengine.cpp
    uianimator.h
    uianimator.cpp
    client.h
    client.cpp
    employe.h
    employe.cpp
    smartmotorcontroller.h
    smartmotorcontroller.cpp
)

if(QT_VERSION_MAJOR VERSION_GREATER_EQUAL 6)
    qt_add_executable(ProjetCpp
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
else()
    if(ANDROID)
        add_library(ProjetCpp SHARED ${PROJECT_SOURCES})
    else()
        add_executable(ProjetCpp ${PROJECT_SOURCES})
    endif()
endif()

target_link_libraries(ProjetCpp PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::PrintSupport
    Qt${QT_VERSION_MAJOR}::Charts
    Qt${QT_VERSION_MAJOR}::Sql
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::SerialPort
)

set(BUNDLE_ID_OPTION "")
if(APPLE AND QT_VERSION VERSION_LESS 6.1.0)
    set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.ProjetCpp)
endif()

if(APPLE)
    set_target_properties(ProjetCpp PROPERTIES
        ${BUNDLE_ID_OPTION}
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
        MACOSX_BUNDLE TRUE
    )
else()
    set_target_properties(ProjetCpp PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()

include(GNUInstallDirs)
install(TARGETS ProjetCpp
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR VERSION_GREATER_EQUAL 6)
    qt_finalize_executable(ProjetCpp)
endif()

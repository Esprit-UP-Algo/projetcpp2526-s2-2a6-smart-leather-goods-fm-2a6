cmake_minimum_required(VERSION 3.16)

project(projetCsmart-leather VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# MODIFIÉ : Ajout de Sql
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Sql)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Sql)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(projetCsmart-leather
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        connexion.h      # ← MODIFIÉ (plus de chemin)
        connexion.cpp    # ← MODIFIÉ (plus de chemin)
    )
else()
    if(ANDROID)
        add_library(projetCsmart-leather SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(projetCsmart-leather
            ${PROJECT_SOURCES}
        )
    endif()
endif()

# MODIFIÉ : Ajout de Sql dans les liens
target_link_libraries(projetCsmart-leather PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Sql)

if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.projetCsmart-leather)
endif()
set_target_properties(projetCsmart-leather PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS projetCsmart-leather
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(projetCsmart-leather)
endif()

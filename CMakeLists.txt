cmake_minimum_required(VERSION 3.16)

project(projetCsmart-leather VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt modules
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Sql)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Sql Charts Core Gui Network)

# ===================== SOURCES PROJET =====================
set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    connexion.h
    connexion.cpp
    client.h
    client.cpp
    smtp/smtpclient.cpp
    smtp/emailaddress.cpp
    smtp/mimemessage.cpp
    smtp/mimepart.cpp
    smtp/mimetext.cpp
)

# ===================== SMTP SOURCES (LIB PROPRE) =====================
file(GLOB SMTP_SOURCES
    ${CMAKE_SOURCE_DIR}/smtp/*.cpp
)

file(GLOB SMTP_HEADERS
    ${CMAKE_SOURCE_DIR}/smtp/*.h
)

# ===================== EXECUTABLE =====================
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(projetCsmart-leather
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        ${SMTP_SOURCES}
        ${SMTP_HEADERS}
    )
else()
    add_executable(projetCsmart-leather
        ${PROJECT_SOURCES}
        ${SMTP_SOURCES}
        ${SMTP_HEADERS}
    )
endif()

# ===================== LINK LIBRARIES =====================
target_link_libraries(projetCsmart-leather PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Sql
    Qt${QT_VERSION_MAJOR}::Charts
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Network
)

# ===================== INCLUDE PATH =====================
target_include_directories(projetCsmart-leather PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/smtp
)

# ===================== WINDOWS / MAC SETTINGS =====================
set_target_properties(projetCsmart-leather PROPERTIES
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

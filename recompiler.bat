@echo off
echo ========================================
echo   RECOMPILATION COMPLETE DU PROJET
echo ========================================
echo.

REM Configuration des chemins Qt
set QT_DIR=C:\Qt\6.7.3\mingw_64
set CMAKE_DIR=C:\Qt\Tools\CMake_64\bin
set MINGW_DIR=C:\Qt\Tools\mingw1310_64\bin
set PATH=%QT_DIR%\bin;%CMAKE_DIR%;%MINGW_DIR%;%PATH%

echo [1/5] Nettoyage du dossier build...
if exist build rmdir /s /q build
if exist .qtcreator rmdir /s /q .qtcreator
echo OK Nettoyage termine

echo.
echo [2/5] Creation du dossier build...
mkdir build
cd build
echo OK Dossier cree

echo.
echo [3/5] Configuration avec CMake...
cmake -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH=%QT_DIR% ..
if errorlevel 1 (
    echo ERREUR de configuration CMake
    pause
    exit /b 1
)
echo OK Configuration reussie

echo.
echo [4/5] Compilation...
cmake --build . --config Release
if errorlevel 1 (
    echo ERREUR de compilation
    pause
    exit /b 1
)
echo OK Compilation reussie

echo.
echo [5/5] Verification de l'executable...
if exist ProjetCpp.exe (
    echo OK ProjetCpp.exe cree avec succes!
    echo.
    echo ========================================
    echo   COMPILATION TERMINEE AVEC SUCCES!
    echo ========================================
    echo.
    echo Voulez-vous lancer l'application? (O/N)
    choice /c ON /n
    if errorlevel 2 goto end
    if errorlevel 1 ProjetCpp.exe
) else (
    echo ERREUR ProjetCpp.exe n'a pas ete cree
    pause
    exit /b 1
)

:end
pause

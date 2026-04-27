@echo off
echo ========================================
echo Compilation du projet FIL D'OR
echo ========================================
echo.

REM Recherche de Qt Creator et CMake
set QT_DIR=C:\Qt\6.7.3\mingw_64
set CMAKE_DIR=C:\Qt\Tools\CMake_64\bin
set MINGW_DIR=C:\Qt\Tools\mingw1120_64\bin

REM Vérifier si Qt est installé
if not exist "%QT_DIR%" (
    echo ERREUR: Qt 6.7.3 n'est pas installe dans %QT_DIR%
    echo Veuillez ajuster le chemin dans ce script.
    pause
    exit /b 1
)

REM Ajouter les outils au PATH
set PATH=%CMAKE_DIR%;%MINGW_DIR%;%QT_DIR%\bin;%PATH%

echo Nettoyage du dossier build...
if exist build\Desktop_Qt_6_7_3_MinGW_64_bit-Debug (
    cd build\Desktop_Qt_6_7_3_MinGW_64_bit-Debug
    mingw32-make clean
    cd ..\..
)

echo.
echo Compilation en cours...
cd build\Desktop_Qt_6_7_3_MinGW_64_bit-Debug
mingw32-make -j4

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ========================================
    echo COMPILATION REUSSIE !
    echo ========================================
    echo.
    echo L'executable se trouve dans:
    echo build\Desktop_Qt_6_7_3_MinGW_64_bit-Debug\
    echo.
    echo Voulez-vous lancer l'application ? (O/N)
    set /p LAUNCH=
    if /i "%LAUNCH%"=="O" (
        start PROJET_CPP.exe
    )
) else (
    echo.
    echo ========================================
    echo ERREUR DE COMPILATION
    echo ========================================
    echo Verifiez les erreurs ci-dessus.
)

cd ..\..
pause

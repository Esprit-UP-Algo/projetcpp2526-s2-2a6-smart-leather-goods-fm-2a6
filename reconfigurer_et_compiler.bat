@echo off
echo ========================================
echo RECONFIGURATION ET COMPILATION DU PROJET
echo ========================================
echo.

REM Définir les chemins Qt et CMake (à ajuster selon votre installation)
set QT_DIR=C:\Qt\6.7.3\mingw_64
set CMAKE_DIR=C:\Qt\Tools\CMake_64\bin
set MINGW_DIR=C:\Qt\Tools\mingw1310_64\bin

REM Ajouter au PATH
set PATH=%CMAKE_DIR%;%MINGW_DIR%;%QT_DIR%\bin;%PATH%

echo [1/4] Nettoyage du dossier build...
if exist build (
    rmdir /s /q build
    echo Build supprime
) else (
    echo Pas de build existant
)

echo.
echo [2/4] Creation du dossier build...
mkdir build
cd build

echo.
echo [3/4] Configuration avec CMake...
cmake -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH=%QT_DIR% ..

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ERREUR: La configuration CMake a echoue!
    echo.
    echo Verifiez que:
    echo - Qt est installe dans %QT_DIR%
    echo - CMake est installe dans %CMAKE_DIR%
    echo - MinGW est installe dans %MINGW_DIR%
    echo.
    pause
    exit /b 1
)

echo.
echo [4/4] Compilation...
cmake --build .

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ERREUR: La compilation a echoue!
    echo Consultez les messages d'erreur ci-dessus.
    echo.
    pause
    exit /b 1
)

echo.
echo ========================================
echo COMPILATION REUSSIE!
echo ========================================
echo.
echo L'executable se trouve dans: build\ProjetCpp.exe
echo.
echo Pour lancer l'application:
echo   cd build
echo   ProjetCpp.exe
echo.
pause

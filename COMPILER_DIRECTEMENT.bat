@echo off
echo ========================================
echo COMPILATION DIRECTE AVEC MINGW
echo ========================================
echo.

REM Définir les chemins
set QT_DIR=C:\Qt\6.7.3\mingw_64
set CMAKE_DIR=C:\Qt\Tools\CMake_64\bin
set MINGW_DIR=C:\Qt\Tools\mingw1310_64\bin

REM Ajouter au PATH
set PATH=%CMAKE_DIR%;%MINGW_DIR%;%QT_DIR%\bin;%PATH%

echo [1/4] Nettoyage...
if exist build rmdir /s /q build
mkdir build

echo.
echo [2/4] Configuration avec CMake...
cd build
cmake -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH=%QT_DIR% -DCMAKE_BUILD_TYPE=Debug ..

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ERREUR: Configuration CMake echouee!
    echo.
    echo Verifiez que:
    echo - Qt est installe dans: %QT_DIR%
    echo - MinGW est installe dans: %MINGW_DIR%
    echo - CMake est installe dans: %CMAKE_DIR%
    echo.
    pause
    exit /b 1
)

echo.
echo [3/4] Compilation...
cmake --build . --config Debug

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ERREUR: Compilation echouee!
    echo Consultez les messages d'erreur ci-dessus.
    echo.
    pause
    exit /b 1
)

echo.
echo [4/4] Verification...
if exist ProjetCpp.exe (
    echo.
    echo ========================================
    echo COMPILATION REUSSIE!
    echo ========================================
    echo.
    echo L'executable se trouve dans: build\ProjetCpp.exe
    echo.
    echo Pour lancer l'application:
    echo   ProjetCpp.exe
    echo.
    choice /C ON /M "Voulez-vous lancer l'application maintenant"
    if errorlevel 2 goto :end
    if errorlevel 1 (
        echo.
        echo Lancement de l'application...
        start ProjetCpp.exe
    )
) else (
    echo.
    echo ERREUR: L'executable n'a pas ete cree!
    echo.
)

:end
pause

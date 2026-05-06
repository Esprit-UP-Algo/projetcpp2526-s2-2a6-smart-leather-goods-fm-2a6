@echo off
echo ========================================
echo   RECONFIGURATION POUR QT CREATOR
echo ========================================
echo.

set PATH=C:\Qt\6.7.3\mingw_64\bin;C:\Qt\Tools\mingw1310_64\bin;C:\Qt\Tools\CMake_64\bin;%PATH%

echo [1/3] Nettoyage du dossier build...
cd build
del /q CMakeCache.txt 2>nul
echo OK

echo.
echo [2/3] Reconfiguration avec Debug...
C:\Qt\Tools\CMake_64\bin\cmake.exe -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH=C:\Qt\6.7.3\mingw_64 ..
if errorlevel 1 (
    echo ERREUR de configuration
    pause
    exit /b 1
)
echo OK

echo.
echo [3/3] Compilation...
C:\Qt\Tools\CMake_64\bin\cmake.exe --build .
if errorlevel 1 (
    echo ERREUR de compilation
    pause
    exit /b 1
)

echo.
echo ========================================
echo   CONFIGURATION REUSSIE!
echo ========================================
echo.
echo Maintenant dans Qt Creator:
echo 1. Fermer et rouvrir Qt Creator
echo 2. Ouvrir CMakeLists.txt
echo 3. Le projet devrait etre configure
echo.
pause

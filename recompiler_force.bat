@echo off
echo ========================================
echo   RECOMPILATION FORCEE
echo ========================================
echo.

echo [1/4] Fermeture de ProjetCpp.exe...
taskkill /F /IM ProjetCpp.exe 2>nul
timeout /t 2 /nobreak >nul
echo OK

echo.
echo [2/4] Configuration PATH...
set PATH=C:\Qt\6.7.3\mingw_64\bin;C:\Qt\Tools\mingw1310_64\bin;C:\Qt\Tools\CMake_64\bin;%PATH%
echo OK

echo.
echo [3/4] Compilation...
cd build
C:\Qt\Tools\CMake_64\bin\cmake.exe --build .
if errorlevel 1 (
    echo ERREUR de compilation
    pause
    exit /b 1
)
echo OK

echo.
echo [4/4] Verification...
if exist ProjetCpp.exe (
    echo OK ProjetCpp.exe cree avec succes!
    echo.
    echo ========================================
    echo   COMPILATION TERMINEE!
    echo ========================================
) else (
    echo ERREUR ProjetCpp.exe non cree
)

echo.
pause

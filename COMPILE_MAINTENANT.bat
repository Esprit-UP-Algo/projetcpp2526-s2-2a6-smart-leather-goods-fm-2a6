@echo off
echo ========================================
echo COMPILATION DU PROJET
echo ========================================
echo.

cd build\Desktop_Qt_6_7_3_MinGW_64_bit-Debug

echo Nettoyage...
mingw32-make clean

echo.
echo Compilation...
mingw32-make -j4

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ========================================
    echo COMPILATION REUSSIE !
    echo ========================================
    echo.
    echo Lancement de l'application...
    start PROJET_CPP.exe
) else (
    echo.
    echo ========================================
    echo ERREUR DE COMPILATION
    echo ========================================
)

pause

@echo off
echo === Compilation du projet ===
cd build
cmake --build . 2>&1
echo.
echo === Fin de la compilation ===
pause

@echo off
set PATH=C:\Qt\6.7.3\mingw_64\bin;C:\Qt\Tools\mingw1310_64\bin;C:\Qt\Tools\CMake_64\bin;%PATH%
cd build
del /F /Q ProjetCpp.exe 2>nul
C:\Qt\Tools\CMake_64\bin\cmake.exe --build .
if exist ProjetCpp.exe (
    echo SUCCES!
    dir ProjetCpp.exe
) else (
    echo ERREUR
)

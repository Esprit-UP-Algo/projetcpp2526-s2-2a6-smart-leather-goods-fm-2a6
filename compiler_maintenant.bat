@echo off
set PATH=C:\Qt\6.7.3\mingw_64\bin;C:\Qt\Tools\mingw1310_64\bin;C:\Qt\Tools\CMake_64\bin;%PATH%
cd build
C:\Qt\Tools\CMake_64\bin\cmake.exe --build . 2>&1
pause

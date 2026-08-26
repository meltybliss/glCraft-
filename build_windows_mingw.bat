@echo off
setlocal
cd /d "%~dp0"

where cmake >nul 2>nul || (
    echo [ERROR] CMake was not found in PATH.
    exit /b 1
)

where ninja >nul 2>nul || (
    echo [ERROR] Ninja was not found in PATH.
    exit /b 1
)

where g++ >nul 2>nul || (
    echo [ERROR] MinGW-w64 g++ was not found in PATH.
    exit /b 1
)

cmake --preset windows-mingw || exit /b 1
cmake --build --preset windows-mingw || exit /b 1

echo.
echo Build completed:
echo   build\windows-mingw\bin\glCraft++.exe

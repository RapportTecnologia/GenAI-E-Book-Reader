@echo off
rem This script builds the project in a clean environment to avoid PATH conflicts.

echo --- Setting up a clean build environment for MinGW ---

rem 1. Set a clean PATH, including only the necessary system directories and the compiler.
set PATH=C:\Windows\system32;C:\Windows
set PATH=%PATH%;C:\Qt\Tools\mingw1310_64\bin
set PATH=%PATH%;C:\Qt\6.9.2\mingw_64\bin
set PATH=%PATH%;C:\Program Files\CMake\bin;C:\Users\consu\AppData\Local\Programs\Python\Python310\Scripts

echo --- New PATH for this session: ---
echo %PATH%
echo.

rem 2. Clean the previous build directory.
echo --- Cleaning old build directory ---
if exist build ( rmdir /s /q build )
echo.

rem 3. Configure the project using CMake and Ninja.
echo --- Configuring project with CMake... ---
cmake -S . -B build -G "Ninja" ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DCMAKE_C_COMPILER=C:/Qt/Tools/mingw1310_64/bin/gcc.exe ^
    -DCMAKE_CXX_COMPILER=C:/Qt/Tools/mingw1310_64/bin/g++.exe ^
    -DCMAKE_PREFIX_PATH="C:/Qt/6.9.2/mingw_64"

rem Check if CMake configuration was successful
if %errorlevel% neq 0 (
    echo.
    echo ******************************************
    echo *  CMake configuration FAILED. Aborting.  *
    echo ******************************************
    goto :eof
)

echo.
echo --- CMake configuration successful. ---
echo.

rem 4. Compile the project.
echo --- Compiling project with Ninja... ---
cmake --build build -j

if %errorlevel% neq 0 (
    echo.
    echo ******************************************
    echo *      Build FAILED. Check errors.       *
    echo ******************************************
) else (
    echo.
    echo ******************************************
    echo *       Build SUCCEEDED!                 *
    echo *  Executable is in build\bin          *
    echo ******************************************
)

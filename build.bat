@echo off
setlocal

rem
set "BUILD_TYPE=Debug"
set "BUILD_DIR=build-debug"
set "RUN=1"

:parse
if /i "%~1"=="-R" (
    set "BUILD_TYPE=Release"
    set "BUILD_DIR=build-release"
) else if /i "%~1"=="-D" (
    rem
) else if /i "%~1"=="-N" (
    rem
    set "RUN=0"
) else if not "%~1"=="" (
    echo UNKNOWN: "%~1" - use -D ^(Debug^), -R ^(Release^) or -N ^(without running^).
    exit /b 1
)
shift
if not "%~1"=="" goto parse

echo === VoxelRenderer build: %BUILD_TYPE% -^> %BUILD_DIR% ===

rem
where clang-format >nul 2>nul
if %errorlevel%==0 (
    echo === Formatting source ^(clang-format^) ===
    for /r "src" %%f in (*.cpp *.hpp *.h) do (
        if /i not "%%~nxf"=="stb_image.h" clang-format -i "%%f"
    )
) else (
    echo clang-format not found - skipping formatting
)

rem
if not exist "%BUILD_DIR%\CMakeCache.txt" (
    cmake -S . -B "%BUILD_DIR%" -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=%BUILD_TYPE% || exit /b 1
)

rem
cmake --build "%BUILD_DIR%" -j || exit /b 1

if "%RUN%"=="0" (
    echo === Done: %BUILD_DIR%\VoxelRenderer.exe ===
    endlocal
    exit /b 0
)

echo === DONE - running %BUILD_DIR%\VoxelRenderer.exe ===

rem
pushd "%BUILD_DIR%"
VoxelRenderer.exe
popd

endlocal

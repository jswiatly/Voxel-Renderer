@echo off
setlocal

rem --- domyslnie Debug: rozwijasz na Debug, walidacja wlaczona ---
set "BUILD_TYPE=Debug"
set "BUILD_DIR=build-debug"

if /i "%~1"=="-R" (
    set "BUILD_TYPE=Release"
    set "BUILD_DIR=build-release"
) else if /i "%~1"=="-D" (
    rem zostaje Debug
) else if not "%~1"=="" (
    echo Nieznana opcja "%~1" - uzyj -D ^(Debug^) lub -R ^(Release^).
    exit /b 1
)

echo === Vesta build: %BUILD_TYPE% -^> %BUILD_DIR% ===

rem --- konfiguruj tylko gdy brak cache (pierwszy raz / po wyczyszczeniu) ---
if not exist "%BUILD_DIR%\CMakeCache.txt" (
    cmake -S . -B "%BUILD_DIR%" -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=%BUILD_TYPE% || exit /b 1
)

rem --- buduj ---
cmake --build "%BUILD_DIR%" -j || exit /b 1

echo === Gotowe: %BUILD_DIR%\Vesta.exe ===
endlocal
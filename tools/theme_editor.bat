@echo off
setlocal enabledelayedexpansion

REM Get the directory of the current script
set "SCRIPT_DIR=%~dp0"

REM Get the project root (parent directory of script directory)
for %%I in ("%SCRIPT_DIR%..") do set "PROJECT_ROOT=%%~fI"

REM Add project root to PYTHONPATH
if defined PYTHONPATH (
    set "PYTHONPATH=%PROJECT_ROOT%;%PYTHONPATH%"
) else (
    set "PYTHONPATH=%PROJECT_ROOT%"
)

REM Change to project root directory
cd /d "%PROJECT_ROOT%"

REM Run the Python script
python -m tools.theme_editor gui

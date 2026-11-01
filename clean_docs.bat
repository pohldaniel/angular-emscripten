@echo off
setlocal enabledelayedexpansion
set "TARGET_DIR=docs\assets\wasm"

:: Überprüfen, ob der Ordner überhaupt existiert
if not exist "%TARGET_DIR%" (
    pause
    exit /b
)

for /R "%TARGET_DIR%" %%F in (*) do (
    set "EXT=%%~xF"
    
    if /I not "!EXT!"==".js" (
        if /I not "!EXT!"==".data" (
            if /I not "!EXT!"==".wasm" (
                del /F /Q "%%F"
            )
        )
    )
)
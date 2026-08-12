@echo off
echo ==============================================
echo OmorEkushe Registry Cleaner Tool
echo ==============================================
echo.

echo Removing HKCU\SOFTWARE\BijoyEkushe...
reg delete "HKCU\SOFTWARE\BijoyEkushe" /f >nul 2>&1
if %errorlevel% equ 0 (
    echo [SUCCESS] Removed BijoyEkushe options.
) else (
    echo [INFO] BijoyEkushe key not found or already removed.
)

echo.
echo Removing HKCU\SOFTWARE\Ekushe...
reg delete "HKCU\SOFTWARE\Ekushe" /f >nul 2>&1
if %errorlevel% equ 0 (
    echo [SUCCESS] Removed Ekushe settings.
) else (
    echo [INFO] Ekushe key not found or already removed.
)

echo.
echo All registry entries removed successfully!
pause

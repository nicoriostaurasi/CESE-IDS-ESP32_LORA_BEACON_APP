@echo off
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0config_env_esp32_win.ps1" %*
exit /b %ERRORLEVEL%

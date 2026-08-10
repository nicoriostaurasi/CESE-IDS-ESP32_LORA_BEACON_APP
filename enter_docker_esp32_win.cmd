@echo off
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0enter_docker_esp32_win.ps1" %*
exit /b %ERRORLEVEL%

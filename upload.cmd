@echo off
rem Run the native Windows uploader from this checkout, regardless of cwd.
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0upload.ps1" %*
exit /b %ERRORLEVEL%

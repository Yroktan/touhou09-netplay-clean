@echo off
setlocal
set "PYTHON=%LOCALAPPDATA%\Programs\Python\Python311\python.exe"
if exist "%PYTHON%" goto local_python
py -3 -u -X utf8 "%~dp0scripts\build.py" %*
goto finished
:local_python
"%PYTHON%" -u -X utf8 "%~dp0scripts\build.py" %*
:finished
set "RESULT=%ERRORLEVEL%"
pause
exit /b %RESULT%

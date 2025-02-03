@echo off
setlocal enabledelayedexpansion

for %%i in (4,8,12,15,18,21,23) do (
    copy "%~dp0Input\FR\input_%%i.inp" input.inp
    copy "%~dp0History\FR\history_%%i.inp" history.inp
	start /wait Euler.exe
	copy output.out "%~dp0Output\FR\output_%%i.out"
	del output.out
)
pause
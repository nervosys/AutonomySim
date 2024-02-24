REM ----------------------------------------------------------------------------------------
REM  Filename
REM    recursive_rename.cmd
REM  Description
REM    Script to recursively rename files and directories using `ren`
REM  Author
REM    Adam Erickson (Nervosys)
REM  Date
REM    2023-11-21
REM  Notes
REM    ...
REM ----------------------------------------------------------------------------------------

@echo on

setlocal

set OLD=.bat
set NEW=.cmd

REM where p=path,n=name,i=filename; removed "delims="
for /f %%i in (dir /b /s *%OLD%) do call ren %%i %%~ni%NEW%

exit /b 0

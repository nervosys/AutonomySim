REM clean_rebuild.cmd
REM
REM Updated for Windows 10/11
REM

@echo on

REM //---------- set up variable ----------
setlocal

set ROOT_DIR=%cd%

rd /s/q external
git clean -ffdx
git pull

REM // Redirect command line arguments
build.cmd %*

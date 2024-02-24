REM @echo off

REM //---------- set up variable ----------
setlocal

set ROOT_DIR=%cd%

rd /s/q external
git clean -ffdx
git pull

REM // Redirect command line arguments
build.cmd %*

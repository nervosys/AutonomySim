@echo off

REM //---------- set up variable ----------
setlocal

set ROOT_DIR=%cd%
REM pushd %~dp0

:success
@echo "Task completed."
goto end

:failed
@echo "Task has failed."
goto end

:end
popd

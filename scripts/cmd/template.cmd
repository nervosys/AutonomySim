REM template.cmd
REM
REM Updated for Windows 10/11
REM

@echo on

REM //---------- set up variable ----------
setlocal

set ROOT_DIR=%cd%
REM pushd %~dp0

:success
echo "Task completed."
goto :end

:failed
echo "Task has failed."
goto :end

:end
popd

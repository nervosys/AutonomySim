@echo off
REM //---------- set up variable ----------
setlocal
set ROOT_DIR=%~dp0

set AutonomySimPath=%1

REM default path works for Blocks environment
if "%AutonomySimPath%"=="" set "AutonomySimPath=..\..\.."

IF NOT EXIST "%AutonomySimPath%" (
	echo "AutonomySimPath %AutonomySimPath% was not found"
	goto :failed
)

echo Using AutonomySimPath = %AutonomySimPath%

robocopy /MIR "%AutonomySimPath%\Unreal\Plugins\AutonomySim" Plugins\AutonomySim /XD temp *. /njh /njs /ndl /np
robocopy /MIR "%AutonomySimPath%\AutonomyLib" Plugins\AutonomySim\Source\AutonomyLib /XD temp *. /njh /njs /ndl /np
robocopy  /njh /njs /ndl /np "%AutonomySimPath%\Unreal\Environments\Blocks" "." *.cmd 
robocopy  /njh /njs /ndl /np "%AutonomySimPath%\Unreal\Environments\Blocks" "." *.sh  
rem robocopy /njh /njs /ndl /np "%AutonomySimPath%" "." *.gitignore

REM cmd /c clean.cmd
REM cmd /c GenerateProjectFiles.cmd

goto :done

:failed
echo Error occured while updating.
exit /b 1

:done
REM if "%1"=="" pause
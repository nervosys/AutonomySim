@echo off

REM //---------- Set variables ----------
setlocal
REM set PROJECT_DIR=%~dp0  REM %~dp0 = directory path of the batch file (AutonomySim/scripts)
REM %cd%  = current working directory path (AutonomySim)
set PROJECT_DIR=%cd%
set SCRIPT_DIR=%PROJECT_DIR%\scripts
set RPCLIB_VERSION=2.3.0
set CPU_COUNT_MAX=12
set PLATFORM=x64

REM // Check command line arguments
set "fullPolyCar="
set "buildMode="

REM // Check Visual Studio version
if "%VisualStudioVersion%" == "" (
    echo.
    echo ERROR: command must be run from VS2022 Developer Command Prompt or Developer PowerShell.
    goto :buildfailed_nomsg
)

if "%VisualStudioVersion%" lss "17.0" (
    echo.
    echo AutonomySim supports up to Unreal Engine 5.3 and Visual Studio 2022.
    echo Here are few easy steps to perform the upgrade:
    echo    https://github.com/nervosys/AutonomySim/blob/main/docs/unreal_upgrade.md
    goto :buildfailed_nomsg
)

REM //---------- Parse arguments ----------
if "%1"=="" goto noargs
if "%1"=="--full-poly-car" set "fullPolyCar=y"
if "%1"=="--Debug" set "buildMode=Debug"
if "%1"=="--Release" set "buildMode=Release"
if "%1"=="--RelWithDebInfo" set "buildMode=RelWithDebInfo"

if "%2"=="" goto noargs
if "%2"=="--Debug" set "buildMode=Debug"
if "%2"=="--Release" set "buildMode=Release"
if "%2"=="--RelWithDebInfo" set "buildMode=RelWithDebInfo"

:noargs
set powershell=powershell
where powershell > nul 2>&1
if ERRORLEVEL 1 goto :pwsh
echo Found `powershell` command. && goto start

:pwsh
set powershell=pwsh
where pwsh > nul 2>&1
if ERRORLEVEL 1 goto :nopwsh
set PWSHV7=1
echo Found `pwsh` command. && goto start

:nopwsh
echo PowerShell or `pwsh` not found, please install it: `winget install --id Microsoft.Powershell --source winget`
goto :eof

:start
cd %PROJECT_DIR%

echo.
echo -----------------------------------------------------------------------------------------
echo Parameters
echo -----------------------------------------------------------------------------------------
echo Project directory:  %PROJECT_DIR%
echo Script directory:   %SCRIPT_DIR%
echo RPClib version:     %RPCLIB_VERSION%
echo Full-polycount car: %fullPolyCar%
echo Build mode:         %buildMode%
echo Platform:           %PLATFORM%
echo CPU count max:      %CPU_COUNT_MAX%
echo PowerShell command: %powershell%
echo -----------------------------------------------------------------------------------------
echo.

REM //---------- Check cmake version ----------
call scripts\check_cmake.cmd
if ERRORLEVEL 1 (
  call scripts\check_cmake.cmd
  if ERRORLEVEL 1 (
    echo(
    echo ERROR: CMake not installed correctly.
    goto :buildfailed
  )
)

REM //---------- Create directories ----------
if not exist tmp mkdir tmp
if not exist external mkdir external

REM //---------- Get rpclib ----------
REM if not exist external\rpclib mkdir external\rpclib

if not exist external\rpclib\rpclib-%RPCLIB_VERSION% (
    REM // Leave some blank lines because %powershell% shows download banner at top of console
    echo.
    echo.
    echo.
    echo -----------------------------------------------------------------------------------------
    echo Downloading rpclib
    echo -----------------------------------------------------------------------------------------
    echo.

    @echo on
    if "%PWSHV7%" == "" (
        %powershell% -command "& { [Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12; iwr https://github.com/rpclib/rpclib/archive/v2.3.0.zip -OutFile tmp\rpclib.zip }"
    ) else (
        %powershell% -command "iwr https://github.com/rpclib/rpclib/archive/v2.3.0.zip -OutFile tmp\rpclib.zip"
    )
    @echo off
    
    REM // Remove any previous versions
    rmdir external\rpclib /q /s

    %powershell% -command "Expand-Archive -Path tmp\rpclib.zip -DestinationPath external\rpclib"
    del tmp\rpclib.zip /q
    
    REM // Fail the build if unable to download rpclib
    if not exist external\rpclib\rpclib-%RPCLIB_VERSION% (
        echo ERROR: Unable to download rpclib. Stopping build.
        goto :buildfailed
    )
)

REM //---------- Build rpclib binaries ------------
echo Building rpclib with CMake...
if not exist external\rpclib\rpclib-%RPCLIB_VERSION%\build mkdir external\rpclib\rpclib-%RPCLIB_VERSION%\build
cd external\rpclib\rpclib-%RPCLIB_VERSION%\build

cmake -G"Visual Studio 17 2022" ..

if "%buildMode%" == "" (
    cmake --build .
    cmake --build . --config Release
) else (
    cmake --build . --config %buildMode%
)

if ERRORLEVEL 1 goto :buildfailed
cd %PROJECT_DIR%
echo Current directory: %PROJECT_DIR%

REM //---------- Copy rpclib binaries and include folder inside AutonomyLib folder ----------
set RPCLIB_TARGET_LIB=AutonomyLib\deps\rpclib\lib\x64
if not exist %RPCLIB_TARGET_LIB% mkdir %RPCLIB_TARGET_LIB%
set RPCLIB_TARGET_INCLUDE=AutonomyLib\deps\rpclib\include
if not exist %RPCLIB_TARGET_INCLUDE% mkdir %RPCLIB_TARGET_INCLUDE%
robocopy /MIR external\rpclib\rpclib-%RPCLIB_VERSION%\include %RPCLIB_TARGET_INCLUDE%

if "%buildMode%" == "" (
    robocopy /MIR external\rpclib\rpclib-%RPCLIB_VERSION%\build\Debug %RPCLIB_TARGET_LIB%\Debug
    robocopy /MIR external\rpclib\rpclib-%RPCLIB_VERSION%\build\Release %RPCLIB_TARGET_LIB%\Release
) else (
    robocopy /MIR external\rpclib\rpclib-%RPCLIB_VERSION%\build\%buildMode% %RPCLIB_TARGET_LIB%\%buildMode%
)

REM //---------- Get High-polycount Car Model ------------
if not exist Unreal\Plugins\AutonomySim\Content\VehicleAdv mkdir Unreal\Plugins\AutonomySim\Content\VehicleAdv
if not exist Unreal\Plugins\AutonomySim\Content\VehicleAdv\SUV\v1.2.0 (
    if %fullPolyCar% == 'y' or %fullPolyCar% == 'yes' (
        REM //leave some blank lines because %powershell% shows download banner at top of console
        echo.
        echo.
        echo.
        echo -----------------------------------------------------------------------------------------
        echo Downloading ~37MB of high-poly car assets...
        echo To perform the installation without these assets, run `build.cmd --no-full-poly-car`
        echo -----------------------------------------------------------------------------------------
        echo.

        if exist tmp\suv_download rmdir tmp\suv_download /q /s
        mkdir tmp\suv_download
        @echo on
        REM %powershell% -command "& { Start-BitsTransfer -Source https://github.com/nervosys/AutonomySim/releases/download/v1.2.0/car_assets.zip -Destination tmp\suv_download\car_assets.zip }"
        REM %powershell% -command "& { (New-Object System.Net.WebClient).DownloadFile('https://github.com/nervosys/AutonomySim/releases/download/v1.2.0/car_assets.zip', tmp\suv_download\car_assets.zip) }"
        if "%PWSHV7%" == "" (
            %powershell% -command "& { [Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12; iwr https://github.com/nervosys/AutonomySim/releases/download/v2.0.0-beta.0/car_assets.zip -OutFile tmp\suv_download\car_assets.zip }"
        ) else (
            %powershell% -command "iwr https://github.com/nervosys/AutonomySim/releases/download/v2.0.0-beta.0/car_assets.zip -OutFile tmp\suv_download\car_assets.zip"
        )
        @echo off
        rmdir Unreal\Plugins\AutonomySim\Content\VehicleAdv\SUV /q /s
        %powershell% -command "Expand-Archive -Path tmp\suv_download\car_assets.zip -DestinationPath Unreal\Plugins\AutonomySim\Content\VehicleAdv"
        rmdir tmp\suv_download /q /s
        
        REM //Don't fail the build if the high-poly car is unable to be downloaded
        REM //Instead, just notify users that the gokart will be used.
        if not exist Unreal\Plugins\AutonomySim\Content\VehicleAdv\SUV echo "Download of high-polycount SUV failed. Your AutonomySim build will use the default vehicle instead."
    ) else (
        echo Skipping download of high-poly car asset. Default Unreal Engine vehicle will be used.
    )
)

REM //---------- Get Eigen library ----------
if not exist AutonomyLib\deps mkdir AutonomyLib\deps
if not exist AutonomyLib\deps\eigen3 (
    if "%PWSHV7%" == "" (
        %powershell% -command "& { [Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12; iwr https://gitlab.com/libeigen/eigen/-/archive/3.3.7/eigen-3.3.7.zip -OutFile tmp\eigen3.zip }"
    ) else (
        %powershell% -command "iwr https://gitlab.com/libeigen/eigen/-/archive/3.3.7/eigen-3.3.7.zip -OutFile tmp\eigen3.zip"
    )
    %powershell% -command "Expand-Archive -Path tmp\eigen3.zip -DestinationPath AutonomyLib\deps"
    %powershell% -command "Move-Item -Path AutonomyLib\deps\eigen* -Destination AutonomyLib\deps\del_eigen"
    REM move AutonomyLib\deps\eigen* AutonomyLib\deps\del_eigen
    mkdir AutonomyLib\deps\eigen3
    move AutonomyLib\deps\del_eigen\Eigen AutonomyLib\deps\eigen3\Eigen
    rmdir AutonomyLib\deps\del_eigen /q /s
    del tmp\eigen3.zip /q
)
if not exist AutonomyLib\deps\eigen3 goto :buildfailed

REM //---------- We now have all dependencies to compile AutonomySim.sln, which will also compile MavLinkCom ----------
if "%buildMode%" == "" (
    msbuild -maxcpucount:%CPU_COUNT_MAX% /p:Platform=%PLATFORM% /p:Configuration=Debug AutonomySim.sln
    if ERRORLEVEL 1 goto :buildfailed
    msbuild -maxcpucount:%CPU_COUNT_MAX% /p:Platform=%PLATFORM% /p:Configuration=Release AutonomySim.sln 
    if ERRORLEVEL 1 goto :buildfailed
) else (
    msbuild -maxcpucount:%CPU_COUNT_MAX% /p:Platform=%PLATFORM% /p:Configuration=%buildMode% AutonomySim.sln
    if ERRORLEVEL 1 goto :buildfailed
)

REM //---------- Copy binaries and includes for MavLinkCom in deps ----------
set MAVLINK_TARGET_LIB=AutonomyLib\deps\MavLinkCom\lib
if not exist %MAVLINK_TARGET_LIB% mkdir %MAVLINK_TARGET_LIB%
set MAVLINK_TARGET_INCLUDE=AutonomyLib\deps\MavLinkCom\include
if not exist %MAVLINK_TARGET_INCLUDE% mkdir %MAVLINK_TARGET_INCLUDE%
robocopy /MIR MavLinkCom\include %MAVLINK_TARGET_INCLUDE%
robocopy /MIR MavLinkCom\lib %MAVLINK_TARGET_LIB%

REM //---------- All of our outputs go to Unreal/Plugins folder ----------
if not exist Unreal\Plugins\AutonomySim\Source\AutonomyLib mkdir Unreal\Plugins\AutonomySim\Source\AutonomyLib
robocopy /MIR AutonomyLib Unreal\Plugins\AutonomySim\Source\AutonomyLib  /XD temp *. /njh /njs /ndl /np
copy /y AutonomySim.props Unreal\Plugins\AutonomySim\Source\AutonomyLib

REM //---------- Update all Unreal environments ----------
for /D %%E in (Unreal\Environments\*) do (
    cd %%E
    call .\update_from_git.cmd ..\..\..
    cd ..\..\..
)

REM //---------- Done building ----------
exit /b 0

REM //---------- Fail and cleanup ----------
:buildfailed
echo.
echo ERROR: build failed! Review previous log messages. 1>&2
goto :cleanup

:buildfailed_nomsg
cd %PROJECT_DIR% 
goto :cleanup

:cleanup
rmdir tmp /q /s
rmdir external /q /s
exit /b 1

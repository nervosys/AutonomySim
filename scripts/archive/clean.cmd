rd /s/q AutonomyLib\lib
rd /s/q AutonomyLib\deps\MavLinkCom
rd /s/q AutonomyLib\deps\rpclib
rd /s/q external\rpclib\build

msbuild /p:Platform=x64 /p:Configuration=Debug AutonomySim.sln /t:Clean
if ERRORLEVEL 1 goto :buildfailed
msbuild /p:Platform=x64 /p:Configuration=Release AutonomySim.sln /t:Clean
if ERRORLEVEL 1 goto :buildfailed
goto :eof

:buildfailed
echo #### Build failed
goto :eof
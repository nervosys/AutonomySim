REM check_cmake.cmd
REM
REM Updated for Windows 10/11
REM

@echo on

REM //---------- set up variable ----------
setlocal

REM set ROOT_DIR=%~dp0
set cmake_min_majmin=3.14
REM set cmake_version=

where /q cmake
if %ERRORLEVEL%==0 (
	for /F "tokens=3" %%a in ( 'cmake --version ^| find "cmake version"' ) do (
		set cmake_version=%%a
		echo "CMake version: %%a"
	)
	if not defined cmake_version (
	  echo "Unable to get version of CMake." >&2
	  exit /b 2
	)
) else (
  echo "CMake was not found in path."
  goto :download_install
)

for /F "tokens=1,2 delims=." %%a in ( "%cmake_version%" ) do (
  set "cmake_ver_major=%%a"
  set "cmake_ver_minor=%%b"
)

if not defined cmake_ver_major or not defined cmake_ver_minor (
	echo "CMake version not defined. Exiting."
	exit /b 1
)

set cmake_majmin=%cmake_ver_major:~-4%.%cmake_ver_minor:~-4%
echo "CMake version major,minor: %cmake_majmin%"

if %cmake_majmin% lss %cmake_min_majmin% (
  echo.
  echo "Newer AutonomySim requires cmake verion %cmake_min_majmin% but you have %cmake_majmin% which is older." >&2
  goto :download_install
)

echo "Found CMake version: %cmake_version%"
exit /b 0

:download_install
set /p choice="Press any key to download and install cmake (make sure to add it in path in install options)"
if not exist %temp%\cmake-3.14.7-win64-x64.msi (
	REM @echo on
	powershell -command "& { iwr https://cmake.org/files/v3.14/cmake-3.14.7-win64-x64.msi -OutFile %temp%\cmake-3.14.7-win64-x64.msi }"
	REM @echo off
)
msiexec.exe /i %temp%\cmake-3.14.7-win64-x64.msi
exit /b 1

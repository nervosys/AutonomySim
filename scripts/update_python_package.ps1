# update_package.ps1

$confirmation = Read-Host -Prompt "Confirming package version was incremented in `setup.py` [y/N]"
if ($confirmation -ne 'y') { Exit 1 }

$PYTHON_EXE = (Get-Command python.exe).Source
$PIP_EXE = (Get-Command pip.exe).Source
$TWINE_EXE = (Get-Command pip.exe).Source

Write-Output "Found the following executables:"
Write-Output "`tpython: $($PYTHON_EXE)`n`tpip: $($PIP_EXE)`n`ttwine: $($TWINE_EXE)"

Start-Process $PYTHON_EXE -ArgumentList "setup.py", "sdist" -Wait -NoNewWindow
Start-Process $TWINE_EXE  -ArgumentList "upload", "--repository-url", "https://upload.pypi.org/legacy/ dist/*"

# Start-Process $PIP_EXE -ArgumentList "install autonomysim","--upgrade" -Wait -NoNewWindow
# Start-Process $PIP_EXE -ArgumentList "show autonomysim" -Wait -NoNewWindow

# Start-Process $PIP_EXE -ArgumentList "install yolk3k" -Wait -NoNewWindow
# Start-Process "yolk" -ArgumentList "-V","autonomysim" -Wait -NoNewWindow

Write-Output "Python package update complete."

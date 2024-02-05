# Start AutonomySim on Azure VM

# Script parameters
$AutonomySimExecutable = "C:\AutonomySim\Blocks\blocks.exe"
$AutonomySimProcessName = "Blocks"

# Ensure proper path
$env:Path = 
    [System.Environment]::GetEnvironmentVariable("Path","Machine") + ";" +
    [System.Environment]::GetEnvironmentVariable("Path","User")

# Install python app requirements
pip3 install -r .\app\requirements.txt

# Overwrite AutonomySim configuration
New-Item -ItemType Directory -Force -Path $env:USERPROFILE\Documents\AutonomySim\
copy .\app\settings.json $env:USERPROFILE\Documents\AutonomySim\

# Kill previous AutonomySim instance
Stop-Process -Name $AutonomySimProcessName -Force -ErrorAction SilentlyContinue
sleep 2

# Start new AutonomySim instance
Start-Process -NoNewWindow  -FilePath $AutonomySimExecutable -ArgumentList "-RenderOffScreen"
echo "Starting the AutonomySim environment has completed."

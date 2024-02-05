PAUSE Make sure to increment version in setup.py. Continue?
python setup.py sdist
twine upload --repository-url https://upload.pypi.org/legacy/ dist/*
REM pip install AutonomySim --upgrade
REM pip show AutonomySim

REM pip install yolk3k
REM yolk -V AutonomySim

@echo off
echo Checking out project and filters file
p4 edit evepathfinder.vcxproj
p4 edit evepathfinder.vcxproj.filters
echo Regenerating
..\..\..\..\..\..\shared_tools\python\27\python.exe ..\..\..\carbon\tools\ProjectFileGenerator\ProjectFileGenerator.py -i evePathfinder.ccpproj
pause
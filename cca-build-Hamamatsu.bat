cls

python mvn_build.py hamamatsu -o
pause

python mvn_build.py -f pom-win-dev.xml device -o
pause

..\Lima-scripts\lima2ds.exe dev

xcopy /y /d c:\DeviceServers\* E:\DeviceServers\
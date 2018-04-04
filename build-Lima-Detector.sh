#!/bin/bash

cp -R "/home/nanoscopium/CC/Jungfrau/DeviceSources/bin64/YAT4Tango-amd64-Linux-gcc-shared-release" "/home/nanoscopium/.m2/repository/fr/soleil/lib"

read -rsn1 -p"Press any key to continue";echo

cd applications/tango/cpp

mvn clean install -f pom_64.xml -o
#python mvn_build.py -f pom-linux-dev_64.xml device -o
read -rsn1 -p"Press any key to continue";echo

cd ../../..

cp "applications/tango/cpp/target/nar/bin/amd64-Linux-g++/ds_LimaDetector" "../../DeviceServers/ds_LimaDetector"
#tclsh ../Lima-scripts/lima2ds-64.tcl dev
read -rsn1 -p"Press any key to continue";echo

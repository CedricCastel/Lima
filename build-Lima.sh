#!/bin/bash

cp -R "/home/nanoscopium/CC/Jungfrau/DeviceSources/bin64/YAT-amd64-Linux-gcc-shared-release" "/home/nanoscopium/.m2/repository/fr/soleil/lib"

cp -R "/home/nanoscopium/CC/Jungfrau/DeviceSources/bin64/NexusCPP-amd64-Linux-gcc-shared-release" "/home/nanoscopium/.m2/repository/fr/soleil/lib"

read -rsn1 -p"Press any key to continue";echo

#~/CC/Jungfrau/DeviceSources/maven/current/bin/mvn clean install -f pom_64.xml
mvn clean install -f pom_64.xml
read -rsn1 -p"Press any key to continue";echo


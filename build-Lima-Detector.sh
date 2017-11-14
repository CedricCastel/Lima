#!/bin/bash

cp -R "/home/informatique/ica/langlois/.m2/repository/fr/soleil/lib/YAT4Tango-amd64-Linux-gcc-shared-release" "/home/informatique/ica/castel/.m2/repository/fr/soleil/lib"

read -rsn1 -p"Press any key to continue";echo

python mvn_build.py -f pom-linux-dev_64.xml device
read -rsn1 -p"Press any key to continue";echo

tclsh ../Lima-scripts/lima2ds-64.tcl dev
read -rsn1 -p"Press any key to continue";echo


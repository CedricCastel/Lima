#!/bin/bash

python mvn_build.py simulator
read -rsn1 -p"Press any key to continue";echo

python mvn_build.py -f pom-linux-dev.xml device
read -rsn1 -p"Press any key to continue";echo

tclsh ../Lima-scripts/lima2ds.tcl dev
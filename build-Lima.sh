#!/bin/bash

cp -R "/home/informatique/ica/langlois/.m2/repository/fr/soleil/lib/YAT-amd64-Linux-gcc-shared-release" "/home/informatique/ica/castel/.m2/repository/fr/soleil/lib"

cp -R "/home/informatique/ica/langlois/.m2/repository/fr/soleil/lib/NexusCPP-amd64-Linux-gcc-shared-release" "/home/informatique/ica/castel/.m2/repository/fr/soleil/lib"

read -rsn1 -p"Press any key to continue";echo

mvn clean install -f pom_64.xml
read -rsn1 -p"Press any key to continue";echo


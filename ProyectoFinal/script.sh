#!/bin/bash
# ? Construir el proyecto
# xcodebuild -scheme Acelerometro -configuration Debug -sdk iphoneos - build

# ? Correr el proyecto
ios-deploy --bundle /Users/bryanvargas/Library/Developer/Xcode/DerivedData/Acelerometro-gfpdsvexyxavbccvsonezprxnyji/Build/Products/Debug-iphoneos/Acelerometro.app --debug > rawdata.txt &

# ? Procesar los datos
while true; do
  sleep 0.333
  tail -n 2 rawdata.txt | head -n 1 | sed -E 's/.*x ([0-9.-]+) y ([0-9.-]+) z ([0-9.-]+).*/\1,\2,\3/' > values.csv
done


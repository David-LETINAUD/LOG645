#!/bin/bash
cd tests/
for f in *.cpp
do mpic++ $f ../src/output/*.cpp ../src/solver/*.cpp ../src/matrix/*.cpp -o output/$(echo $f | sed s/.cpp//g)
done

for f in output/*
do echo $f && $f
done

rm output/*

#!/bin/bash

for (( i = 0; i < 255; i++ ))
do
	./simulador -level $i -verbose
	mv game.ppm $i.ppm
	mv $i.ppm images_of_levels
	convert images_of_levels/$i.ppm -scale 500x500 images_of_levels/$i.png
done

cd images_of_levels
rm *.ppm

#for FILE in `ls *.ppm`
#do
#	convert $FILE -scale 500x500 $FILE.png
#done
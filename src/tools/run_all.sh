#!/bin/sh
#OUTPUT="${OUTPUT:=test.txt}"
for dir in ~/AA_DOA/src/data/rss360/*.dat; do
	echo "\n${dir}"
	OUTPUT=$(echo ${dir} | sed 's/\.dat/_tau.bin/')
	#echo $OUTPUT
	cat ~/ncpa/libNCPA/data/xstream_1khz.bin | ~/ncpa/libNCPA/bin/binary2human --type double --N_channels 1 | head -n 96000 | ./noisePropagator --arrayGeometry AA_geometry.tsv --signalInfo "${dir}" --dataSample 100 -gD > "$OUTPUT"
	#OUTPUT=$(echo ${dir} | sed 's/.dat/.bin/')
	#cat ~/ncpa/libNCPA/data/xstream_1khz.bin | ~/ncpa/libNCPA/bin/binary2human --type double --N_channels 1 | head -n 96000 | ./a.out --arrayGeometry AA_geometry.tsv --signalInfo "${dir}" --dataSample 100 > "$OUTPUT"
done

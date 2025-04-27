#!/bin/sh
#OUTPUT="${OUTPUT:=test.txt}"
for dir in ~/AA_DOA/src/data/raw_rss360/*.dat; do
	echo "\n${dir}"
	#OUTPUT=$(echo ${dir} | sed 's/\.dat/_tau.bin/')
	#echo $OUTPUT
	#cat ~/ncpa/libNCPA/data/xstream_1khz.bin | ~/ncpa/libNCPA/bin/binary2human --type double --N_channels 1 | head -n 96000 | ./noisePropagator --arrayGeometry AA_geometry.tsv --signalInfo "${dir}" --dataSample 100 -gD > "$OUTPUT"
	OUTPUT=$(echo ${dir} | sed 's/\.dat/_raw.bin/')
	cat ~/AA_DOA/src/tools/signal_1khz.bin | ./noisePropagator --arrayGeometry AA_geometry.tsv --signalInfo "${dir}" --dataSample 1 > "test70.bin"
done

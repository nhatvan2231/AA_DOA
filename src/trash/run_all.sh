#!/bin/sh
#OUTPUT="${OUTPUT:=test.txt}"
touch /tmp/dummy.txt
dummy_file="/tmp/dummy.txt"
for dir in ~/AA_DOA/src/data/raw_rss360/*.dat; do
	#OUTPUT=$(echo ${dir} | sed 's/\.dat/_tau.bin/')
	OUTPUT=$(echo ${dir} | sed 's/\.dat/_raw.bin/')
	echo ${OUTPUT}
	>${OUTPUT}
	while read -r line; do
		l="$line"
		echo "$line" > $dummy_file
		cat ~/AA_DOA/src/tools/signal_1khz.bin | ./noisePropagator --arrayGeometry AA_geometry.tsv --signalInfo ${dummy_file} | tee -a ${OUTPUT} >/dev/null
		# >> "test72.bin"

	done < "$dir"
	#echo "\n${dir}"
	##echo $OUTPUT
	##cat ~/ncpa/libNCPA/data/xstream_1khz.bin | ~/ncpa/libNCPA/bin/binary2human --type double --N_channels 1 | head -n 96000 | ./noisePropagator --arrayGeometry AA_geometry.tsv --signalInfo "${dir}" --dataSample 100 -gD > "$OUTPUT"
done

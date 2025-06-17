#!/bin/sh

# default variable
default_dir="$HOME/AA_DOA/src/data/rss" # Directory to save data
default_lb=0.0		# lower angle bound
default_ub=360.0	# upper angle bound
default_na=100		# number of sample per file
default_sa=20		# number of file
default_s=0			# random sound speed std
default_geo="$HOME/AA_DOA/src/tools/AA_geometry.tsv"
default_sig="$HOME/AA_DOA/src/tools/signal_1khz.bin"

# function for yes or no prompt
yes_no_prompt() {
	while true; do
		#prompt=$1
		printf "$1 [Y/n]: "; read -r yn
		yn=${yn:-"yes"}
		case "$yn" in
			#[yY]|[yY][eE][sS])
			[yY] | [yY][eE][sS])
				return 0;
				;;
			[nN] | [nN][oO])
				return 1;
				;;
			*)
				printf "Please answer yes or no\n"
				;;
		esac
	done
}

# Prompting for directory to save data
printf "Enter directory to save [%s]: " "$default_dir"; read -r dir
dir=${dir:-$default_dir}
dir_info=$dir/signal_info
dir_pres=$dir/signal_pres
dir_delay=$dir/signal_delay
mkdir -p $dir_info $dir_pres $dir_delay

# Prompting to generate source signals information
yes_no_prompt "Generate random source signal information?" && {
	printf "Enter Lower angle bound [%s]: " "$default_lb"; read -r lb
	lb=${lb:-$default_lb}
	printf "Enter Upper angle bound [%s]: " "$default_ub"; read -r ub
	ub=${ub:-$default_ub}
	printf "Enter Number of sample per file [%s]: " "$default_na"; read -r na
	na=${na:-$default_na}
	printf "Enter Number of file [%s]: " "$default_sa"; read -r sa
	sa=${sa:-$default_sa}
	printf "Enter Random sound speed standard deviation [%s]: " "$default_s"; read -r s
	s=${s:-$default_s}

	# Generate source signals information
	printf "Generating random source signals information...\n"
	./rnd_signal_info -dir "$dir_info" -lB $lb -uB $ub -nA $na -sA $sa -s $s
	printf "Done!\n"
}
# Prompt propagating pressure wave for each microphones
yes_no_prompt "Propagating pressure wave for each microphones?" && gen_pres=1 || gen_pres=0
# Prompt get the time delay
yes_no_prompt "Get the time delay?" && gen_delay=1 || gen_pres=0

# Prompt propagating pressure wave for each microphones
yes_no_prompt "Propagating pressure wave for each microphones?" && {
	# Prompt for array source signal file
	printf "Enter source signal file [%s]: " "$default_sig"; read -r sig
	sig=${sig:-$default_sig}

	# Prompt for array geometry file
	printf "Enter Array Geometry file [%s]: " "$default_geo"; read -r geo
	geo=${geo:-$default_geo}

	# dummy file to dump a single signal file for generating pressure
	dummy_file="/tmp/dummy.txt"
	touch $dummy_file
	printf "Generate data...\n"
	for d in $dir_info/*.dat; do
	#	# substitute file name
		[ $gen_pres=1 ] &&  {
			file_pres=$(echo $d | sed 's/info/pres/g' | sed 's/\.dat/\.bin/')
			echo $file_pres
		}
		[ $gen_delay=1 ] &&  {
			file_delay=$(echo $d | sed 's/info/delay/g' | sed 's/\.dat/\.bin/')
			echo $file_delay
		}
		while read -r line; do
			echo "$line" > $dummy_file
			[ $gen_pres=1 ] &&  cat $sig | ./noisePropagator --arrayGeometry $geo --signalInfo $dummy_file | tee -a $file_pres > /dev/null
			[ $gen_delay=1 ] &&  cat $sig | ./noisePropagator --arrayGeometry $geo --signalInfo $dummy_file --getDelay | tee -a $file_delay > /dev/null
		done < $d
	done
	printf "Done!\n"
}


# Copy geometry and signal file into directory
cp $geo $dir
cp $sig $dir

# make a README
touch $dir/README.md
printf	"%s\n" \
			"Random sources signal at different angles and sound speed\n" \
			"* signal\_info -> source signal information:\n" \
			"	- Sampling Frequency: 48000.0\n" \
			"	- Sound speed: mean=0, std=$s\n" \
			"	- Angle: lower bound=$lb, upper bound=$ub\n" \
			"* signal\_pres -> pressure wave received by microphones\n"\
			"* signal\_delay -> time delay betwee microphones\n"\
			"* $( echo $geo | sed "s/.*\///" ) -> array geometry file\n" \
			"* $( echo $sig | sed "s/.*\///" ) -> source signal file\n" \
			> $dir/README.md


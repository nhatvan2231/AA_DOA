#include <fstream>
#include <iostream>
#include <iomanip>
#include <random>
#include <format>
#include <string>
#include <iostream>
#include <cstring>

using namespace std;

int main(int argc, char* argv[]){
	if(argc < 3 || argc > 11){
		cerr << "Usage: " << argv[0] << " --directory <path to directory> [optional: --lowerBound <angle lower bound> --upperBound <angle upper bound> --numofAngles <number of Angles in a set> --setofAngle <number of set of Angle>]\n" << endl;
		return 1;
	}
	random_device rd;
	default_random_engine mt{rd()};
	float lower_bound = 0.0;
	float upper_bound = 1.0;
	int N = 100;
	int M = 10;
	string directory = ".";
	for (int arg = 0; arg < argc; ++arg){
		if(strcmp(argv[arg], "--directory") == 0 || strcmp(argv[arg], "-dir") == 0)
			directory = argv[arg+1];
		else if(strcmp(argv[arg], "--lowerBound") == 0 || strcmp(argv[arg], "-lB") == 0)
			lower_bound = atof(argv[arg+1]);
		else if(strcmp(argv[arg], "--upperBound") == 0 || strcmp(argv[arg], "-uB") == 0)
			upper_bound = atof(argv[arg+1]);
		else if(strcmp(argv[arg], "--numofAngles") == 0 || strcmp(argv[arg], "-nA") == 0)
			N = atoi(argv[arg+1]);
		else if(strcmp(argv[arg], "--setofAngles") == 0 || strcmp(argv[arg], "-sA") == 0)
			M = atoi(argv[arg+1]);
	}
	//std::uniform_real_distribution<float> dist(0.0,180.0);
	std::uniform_real_distribution<float> random_angle(lower_bound,upper_bound);
	std::normal_distribution<float> random_speed(343.0, 5);
	float azimuth[N];
	float wind_speed[N];
	std::ofstream outfile;
	for (int j = 0; j<M; ++j){
		std::string out_path = directory + "/random_signal_source_info_" + to_string(j) + ".dat";
		outfile.open(out_path);
		outfile<<"sampleFrequency\t"<<"speed\t"<<"azimuth\t"<<"inclination"<<endl;
		for (int i=0; i<N; i++){
			azimuth[i] = random_angle(mt);
			wind_speed[i] = random_speed(mt);
		}
		for (int i=0; i<N; i++){
			outfile<<std::fixed<<std::setprecision(1)<<48000.0<<"\t"<<wind_speed[i]<<"\t"<<azimuth[i]<<"\t"<<0<<endl;
		}
		outfile.close();
	}

}

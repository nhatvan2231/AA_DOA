#include <fstream>
#include <iostream>
#include <iomanip>
#include <random>
#include <format>
#include <string>

using namespace std;

int main(){
	random_device rd;
	default_random_engine mt{rd()};
	//std::uniform_real_distribution<float> dist(-10000,10000);
	std::uniform_real_distribution<float> dist(0.0,180.0);
	//int xi = dist(mt);
	//int yi = dist(mt);
	int N = 100;
	float azimuth[N];
	std::ofstream outfile;
	for (int j = 0; j<10; ++j){
		std::string out_path = "random_signal_source_info_" + to_string(j) + ".dat";
		outfile.open(out_path);
		outfile<<"sampleFrequency\t"<<"speed\t"<<"azimuth\t"<<"inclination"<<endl;
		for (int i=0; i<N; i++){
			azimuth[i] = dist(mt);
		}
		for (int i=0; i<N; i++){
			outfile<<std::fixed<<std::setprecision(1)<<48000.0<<"\t"<<343<<"\t"<<azimuth[i]<<"\t"<<0<<endl;
		}
		outfile.close();
	}

}

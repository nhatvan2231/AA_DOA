#define MAX_CHANNELS 128

// Note: your units should be in the KMS standard (kilogram, meter, second)
#define ARRAY_GEOMETRY_FMT "<x1>\t<y1>\t<z1>\n" \
                           "<x2>\t<y2>\t<z2>\n" \
                           "...\n"
#define SIGNAL_SOURCE_FMT "sampleFrequency = <sample frequency>\n"\
	                       "speed = <signal speed in meters per second>\n"\
	                       "azimuth = <azimuthal angle of signal direction in degrees>\n"\
	                       "inclination = <polar angle of signal direction in degrees>\n"\

#include <iostream> // std::cerr std::cout
#include <iomanip> // std::setw(int)
#include <random> // std::default_random_engine generator std::normal_distribution<float> dist(float, float)
#include <string>
#include <fstream>
#include <climits> // INT_MAX
#include <cstring> // std::strcmp(char*, char*)
//#include <filesystem> // std::filesystem::path
#include "importArrayGeometry.h"
#ifndef inputBufferSize
#define inputBufferSize 8192
#endif
using namespace std;
void calcSlownessVector(float sampleFreq, float speed, float azimuth, float inclination, float slowness[3]);
void calcSampleDelays(float sampleDelays[], double arrayGeometry[][3], float slowness[3], unsigned N_channels, float sampleFrequency);
void calcTimeDelays(float sampleDelays[], double arrayGeometry[][3], float slowness[3], unsigned N_channels);

int main(int argc, char* argv[]) {
	if(argc < 7 || argc > 9){
		cerr << "Usage: " << argv[0] << " --arrayGeometry <array geometry filename> --signalInfo <signal information filename> [optional: --humanReadable]\n" << endl;
		cerr << "Array Geometry File Format:\n"<< ARRAY_GEOMETRY_FMT << endl;
		cerr << "Signal Source File Format:\n"<< SIGNAL_SOURCE_FMT << endl;
		cerr << "Note: units should follow the KMS standard (kilogram, meter, second)\n" << endl;
		return 1;
	}

	double arrayGeometry[MAX_CHANNELS][3];
	int N_channels = 0;

	ifstream inputFile;
	string path;
	bool getDelay = false;
	bool humanReadable = false;
	int dataSample = 0;

	for(int arg = 0; arg < argc; ++arg) {
		if(strcmp(argv[arg], "--arrayGeometry") == 0 || strcmp(argv[arg], "-aG") == 0)
			N_channels = importArrayGeometry(argv[arg + 1], arrayGeometry);
		else if(strcmp(argv[arg], "--signalInfo") == 0 || strcmp(argv[arg], "-sI") == 0)
			path = argv[arg + 1];
		else if(strcmp(argv[arg], "--dataSample") == 0 || strcmp(argv[arg], "-dS") == 0)
			dataSample = atoi(argv[arg+1]);
		else if(strcmp(argv[arg], "--getDelay") == 0 || strcmp(argv[arg], "-gD") == 0)
			getDelay = true;
		else if(strcmp(argv[arg], "--humanReadable") == 0 || strcmp(argv[arg], "-hR") == 0)
			humanReadable = true;
	}
	inputFile.open(path);
	if(N_channels <= 0 || !inputFile) {
		cerr << "Error in " << argv[0] << ":\n Invalid command-line arguments" << endl;
		return 1;
	}
	float* sampleFrequency = new float[dataSample];
	float* speed = new float[dataSample];
	float* azimuth = new float[dataSample];
	float* inclination = new float[dataSample];

	string dummy[4];
	inputFile >> dummy[0] >> dummy[1] >> dummy[2] >> dummy[3];
	int i = 0;
	for (int i = 0; i < dataSample; ++i){
		inputFile >> sampleFrequency[i] >> speed[i] >> azimuth[i] >> inclination[i];
	}
	inputFile.close();


	float** slowness = new float*[dataSample];
	float** timeDelays = new float*[dataSample];

	float** sampleDelays = new float*[dataSample];
	for (int i = 0; i < dataSample; ++i){
		slowness[i] = new float[3];
		sampleDelays[i] = new float[N_channels];
		timeDelays[i] = new float[N_channels];
	}

	for (int i = 0; i < dataSample; ++i){
		calcSlownessVector(sampleFrequency[i], speed[i], azimuth[i], inclination[i], slowness[i]);
		calcSampleDelays(sampleDelays[i], arrayGeometry, slowness[i], N_channels, sampleFrequency[i]);
		calcTimeDelays(timeDelays[i], arrayGeometry, slowness[i], N_channels);
	}

	double inputBuffer[inputBufferSize];
	fill(inputBuffer, inputBuffer+inputBufferSize, 0.0);
	unsigned inputIndex = 0;
	double bufferholder = 0;

	if(humanReadable) { // Print human-readable data
		if (getDelay){
			for (int i = 0; i < dataSample; ++i){
				for (int j = 0; j < N_channels; ++j){
							float output = timeDelays[i][j];
							cout << output << "\t";
				}
			}
			return 0;
		}
		while(cin >> inputBuffer[inputIndex]) {
			for(int i = 0; i < dataSample; i++){
				for(int c = 0; c < N_channels; ++c) {
					int upperDelay = std::ceil(sampleDelays[i][c]);
					int lowerDelay = std::floor(sampleDelays[i][c]);
					unsigned channelInputIndexUpper = (inputIndex - upperDelay + inputBufferSize) % inputBufferSize;
					unsigned channelInputIndexLower = (inputIndex - lowerDelay + inputBufferSize) % inputBufferSize;
					float output = 0;
					if(upperDelay == lowerDelay)
						output = inputBuffer[channelInputIndexUpper];
					else
						output = (upperDelay - sampleDelays[i][c]) * inputBuffer[channelInputIndexLower] + (sampleDelays[i][c] - lowerDelay) * inputBuffer[channelInputIndexUpper];
					if(c < N_channels - 1)
						cout << output << "\t";
					else
						cout << output << endl;
				}
				inputIndex = (inputIndex + 1) % inputBufferSize;
			}
		}
	} else {
		if (getDelay){
			for (int i = 0; i < dataSample; ++i){
				for (int j = 0; j < N_channels; ++j){
							float output = timeDelays[i][j];
							fwrite(&output, sizeof(float), 1, stdout);
				}
			}
			return 0;
		}
		while(fread(&inputBuffer[inputIndex], sizeof(double), 1, stdin)) {
		//while(cin >> inputBuffer[inputIndex]) {
			for(int i = 0; i < dataSample; i++){
				for(int c = 0; c < N_channels; ++c) {
					int upperDelay = std::ceil(sampleDelays[i][c]);
					int lowerDelay = std::floor(sampleDelays[i][c]);
					unsigned channelInputIndexUpper = (inputIndex - upperDelay + inputBufferSize) % inputBufferSize;
					unsigned channelInputIndexLower = (inputIndex - lowerDelay + inputBufferSize) % inputBufferSize;
					float output = 0;
					if(upperDelay == lowerDelay)
						output = inputBuffer[channelInputIndexUpper];
					else
						output = (upperDelay - sampleDelays[i][c]) * inputBuffer[channelInputIndexLower] + (sampleDelays[i][c] - lowerDelay) * inputBuffer[channelInputIndexUpper];
					if(c < N_channels - 1)
						cout << output << "\t";
					else
						cout << output << endl;
					//fwrite(&output, sizeof(float), 1, stdout);
				}
				inputIndex = (inputIndex + 1) % inputBufferSize;
			}
		}
	}

	//inputFile.close();
	return 0;
}
//
// returns sample frequency
void calcSlownessVector(float sampleFreq, float speed, float azimuth, float inclination, float slowness[3]){
	float slownessCheck = 0;
	azimuth *= M_PI/180.0;
	inclination *= M_PI/180.0;
	slowness[0] = cos(azimuth) * cos(inclination) / speed;
	slowness[1] = sin(azimuth) * cos(inclination) / speed;
	slowness[2] = sin(inclination) / speed;
	for(int d = 0 ; d < 3; ++d) {
		slownessCheck += slowness[d] * slowness[d];
	}
	if(slownessCheck <= 0.0) {
		cerr << "Error in calcSlownessVector(FILE*,float[3]):\nInvalid signal direction" << endl;
		exit(1);
	}

}
void calcSampleDelays(float sampleDelays[], double arrayGeometry[][3], float slowness[3], unsigned N_channels, float sampleFrequency){
	int maxDelay = -INT_MAX;
	int minDelay = INT_MAX;
	for(int c = 0; c < N_channels; ++c) {
		sampleDelays[c] = 0;
		for(int d = 0 ; d < 3; ++d) {
			sampleDelays[c] -= arrayGeometry[c][d] * slowness[d] * sampleFrequency;
		}
		if(sampleDelays[c] > maxDelay)
			maxDelay = sampleDelays[c];
		if(sampleDelays[c] < minDelay)
			minDelay = sampleDelays[c];
	}
	for(int c = 0; c < N_channels; ++c) {
		sampleDelays[c] += abs(minDelay) + 1;
	}
}

void calcTimeDelays(float timeDelays[], double arrayGeometry[][3], float slowness[3], unsigned N_channels){
	float maxDelay = -INT_MAX;
	float minDelay = INT_MAX;
	for(int c = 0; c < N_channels; ++c) {
		timeDelays[c] = 0;
		for(int d = 0 ; d < 3; ++d) {
			timeDelays[c] -= arrayGeometry[c][d] * slowness[d];
		}
		if(timeDelays[c] > maxDelay)
			maxDelay = timeDelays[c];
		if(timeDelays[c] < minDelay)
			minDelay = timeDelays[c];
	}
	for(int c = 0; c < N_channels; ++c) {
		timeDelays[c] += abs(minDelay);
	}
}

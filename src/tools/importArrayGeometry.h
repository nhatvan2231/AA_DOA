#ifndef importArrayGeometry_H
#define importArrayGeometry_H

#ifndef MAX_CHANNELS
#define MAX_CHANNELS 128
#endif

#include <cstdio>
#include <cstddef>
#ifdef DEBUG
#include <iostream> // std::cerr std::endl
#include <iomanip> // std::setw(int)
#endif

#define ARRAY_GEOMETRY_FMT "<x1>\t<y1>\t<z1>\n" \
                           "<x2>\t<y2>\t<z2>\n" \
                           "...\n"
// Returns Number of channels
// Passes back array geometry by reference
inline int importArrayGeometry(char filename[], double (&arrayGeometry)[MAX_CHANNELS][3]) {
	int N_channel = 0;

	FILE* inputFile = fopen(filename, "r");
	if(inputFile == NULL) {
#ifdef DEBUG
		std::cerr << "Error in importArrayGeometry.h" << std::endl <<  "Invalid array geometry filename" << std::endl;
#endif
		return 1;
	} else {
		while(!feof(inputFile) && N_channel < MAX_CHANNELS) {
			fscanf(inputFile, "%lf\t%lf\t%lf\n", &arrayGeometry[N_channel][0], &arrayGeometry[N_channel][1], &arrayGeometry[N_channel][2]);
			N_channel++;
		}
		fclose(inputFile);
	}
#ifdef DEBUG
	for(int c = 0; c < N_channel; ++c) {
		for(int d = 0; d < 3; ++d)
			std::cout << std::setw(16) << arrayGeometry[c][d];
		std::cout << std::endl;
	}
#endif
	return N_channel;
}
#endif

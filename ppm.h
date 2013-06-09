#ifndef __PULSAR_PPM_H__
#define __PULSAR_PPM_H__

#include <cstdio>

namespace pulsar{
	
	inline double clamp(double x){
		return x<0 ? 0 : x>1 ? 1 : x;
	}
	inline int toInt(double x){
		return int(pow(clamp(x), 1/2.2) * 255 + 0.5);
	}

	void save_ppm_file(const char* szFilename, const float* image, const int width, const int height) {
		using namespace std;
		FILE *f = fopen(szFilename, "wt"); // Write image to PPM file.
		fprintf(f, "P3\n%d %d\n%d\n", width, height, 255);
		for (int i=0; i<width*height; i++)
		fprintf(f,"%d %d %d ", toInt(image[3*i+0]), toInt(image[3*i+1]), toInt(image[3*i+2]));
	}

}

#endif
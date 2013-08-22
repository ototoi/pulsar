#ifndef __PULSAR_PPM_H__
#define __PULSAR_PPM_H__

#include <cstdio>
#include <cmath>

namespace pulsar{
	
	inline float clamp(float x){
		return x<0 ? 0 : x>1 ? 1 : x;
	}
	inline int toInt(float x){
		return int(pow(clamp(x), 1.0f/2.2f) * 255 + 0.5f);
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
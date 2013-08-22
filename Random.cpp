#include "Random.h"
#include <climits>

namespace pulsar{

	Random::Random(const unsigned int initial_seed)
	{
		unsigned int s = initial_seed;
		for (int i = 1; i <= 4; i++){
			seed_[i-1] = s = 1812433253U * (s^(s>>30)) + i;
		}
	}

	unsigned int Random::next()const
	{
		const unsigned int t = seed_[0] ^ (seed_[0] << 11);
		seed_[0] = seed_[1]; 
		seed_[1] = seed_[2];
		seed_[2] = seed_[3];
		return seed_[3] = (seed_[3] ^ (seed_[3] >> 19)) ^ (t ^ (t >> 8)); 
	}

	float Random::next01()const
	{
		return (float)next() / UINT_MAX;
	}

}


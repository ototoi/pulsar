#ifndef __PULSAR_RANDOM_H__
#define __PULSAR_RANDOM_H__

namespace pulsar{

	class Random
	{
	public:
		Random(const unsigned int initial_seed);
		unsigned int next()const;
		float next01()const;
	private:
		mutable unsigned int seed_[4];
	};

}

#endif
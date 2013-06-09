#ifndef __PULSAR_BOUND_H__
#define __PULSAR_BOUND_H__

#include "Vector3.h"

namespace pulsar{
	
	class Bound
	{
	public:
		Bound(const Vector3& min, const Vector3& max):min_(min), max_(max){}
		Vector3 min()const{return min_;}
		Vector3 max()const{return max_;}
	private:
		Vector3 min_;
		Vector3 max_;
	};
	
}

#endif
#ifndef __PULSAR_BOUND_H__
#define __PULSAR_BOUND_H__

#include "Vector3.h"
#include <utility>
#include <algorithm>

namespace pulsar{
	
	class Bound
	{
	public:
		Bound(const Vector3& min, const Vector3& max):min_(min), max_(max){}
		Vector3 min()const{return min_;}
		Vector3 max()const{return max_;}

		Vector3& min(){return min_;}
		Vector3& max(){return max_;}
	private:
		Vector3 min_;
		Vector3 max_;
	};

	inline Bound grow(const Bound& a, const Bound& b)
	{
		Bound bnd = a;
		for(int i=0;i<3;i++)
		{
			bnd.min()[i] = std::min(a.min()[i], b.min()[i]);
			bnd.max()[i] = std::max(a.max()[i], b.max()[i]);
		}
		return bnd;
	}
	
}

#endif
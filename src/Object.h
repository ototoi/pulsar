#ifndef __PULSAR_OBJECT_H__
#define __PULSAR_OBJECT_H__

#include "Ray.h"
#include "Bound.h"
#include "Intersection.h"

namespace pulsar{
	
	class Object
	{
	public:
		virtual ~Object(){}
		virtual bool intersect(Intersection* info, const Ray& r, float tmin, float tmax)const=0;
		virtual Bound bound()const=0;
	};
	
}

#endif
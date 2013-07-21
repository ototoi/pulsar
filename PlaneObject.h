#ifndef __PULSAR_PLANE_OBJECT_H__
#define __PULSAR_PLANE_OBJECT_H__

#include "Object.h"

namespace pulsar{
	
	class PlaneObject:public Object{
	public:
		PlaneObject(const Vector3& p, const Vector3& n):p_(p), n_(n){}
		virtual bool intersect(Intersection* info, const Ray& r, float tmin, float tmax)const;
		virtual Bound bound()const;
	private:
		Vector3 p_;
		Vector3 n_;
	};
	
}

#endif
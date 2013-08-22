#ifndef __PULSAR_SPHERE_OBJECT_H__
#define __PULSAR_SPHERE_OBJECT_H__

#include "Object.h"

namespace pulsar{
	
	class SphereObject:public Object{
	public:
		SphereObject(const Vector3& c, float rad):c_(c), rad_(rad){}
		virtual bool intersect(Intersection* info, const Ray& r, float tmin, float tmax)const;
		virtual Bound bound()const;
	private:
		Vector3 c_;
		float rad_;
	};
	
}

#endif
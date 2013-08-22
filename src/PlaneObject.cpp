#include "PlaneObject.h"
#include <limits>

namespace pulsar
{

	bool PlaneObject::intersect(Intersection* info, const Ray& r, float tmin, float tmax)const
	{
		Vector3 p = p_;
		Vector3 n = n_;
		float d = -dot(p, n);
		float v =  dot(r.dir(), n);

		if (fabs(v) < std::numeric_limits<float>::epsilon()) return false;

		float t = -(dot(r.org(), n) + d) / v;

		if ((t > tmin) && (t < tmax)) {
			info->t = t;
			info->position = r.org() + t*r.dir();
			info->normal = n;
			info->pObject = this;
			return true;
		}
		return false;

	}
	
	Bound PlaneObject::bound()const
	{
		return Bound(Vector3(0,0,0),Vector3(0,0,0));
	}



}
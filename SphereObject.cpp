#include "SphereObject.h"


namespace pulsar{
	
	bool SphereObject::intersect(Intersection* info, const Ray& r, float tmin, float tmax)const
	{
		
		return false;
	}
	
	Bound SphereObject::bound()const
	{
		Vector3 C = c_;
		float R = rad_; 
		return Bound(Vector3(C[0]-R, C[1]-R, C[2]-R), Vector3(C[0]+R, C[1]+R, C[2]+R));
	}
	
	
}

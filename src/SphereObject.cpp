#include "SphereObject.h"
#include <cmath>


namespace pulsar{
	
	bool SphereObject::intersect(Intersection* info, const Ray& r, float tmin, float tmax)const
	{
		Vector3 rs = r.org() - c_;

		float B = dot(rs, r.dir());
		float C = dot(rs, rs) - rad_ * rad_;
		float D = B * B - C;
		
		if (D > 0.0) {
			float sqrtD = sqrt(D);
			float ts[] =
			{
				-B - sqrtD,
				-B + sqrtD
			};
			for(int i=0;i<2;i++){
				float t = ts[i];
				if ((t > tmin) && (t < tmax)) {
					info->t = t;
					info->position = r.org() + t*r.dir();
					info->normal  = normalize(info->position-c_);
					info->pObject = this;
					return true;
				}
			}
		}
		return false;
	}
	
	Bound SphereObject::bound()const
	{
		Vector3 C = c_;
		float R = rad_; 
		return Bound(Vector3(C[0]-R, C[1]-R, C[2]-R), Vector3(C[0]+R, C[1]+R, C[2]+R));
	}
	
	
}

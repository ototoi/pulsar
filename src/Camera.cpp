#include "Camera.h"
#include <cmath>

using namespace std;

namespace pulsar{

	PerspectiveCamera::PerspectiveCamera(const Vector3& from, const Vector3& to, const Vector3& upper, float angle, float aspect)
	{
		from_ = from;

		Vector3 az = normalize(to - from);		//
		Vector3 ax = normalize(cross(az,upper));//-y^z
		Vector3 ay = cross(ax,az);				//
		
		axis_z_ = az;
		axis_y_ = ay;
		axis_x_ = ax;

		float tangent = tan(angle*0.5f);
		cof_x_ = aspect* tangent;
		cof_y_ =         tangent;
	}

	Ray PerspectiveCamera::shoot(float i, float j)const
	{
		float WW = cof_x_*i;
		float HH = cof_y_*j;

		return Ray(from_,normalize(WW*axis_x_+HH*axis_y_+axis_z_));
	}

}
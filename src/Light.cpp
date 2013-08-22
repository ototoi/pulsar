#include "Light.h"
#include "Intersection.h"

namespace pulsar{

	PointLight::PointLight(const Vector3& col, const Vector3& pos)
		:col_(col), pos_(pos)
	{
		;
	}

	void PointLight::cast(Vector3& col, Vector3& dir, const Intersection& info)const
	{
		Vector3 d = info.position-pos_;
		if(length(d)>0)
		{
			col=col_;
			dir=normalize(d);
		}else{
			col=Vector3(0,0,0);
			dir=Vector3(0,0,1);
		}
	}


	DirectionalLight::DirectionalLight(const Vector3& col, const Vector3& dir)
		:col_(col), dir_(dir)
	{
		;
	}

	void DirectionalLight::cast(Vector3& col, Vector3& dir, const Intersection& info)const
	{
		col=col_;
		dir=normalize(dir_);
	}

}

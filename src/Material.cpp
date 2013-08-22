#include "Material.h"
#include <cmath>
#include <algorithm>
#include <utility>

using namespace std;

namespace pulsar{

	EmissionMaterial::EmissionMaterial(const Vector3& Ka)
		:Ka_(Ka)
	{
		;
	}
	Vector3 EmissionMaterial::shade(const Intersection& info,  const std::vector<const Light*>& l)const
	{
		return Ka_;
	}


	LambertMaterial::LambertMaterial(const Vector3& Kd)
		:Kd_(Kd)
	{
	}

	Vector3 LambertMaterial::shade(const Intersection& info, const std::vector<const Light*>& l)const
	{
		//return Kd_;
		Vector3 n = normalize(info.normal);
		if(dot(n,info.ray_direction)>0){
			n = -n;
		}
		//return (Vector3(1,1,1)+n)*0.5f;
		Vector3 sum = Vector3(0,0,0);
		for(size_t i =0;i<l.size();i++){
			Vector3 dir,col;
			l[i]->cast(col, dir, info);
			float d = max<float>(0, dot(-dir, n));
			sum += col*d*Kd_;
		}

		return sum;
	}

}

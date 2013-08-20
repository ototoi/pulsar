#ifndef __PULSAR_MATERIAL_H__
#define __PULSAR_MATERIAL_H__

#include "Intersection.h"
#include "Object.h"
#include "Light.h"

#include <vector>

namespace pulsar{
	
	class Material
	{
	public:
		virtual ~Material(){}
		virtual Vector3 shade(const Intersection& info,  const std::vector<const Light*>& l)const=0;
	};

	class EmissionMaterial:public Material
	{
	public:
		EmissionMaterial(const Vector3& Ka);
		Vector3 shade(const Intersection& info,  const std::vector<const Light*>& l)const;
	private:
		Vector3 Ka_;
	}; 

	class LambertMaterial:public Material
	{
	public:
		LambertMaterial(const Vector3& Kd);
		Vector3 shade(const Intersection& info,  const std::vector<const Light*>& l)const;
	private:
		Vector3 Kd_;
	};
	
}

#endif